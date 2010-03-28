<?php
/*
	YLEM上传文件流程：
	1. 接收多个上传文件请求，从query参数中获得本次操作的namespace和响应内容格式ret
	2. 提取各个文件的id、vol(可选)、type(应该对每个文件指定，若不指定则默认为application/octet-stream)
	3. 同YLEM cluster monitor通信，获得经过排序的volume列表
	4. 若存在未指定vol的文件，则选择volume列表中排名最高且非0者作为这些文件的vol，若没有这样的volume存在
	则所有文件都标记为操作失败
	5. 按照vol将文件聚集成组
	6. 对每个组，建立同该组的YLEM tracker服务的连接，对组中的每个文件向tracker
	发送create_open命令，获得多个可能的远程存储位置。若某个文件操作失败，则将该
	文件标记为保存失败，并记录错误原因。
	7. 对组中每个文件，尝试以标准HTTP方式将文件内容保存到可能的存储位置，若第一个
	选择的存储位置操作失败，则尝试下一个；若所有存储位置均尝试失败，则将当前文件
	标记为保存失败，并记录错误原因。当保存成功时，记录当前文件实际使用的存储位置
	8. 组中所有文件保存操作结束(不管成功与否)后，对组中每个文件向tracker发送create_close
	命令，指明实际使用的存储位置，文件名就用id，然后关闭同该组的YLEM tracker服务的连接。
	9. 继续处理下一个组中的文件
	10. 全部文件处理完毕后，根据文件保存状况和响应内容格式构造对应的响应内容返回。

	注意：
	1. 关于用multipart/form-data类型进行HTTP方式文件上传操作的信息请参考RFC 1521和RFC 1867
	2. PHP版的上传文件接口由于是PHP解释器来分离多个文件数据，因此无法实现以id字段作为文件标识这一原始需求，
	为了达到最大的灵活性，在PHP实现中我们忽略Content-Disposition域中name字段(表单上传时input元素的名称)的不同，
	直接以filename字段(表单上传时选中的原始文件名称)作为卷标识ID+文件标识ID(以"[vol_id-]file_id"的形式表现)；
	3. 若上传文件的表单格式为：
		<form enctype="multipart/form-data" action="http://host/ylem/ul?version=1.0&ns=test" method="post">
			file1:<input name="userfile[]" type="file"><br><!-- 选择了文件test1,类型image/jpeg,长度2880 -->
			file2:<input name="file1" type="file"><br><!-- 选择了文件test2,类型text/html,长度123 -->
			file3:<input name="file2" type="file"><br><!-- 选择了文件test3,类型application/octet-stream,长度12 -->
			file4:<input name="userfile[]" type="file"><br><!-- 选择了文件test4,类型text/plain,长度24 -->
			<input type="submit" value="Send file">
		</form>
	则PHP脚本中$_FILES的内容将为：
		array(
			"userfile" => array(	// 数组域多文件上传
				"name" => array(
					"test1",
					"test4"
				),
				"type" => array(
					"image/jpeg",
					"text/plain"
				),
				"tmp_name" => array(
					"/tmp/phpbKMXo8",
					"/tmp/phpNqjK3Z"
				),
				"error" => array(
					UPLOAD_ERR_OK,
					UPLOAD_ERR_OK
				),
				"size" => array(
					2880,
					24
				)
			),
			"file1" => array(	// 独立域文件上传
				"name" => "test2",
				"type" => "text/html",
				"tmp_name" => "/tmp/phpwS7cXK",
				"error" => UPLOAD_ERR_OK,
				"size" => 123
			),
			"file2" => array(	// 独立域文件上传
				"name" => "test3",
				"type" => "application/octet-stream",
				"tmp_name" => "/tmp/phptqktvn",
				"error" => UPLOAD_ERR_OK,
				"size" => 12
			)
		)
 */
require("ylem_cli/ylem.inc");

// 抑制PHP出错信息
error_reporting(~E_ALL);

// 获取URL query参数，这些参数已经由PHP自动进行了URL decoding
$version=filter_input(INPUT_GET,"version",FILTER_SANITIZE_STRING); 	// 接口版本，目前为1.0
$namespace=filter_input(INPUT_GET,"ns",FILTER_SANITIZE_STRING); // 命名空间，大小写敏感
$respfmt=filter_input(INPUT_GET,"ret",FILTER_SANITIZE_STRING);	// 响应格式，大小写不敏感，json/xml

