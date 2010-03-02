<?php
// vi:ft=php ts=4 sw=4
/**
	YLEM存储后端接口
 */
if(!defined("YLEM_FE_STORE")) {
	define("YLEM_FE_STORE",1);

	function output_header($ch,$data)
	{
		// 将后端存储节点返回的Last-Modified和ETag头原封不动输出，以支持CDN缓存策略
		if(strtolower(substr($data,0,13))=="last-modified"
			|| strtolower(substr($data,0,4))=="etag") {
			$str=trim($data);
			header($str);
		}
		return strlen($data);
	}

	/**
		获取指定远程URL处的文件内容并发送到当前的标准输出中(根据CLI/CGI模式的不同而变化)。
		@param $url 待获取的远程URL。
		@ret 操作成功时返回true，否则返回false。
	 */
	function store_echo_file($url)
	{
		if(!$url) {
			return false;
		}
		$ch=curl_init($url);
		curl_setopt($ch,CURLOPT_HEADERFUNCTION,"output_header");
		$res=false;
		if($ch) {
			curl_exec($ch);
			$http_status=curl_getinfo($ch,CURLINFO_HTTP_CODE);
			if($http_status>=200&&$http_status<300) {
				$size=curl_getinfo($ch,CURLINFO_CONTENT_LENGTH_DOWNLOAD);
				$res=true;
			} else {
				error_log("WARN: Failed to execute GET for $url: status=$http_status");
			}
			curl_close($ch);
		} else {
			error_log("WARN: Failed to initialize CURL for $url");
		}
		return $res;
	}

	/**
		获取远程存储后端上给定文件的大小。
		@param $url 远程文件URL。
		@ret 获取成功时返回远程文件的大小，失败时返回-1。
	 */
	function store_get_file_size($url)
	{
		if(!$url) {
			return -1;
		}
		$ch=curl_init($url);
		$res=-1;
		if($ch) {
			curl_setopt($ch,CURLOPT_NOBODY,true);
			curl_setopt($ch,CURLOPT_RETURNTRANSFER,true);
			curl_exec($ch);
			$http_status=curl_getinfo($ch,CURLINFO_HTTP_CODE);
			if($http_status>=200&&$http_status<300) {
				$res=curl_getinfo($ch,CURLINFO_CONTENT_LENGTH_DOWNLOAD);
			} else {
				error_log("WARN: Failed to execute HEAD for $url: status=$http_status");
			}
			curl_close($ch);
		} else {
			error_log("WARN: Failed to initialize CURL for $url");
		}
		return $res;
	}

	/**
		将本地文件上传到指定的YLEM存储后端上。
		@param $url YLEM存储后端目标文件URL。
		@param $file_path 本地文件路径。
		@param $file_size 本地文件大小。
		@ret 上传成功返回true，否则返回false。
	 */
	function store_upload($url,$file_path,$file_size)
	{
		// 打开本地文件
		$fh=fopen($file_path,"r");
		if(!$fh) {
			error_log("WARN: Failed to open local file: $file_path");
			return false;
		}

		$result=false;
		$ch=curl_init($url);
		if($ch) {
			curl_setopt($ch,CURLOPT_PUT,true);
			curl_setopt($ch,CURLOPT_RETURNTRANSFER,true);
			curl_setopt($ch,CURLOPT_INFILE,$fh);
			curl_setopt($ch,CURLOPT_INFILESIZE,$file_size);
			curl_setopt($ch,CURLOPT_HTTPHEADER,
				array(
				"Content-Type: application/octet-stream", 	// 更改content-type
				"Expect:" // 删除HTTP头中的Expect:域，因为lighttpd不识别该域中的100-continue内容
				)
			);
			curl_exec($ch);
			$rc=curl_getinfo($ch,CURLINFO_HTTP_CODE);
			if($rc>=200 && $rc<300) {
				// 操作成功
				$result=true;
			}
			curl_close($ch);
		} else {
			error_log("WARN: CURL initialization failed for URL: $url");
		}

		fclose($fh);
		return $result;
	}

}
?>
