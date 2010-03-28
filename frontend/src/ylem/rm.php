<?php
/*
	YLEM删除文件流程：
	1. 从query参数中获得本次操作的namespace和相应内容格式ret
	2. 从query参数中获得待删除的文件完整ID("vol_id-file_id"形式)
	3. 同YLEM cluster monitor通信，获得经过排序的volume列表
	4. 按照vol_id对待删除文件进行分组，若某一组对应的vol_id在volume列表中不存在，
	则表示对应volume不存在或暂时不可用，直接将该组内的所有文件标记为操作失败
	5. 对每个组，建立同该组的YLEM tracker服务的连接，对组中的每个文件向tracker发送
	delete命令；若某个文件操作失败，则将该文件标记为删除失败。
	6. 组中所有文件删除操作结束(不管成功与否)后，关闭同该组的YLEM tracker服务的连接
	7. 继续处理下一个组中的文件
	8. 全部文件处理完毕后，根据文件删除状况和响应内容格式构造对应的响应内容返回。
 */
require("ylem/common.inc");
require("ylem/tracker.inc");
require("ylem/monitor.inc");

// 抑制PHP出错信息
error_reporting(~E_ALL);

// 获取URL query参数，这些参数已经由PHP自动进行了URL decoding
$version=filter_input(INPUT_GET,"version",FILTER_SANITIZE_STRING); 	// 接口版本，目前为1.0
$namespace=filter_input(INPUT_GET,"ns",FILTER_SANITIZE_STRING); // 命名空间，大小写敏感
$respfmt=filter_input(INPUT_GET,"ret",FILTER_SANITIZE_STRING); 	// 响应格式，大小写不敏感，json/xml
$rmid=filter_input(INPUT_GET,"id",FILTER_SANITIZE_STRING); 		// 待删除文件完整ID列表，每个文件的完整ID必须为"vol_id-file_id"这种形式

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

$ids=array_filter(explode(",",$rmid),"is_canonical_resid"); 	// 将文件ID列表拆分开并过滤出有效的完整文件ID

$res_lst=array();
if(count($ids)>0) {
	// 有待删除的文件
	// 将待删除文件按照所在卷进行分组，以便在一个Tracker连接中对同一卷内的文件进行操作
	$vol_lst=monitor_get_volumes();
	$vol_idx=monitor_build_volumes_index($vol_lst);

	$grp_ids=array();
	for($i=0;$i<count($ids);++$i) {
		list($vol,$id)=explode("-",$ids[$i]);
		if(isset($vol_idx[$vol]) && $vol_lst[$vol_idx[$vol]]["score"]>=0) {
			// 只处理所在卷有效的那些文件
			$grp_ids[$vol][]=$id;
		}
	}

	// 遍历按所在卷划分成组的待删除文件列表，逐个删除之
	foreach($grp_ids as $vol=>$fids) {
		$tracker_host=$vol_lst[$vol_idx[$vol]]["tracker"];
		$fh=tracker_connect($tracker_host,YLEM_TRACKER_TIMEOUT);
		if($fh) {
			// Tracker服务连接成功
			foreach($fids as $fid) {
				if(tracker_delete($fh,$namespace,$fid)) {
					// 文件删除成功，添加到结果列表里
					$res_lst[]="{$vol}-{$fid}";
					// 记录成功删除的文件
					error_log("SUCCESS: Removed file: ns='$namespace', resid='$vol-$fid'");
				} else {
					// 文件删除失败
					error_log("ERROR: Failed to delete file '{$vol}-{$fid}'");
				}
			}
		} else {
			error_log("ERROR: Cannot connect to tracker service in volume $vol");
		}
	}
} else {
	// 没有待删除的文件
}

// 开始生成响应结果
$resp_str="";
if($respfmt=="json") {
	$resp_str.="[";
} else {
	$resp_str.="<?xml version=\"1.0\" encoding=\"utf-8\"?><result>";
}

// 遍历删除成功的文件列表
$start=true;
foreach($res_lst as $fid) {
	if($start) {
		$start=false;
	} else {
		if($respfmt=="json") {
			$resp_str.=",";
		}
	}
	if($respfmt=="json") {
		$resp_str.="\"$fid\"";
	} else {
		$resp_str.="<id>$fid</id>";
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

// vi:ft=php ts=4 sw=4
?>