// 若接口版本串不对或未给出命名空间则直接返回404错误
if(!$version || $version!="1.0"
	|| !$namespace || strlen($namespace)==0) {
	header("HTTP/1.1 404 Invalid parameter");
	return;
}
if(!$respfmt) {
	$respfmt="json";
} else {
	// 给定了响应格式，全部转换为小写字母
	$respfmt=strtolower($respfmt);
}

// 获得上传文件信息列表
$file_lst=get_file_list($_FILES);
if(count($file_lst)>0) {
	// 有待处理的上传文件，从YLEM cluster monitor获取集群卷信息
	$vol_lst=monitor_get_volumes();
	$vol_idx=monitor_build_volumes_index($vol_lst);
	$vol_files=array();

	// 过滤上传文件列表，将无法找到可用存储卷的文件和指定的存储卷不可写的文件状态标记为出错。
	// 另外还将对应卷可写的文件按照卷进行分组，以便在同一个连接中处理完所有目标卷相同的文件。
	for($i=0;$i<count($file_lst);++$i) {
		if(!isset($file_lst[$i]["vol"])) {
			// 当前上传文件没有指定目标卷，使用目前最闲的卷保存该文件
			// XXX: 对于一次上传请求中的所有未指定目标卷的文件，YLEM系统为其选定的目标卷
			// 都是一样的。
			if($vol_lst[0]["score"]>0) {
				// 将目前最闲的卷用来保存当前文件，同时按照卷ID进行文件分组
				$vol_id=$vol_lst[0]["vol"];
				$file_lst[$i]["vol"]=$vol_id;
				$vol_files[$vol_id][]=$i;
			} else {
				// 没有可用卷保存当前文件
				error_log("ERROR: No writable volume available for destination, cannot save file no.$i");
				$file_lst[$i]["error"]=UPLOAD_ERR_CANT_WRITE;
			}
		} else {
			// 当前上传文件已指定目标卷，检查指定卷的合法性
			$idx=$file_lst[$i]["vol"];
			if(!isset($vol_idx[$idx]) || $vol_lst[$vol_idx[$idx]]["score"]==0) {
				// 指定卷非法或对应卷目前不可写，将当前文件标记为出错
				error_log("ERROR: Invalid destination volume / Volume write service down, cannot save file no.$i: "
					."volume id: ".$file_lst[$i]["vol"]);
				$file_lst[$i]["error"]=UPLOAD_ERR_CANT_WRITE;
			} else {
				// 指定卷合法且可写，按照卷ID进行文件分组
				$vol_id=$file_lst[$i]["vol"];
				$vol_files[$vol_id][]=$i;
			}
		}
	}

	// 处理每个目标卷中的文件
	foreach($vol_files as $vol=>$files) {
		// 连接当前卷对应的Tracker服务
		$tracker_host=$vol_lst[$vol_idx[$vol]]["tracker"];
		$trkr_h=tracker_connect($tracker_host,YLEM_TRACKER_TIMEOUT);
		if($trkr_h) {
			// Tracker服务连接成功
			foreach($files as $idx) {
				// 发送CREATE_OPEN命令准备创建文件，获得可能的存储位置列表
				list($fid,$dev_cands,$st_cands)=tracker_create_open($trkr_h,$namespace,$file_lst[$idx]["id"]);
				if(count($st_cands)>0) {
					$st_idx=-1;
					$uploaded=false;
					// 尝试每个可能的存储位置保存当前文件
					for($i=0;$i<count($st_cands);++$i) {
						if(store_upload(
							$st_cands[$i],					// remote url
							$file_lst[$idx]["tmp_name"],	// local file path
							$file_lst[$idx]["size"]			// local file size
						)) {
							// 文件上传到后端存储节点成功
							$st_idx=$i;
							$uploaded=true;
							break;
						}
					}
					if(!$uploaded) {
						// 所有可能的存储位置尝试完毕也没能保存当前文件，将当前文件标记为出错
						error_log("ERROR: No usable storage position, cannot save file no.$idx");
						$file_lst[$idx]["error"]=UPLOAD_ERR_CANT_WRITE;
					} else {
						// 当前文件保存成功，发送CREATE_CLOSE命令结束文件创建过程
						if(tracker_create_close(
							$trkr_h,
							$namespace,					// domain
							$fid,						// fid
							$dev_cands[$st_idx],		// devid
							$st_cands[$st_idx],			// path
							$file_lst[$idx]["id"],		// key
							$file_lst[$idx]["type"],	// mimetype
							$file_lst[$idx]["size"]		// size
						)) {
							// 文件创建过程成功结束
							$resid=$vol.'-'.$file_lst[$idx]["id"];
							error_log("SUCCESS: Uploaded file: ns='$namespace', resid='$resid'");
						} else {
							// CREATE_CLOSE操作失败，无法保存当前文件
							error_log("ERROR: CREATE_CLOSE operation for file no.$idx failed");
							$file_lst[$idx]["error"]=UPLOAD_ERR_CANT_WRITE;
						}
					}
				} else {
					// CREATE_OPEN操作失败，无法保存当前文件
					error_log("ERROR: CREATE_OPEN operation for file no.$idx failed");
					$file_lst[$idx]["error"]=UPLOAD_ERR_CANT_WRITE;
				}
			}

			// 当前卷中的文件处理完毕，关闭同Tracker服务的连接
			// XXX: 改为持久连接后不需要显式关闭同 Tracker 服务的连接
			//tracker_close($trkr_h);
		} else {
			// 无法连接当前卷对应的Tracker服务，所有要保存到当前卷中的文件都要标记为出错
			error_log("ERROR: Cannot connect to tracker service in volume $vol");
			foreach($files as $idx) {
				$file_lst[$idx]["error"]=UPLOAD_ERR_CANT_WRITE;
			}
		}
	}
} else {
	// 没有待处理的上传文件
}

