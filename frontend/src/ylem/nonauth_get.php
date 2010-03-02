<?php
// vi:ft=php ts=4 sw=4
/*
	YLEM获取文件流程：
	1. 接收文件下载请求，从query参数中获得本次操作的namespace和响应内容格式ret(对于文件下载没用！)
	2. 从query参数中获得要下载的文件完整ID("vol_id-file_id"的形式)、下载响应的Content-Type(可选，
	若不指定则默认为保存文件时的Content-Type)、下载响应的filename(可选，若不指定则默认为文件ID加上
	Content-Type对应的后缀)、以及无法获取该文件时的重定向URL(可选，若不指定则当文件无法下载时返回
	404错误)
	3. 同YLEM cluster monitor通信，获得经过排序的volume列表
	4. 若待下载文件的vol_id不在volume列表中，表明对应volume不存在，将该文件标记为下载失败
	5. 连接文件所在volume的Tracker服务，查询可能的存储位置
	6. 逐个尝试每个存储位置，直至找到第一个可以访问的地方
	7. 从Cookie中获取用户yuid，并连接UACS查询该用户是否可以读取当前文件
	8. 若UACS允许此次请求，则发送响应头及文件内容；否则根据有无redirect参数决定是重定向还是直接403
	Forbidden错误

	注意：
	1. 下载响应头中指定文件名应该通过如下形式进行：
		Content-Disposition: inline; filename="预定义文件名";
	这样对于浏览器可识别的文件类型则会自动展现，且另存为文件时预定义文件名会被自动作为默认文件名。
	具体信息可参考RFC 2183
	2. 下载文件失败时的重定向操作直接通过HTTP的302 Redirect操作进行。
	3. 默认文件名的后缀通过文件Content-Type决定，可以以列表形式在配置文件中列出不同mimetype对应的
	后缀名，未列出的mimetype和application/octet-stream的后缀统一为空
 */
require("ylem/common.inc");
require("ylem/tracker.inc");
require("ylem/store.inc");
require("ylem/monitor.inc");

// 抑制PHP出错信息
error_reporting(~E_ALL);

$version=filter_input(INPUT_GET,"version",FILTER_SANITIZE_STRING); 	// 接口版本，目前为1.0
$params=array();
$params["ns"]=filter_input(INPUT_GET,"ns",FILTER_SANITIZE_STRING);
$params["id"]=filter_input(INPUT_GET,"id",FILTER_SANITIZE_STRING);
$params["ct"]=filter_input(INPUT_GET,"ct",FILTER_SANITIZE_STRING);
$params["fn"]=filter_input(INPUT_GET,"fn",FILTER_SANITIZE_STRING);
$params["rd"]=filter_input(INPUT_GET,"rd",FILTER_SANITIZE_STRING);

// XXX by ugg, add expires
$params["ex"]=filter_input(INPUT_GET,"ex",FILTER_SANITIZE_STRING);
// 默认30天
$ex=(isset($params["ex"]) && $params["ex"])?$params["ex"]:false;		// 过期时间头

$namespace=(isset($params["ns"]) && $params["ns"])?$params["ns"]:false; 	// 命名空间，大小写敏感
$resid=(isset($params["id"]) && $params["id"])?$params["id"]:false; 		// 待下载文件的完整资源id("vol_id-file_id"形式)

// 必选参数没有给出，返回400 Bad Request错误
if(!$namespace || strlen($namespace)==0
	|| !$resid || strlen($resid)==0) {
	header("HTTP/1.1 400 Invalid parameter");
	return;
}

$ct_type=(isset($params["ct"]) && $params["ct"])?$params["ct"]:false; 		// 选填参数，指定header中的Content-Type内容。
$filename=(isset($params["fn"]) && $params["fn"])?$params["fn"]:false; 	// 选填参数，指定header中Content-Disposition里的filename，不填则默认为文件id加上可识别的Content-Type后缀。
$redirect=(isset($params["rd"]) && $params["rd"])?$params["rd"]:false; 	// 选填参数，若无法获取指定文件则重定向到该URL，
							// 不填则在无法获取文件时报告404错误

// 若给定的文件资源ID不合法则产生400 Bad Request错误退出
if(!is_canonical_resid($resid)) {
	error_log("ERROR: Given resource id '$resid' is invalid");
	header("HTTP/1.1 400 Invalid resource id");
	return;
}

list($vol,$fid)=explode("-",$resid,2);
$vol_lst=monitor_get_volumes();
$vol_idx=monitor_build_volumes_index($vol_lst);
$got_loc=false;
$paths=array();
$mimetype="";

// 此处不需要检测所在卷的评分是否为0，因为评分为0仅代表对应卷不可写，一般情况下仍然可读
if(isset($vol_idx[$vol])) {
	$tracker_host=$vol_lst[$vol_idx[$vol]]["tracker"];

	// 从Tracker服务中获取给定文件存储位置信息(带缓存)
	$res=tracker_cached_get_paths($tracker_host,$namespace,$vol,$fid,YLEM_TRACKER_TIMEOUT);
	if($res) {
		// 获得了给定文件的存储位置信息
		// 若请求参数中给定了Content-Type则使用之，否则用保存文件时使用的Content-Type
		if($ct_type) {
			$mimetype=$ct_type;
		} else {
			$mimetype=$res["mimetype"];
		}
		$paths=$res["paths"];
		// 若未给定下载文件名，则以文件ID加上MIME type对应的后缀作为文件名
		if(!$filename) {
			$filename=$fid.get_suffix_for_mimetype($mimetype);
		}

		// 尝试从某个存储位置获取文件数据并发回客户端
		for($i=0;$i<count($paths);++$i) {
			$size=store_get_file_size($paths[$i]);
			if($size>0) {
				// 当前存储位置可用
				header("Content-Type: $mimetype");
				header("Content-Length: $size");
				header("Content-Disposition: inline; filename=\"$filename\";");
				if($ex){
					$ExpStr = "Expires: " . gmdate("D, d M Y H:i:s", time() + $ex) . " GMT";
					header($ExpStr);
				}
				store_echo_file($paths[$i]);
				$got_loc=true;
				// 记录成功的下载事件
				error_log("SUCCESS: Downloaded file: ns='$namespace', resid='$resid'");
				break;
			}
		}
	} else {
		// 未获得文件存储位置
		$got_loc=false;
	}
} else {
	// 下载文件所在的卷ID不存在
	error_log("ERROR: Given volume id '$vol' doesn't exist");
	$got_loc=false;
}

if(!$got_loc) {
	// 文件内容获取或发送未成功
	if($redirect) {
		// 定义了重定向参数，重定向到给定的位置
		header("Location: $redirect");
	} else {
		// 没有定义重定向参数，产生404 Not Found错误
		header("HTTP/1.1 404 Failed to get content");
	}
}
?>