// 生成响应结果
$resp_str="";
if($respfmt=="json") {
	$resp_str.="[";
} else {
	$resp_str.="<?xml version=\"1.0\" encoding=\"utf-8\"?><result>";
}

// 遍历上传文件列表
$start=true;
foreach($file_lst as $file) {
	if($start) {
		$start=false;
	} else {
		if($respfmt=="json") {
			$resp_str.=",";
		}
	}
	if(!$file["error"]) {
		// 当前文件上传成功
		if($respfmt=="json") {
			$resp_str.='{"'.$file["id"].'":{"vol":"'.$file["vol"].'"}}';
		} else {
			$resp_str.='<file><id>'.$file["id"].'</id><vol>'.$file["vol"].'</vol></file>';
		}
	} else {
		// 当前文件上传出错
		if($respfmt=="json") {
			$resp_str.='{"'.$file["id"].'":{"err":"'.$file["error"].'"}}';
		} else {
			$resp_str.='<file><id>'.$file["id"].'</id><err>'.$file["error"].'</err></file>';
		}
	}
}

if($respfmt=="json") {
	$resp_str.="]";
} else {
	$resp_str.="</result>";
}

// 输出响应内容
header("Content-Type:text/plain;charset=utf-8");
echo $resp_str;

/**
	将$_FILES数组转换为文件列表，列表格式如下：
	array(
		array(
			"name"=>"文件1的原始文件名",
			"type"=>"文件1的mimetype",
			"tmp_name"=>"文件1上传后的临时文件路径",
			"error"=>"文件1上传时错误代码",
			"size"=>"文件1总长度",
			"id"=>"文件1的标识ID，同name相同",
			"vol"=>"文件1选定的目标卷ID，若不存在则由YLEM系统自行选择目标卷"
		),
		array(
			// 文件2的对应信息
		),
		...
	)
*/
function get_file_list($info_lst)
{
	$fl=array();
	foreach($info_lst as $name=>$info) {
		if(isset($info["name"])) {
			if(is_array($info["name"])) {
				$names=$info["name"];
				$types=$info["type"];
				$tmp_names=$info["tmp_name"];
				$errors=$info["error"];
				$sizes=$info["size"];

				$len=count($names);
				for($i=0;$i<$len;++$i) {
					if(strlen($names[$i])==0) {
						continue;
					}
					$item=array(
						"name"=>$names[$i],
						"type"=>isset($types[$i])?$types[$i]:"application/octet-stream",
						"tmp_name"=>$tmp_names[$i],
						"error"=>$errors[$i],
						"size"=>$sizes[$i]
					);
					$parts=explode("-",$item["name"],2);
					if(count($parts)!=2) {
						$item["id"]=$parts[0];
					} else {
						list($item["vol"],$item["id"])=$parts;
					}
					$fl[]=$item;
				}
			} else {
				if(strlen($info["name"])==0) {
					continue;
				}
				if(!isset($info["type"])) {
					$info["type"]="application/octet-stream";
				}
				$parts=explode("-",$info["name"],2);
				if(count($parts)!=2) {
					$info["id"]=$parts[0];
				} else {
					list($info["vol"],$info["id"])=$parts;
				}
				$item=array();
				foreach($info as $key=>$val) {
					$item[$key]=$val;
				}
				$fl[]=$item;
			}
		}
	}
	return $fl;
}

// vi:ft=php ts=4 sw=4
?>
