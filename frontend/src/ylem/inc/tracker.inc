<?php
// vi:ft=php ts=4 sw=4
/**
	YLEM Tracker交互接口
 */
if(!defined("YLEM_FE_TRACKER")) {
	define("YLEM_FE_TRACKER",1);

	include("cache.inc");

	function tracker_cached_get_paths($host,$domain,$vol,$key,$timeout=0.1)
	{
		$mc=cache_connect();
		$cache_key="meta:$domain:$vol:$key";
		$val=cache_get($mc,$cache_key);
		if(!$val) {
			// 未在缓存系统中发现对应文件的存储位置记录，连接Tracker进行实际的查询
			$fh=tracker_connect($host,$timeout);
			if($fh) {
				// 尝试获取待下载文件的存储位置
				$res=tracker_get_paths($fh,$domain,$key);
				if(count($res["paths"])>0) {
					// 获得了文件存储位置列表，将其置入缓存，缓存时间1个小时
					cache_set($mc,$cache_key,serialize($res),3600);
					return $res;
				} else {
					// 无法获取文件存储位置列表
					error_log("WARN: Failed to get file storage locations for '$resid'");
				}
			} else {
				error_log("WARN: Cannot connect to tracker service in volume '$vol'");
			}

			// 获取文件存储位置失败
			return false;
		} else {
			// 在缓存系统中发现了对应文件的存储位置信息，返回之
			return unserialize($val);
		}
	}

	/**
		连接指定主机上的Tracker服务
		@param $host Tracker主机名，一般指定为一个VIP域名。
		@param $timeout 可选的连接超时时间，默认为0.1s。
		@ret 连接成功时返回连接句柄，连接失败时返回false。
	 */
	function tracker_connect($host,$timeout=0.1)
	{
		$fh=fsockopen($host,YLEM_TRACKER_PORT,$errno,$errstr,$timeout);
		return $fh;
	}

	/**
		关闭给定的Tracker服务连接。
		@param $fh Tracker服务连接句柄。
	 */
	function tracker_close($fh)
	{
		fclose($fh);
	}

	/**
		发送CREATE_OPEN命令准备创建文件，获得可能的存储位置列表
		@param $fh Tracker服务连接句柄。
		@param $domain 待创建文件所在命名空间。
		@param $key 待创建文件的名称。
		@ret array($fid,$dev_cands,$st_cands)，其中$fid为待创建文件的卷内唯一ID，$dev_cands为可能
		的存储设备ID列表，$st_cands为可能的存储位置URL列表，二者是一一对应的。操作失败时返回值为
		array(false,array(),array())。
	 */
	function tracker_create_open($fh,$domain,$key)
	{
		$domain=urlencode($domain);
		$key=urlencode($key);

		// 向Tracker发送CREATE_OPEN命令
		// 由于以\n结尾，根据文件流系统默认的行缓冲特性会自动被flush，因此不进行显式flush操作
		fprintf($fh,"create_open domain=%s&key=%s&multi_dest=1\r\n",$domain,$key);

		// 获取Tracker服务响应，最长接收16KB
		// 注意这里stream_get_line()一定要给出\r\n作为结束符，若用默认的结束符则会无限等待下去。
		$ret=stream_get_line($fh,16384,"\r\n");
		if($ret) {
			// 响应获取成功
			list($status,$data)=explode(" ",$ret,2);
			if(!strcasecmp($status,"OK")) {
				// 响应为成功信息，解析数据
				$kvs=parse_kvs($data);

				// 收集devid和path信息
				$st_cands=array();
				$dev_cands=array();
				for($i=1;$i<=$kvs["dev_count"];++$i) {
					$dev_cands[]=$kvs["devid_{$i}"];
					$st_cands[]=$kvs["path_{$i}"];
				}
				return array($kvs["fid"],$dev_cands,$st_cands);
			} else {
				// 响应为错误信息
				error_log("WARN: Got error response for CREATE_OPEN from Tracker: "
					.urldecode($data));
				return array(false,array(),array());
			}
		} else {
			// 响应获取失败
			error_log("WARN: Failed to get response for CREATE_OPEN from Tracker");
			return array(false,array(),array());
		}
	}

	/**
		发送CREATE_CLOSE命令结束文件创建过程
		@param $fh Tracker服务连接句柄。
		@param $domain 创建完毕的文件所在命名空间。
		@param $fid 创建完毕的文件使用的卷内唯一ID。
		@param $devid 创建完毕的文件所在的存储设备ID。
		@param $path 创建完毕的文件所在的存储位置URL。
		@param $key 创建完毕的文件名。
		@param $mimetype 创建完毕的文件的MIME类型字符串。
		@param $size 创建完毕的文件大小。
		@ret 文件创建过程成功结束时返回true，否则返回false。
	 */
	function tracker_create_close($fh,$domain,$fid,$devid,$path,$key,$mimetype,$size)
	{
		$domain=urlencode($domain);
		$fid=urlencode($fid);
		$devid=urlencode($devid);
		$path=urlencode($path);
		$key=urlencode($key);
		$mimetype=urlencode($mimetype);
		$size=urlencode($size);

		// 向Tracker服务发送CREATE_CLOSE命令
		// 由于以\n结尾，根据文件流系统默认的行缓冲特性会自动被flush，因此不进行显式flush操作
		fprintf($fh,"create_close domain=%s&fid=%s&devid=%s&path=%s&key=%s&mimetype=%s&size=%s\r\n",
			$domain,$fid,$devid,$path,$key,$mimetype,$size);

		// 获取Tracker服务响应，最长接收16KB
		// 注意这里stream_get_line()一定要给出\r\n作为结束符，若用默认的结束符则会无限等待下去。
		$ret=stream_get_line($fh,16384,"\r\n");
		if($ret) {
			// 响应获取成功
			list($status,$data)=explode(" ",$ret,2);
			if(!strcasecmp($status,"OK")) {
				// 响应成功
				return true;
			} else {
				// 响应为错误信息
				error_log("WARN: Got error response for CREATE_CLOSE from Tracker: "
					.urldecode($data));
				return false;
			}
		} else {
			// 响应获取失败
			error_log("WARN: Failed to get response for CREATE_CLOSE from Tracker");
			return false;
		}
	}

	/**
		删除指定的文件。
		@param $fh Tracker服务连接句柄。
		@param $domain 待删除文件所在命名空间。
		@param $key 待删除的文件名。
		@ret 文件删除成功时返回true，否则返回false。
	 */
	function tracker_delete($fh,$domain,$key)
	{
		$domain=urlencode($domain);
		$key=urlencode($key);

		// 向Tracker服务发送DELETE命令
		// 由于以\n结尾，根据文件流系统默认的行缓冲特性会自动被flush，因此不进行显式flush操作
		fprintf($fh,"delete domain=%s&key=%s\r\n",$domain,$key);

		// 获取Tracker服务响应，最长接收16KB
		// 注意这里stream_get_line()一定要给出\r\n作为结束符，若用默认的结束符则会无限等待下去。
		$ret=stream_get_line($fh,16384,"\r\n");
		if($ret) {
			// 响应获取成功
			list($status,$data)=explode(" ",$ret,2);
			if(!strcasecmp($status,"OK")) {
				// 响应成功
				return true;
			} else {
				// 响应为错误信息
				error_log("WARN: Got error response for DELETE from Tracker: "
					.urldecode($data));
				return false;
			}
		} else {
			// 响应获取失败
			error_log("WARN: Failed to get response for DELETE from Tracker");
			return false;
		}
	}

	/**
		获取给定文件的存储位置列表。
		@param $domain 待读取文件所在命名空间。
		@param $key 待读取文件名称。
		@ret array("mimetype"=>$type,"paths"=>$paths)，其中$type为该文件的MIME type，
		$paths为该文件的存储位置URL列表，若获取列表失败则返回array("mimetype"=>false,
		"paths"=>array())。
	 */
	function tracker_get_paths($fh,$domain,$key)
	{
		$domain=urlencode($domain);
		$key=urlencode($key);

		// 向Tracker服务器发送GET_PATHS命令
		// 由于以\n结尾，根据文件流系统默认的行缓冲特性会自动被flush，因此不进行显式flush操作
		fprintf($fh,"get_paths domain=%s&key=%s&noverify=1&pathcount=3\r\n",$domain,$key);

		// 获取Tracker服务响应，最长接收16KB
		// 注意这里stream_get_line()一定要给出\r\n作为结束符，若用默认的结束符则会无限等待下去。
		$ret=stream_get_line($fh,16384,"\r\n");
		if($ret) {
			// 响应获取成功
			list($status,$data)=explode(" ",$ret,2);
			if(!strcasecmp($status,"OK")) {
				// 响应为成功信息，解析数据
				$kvs=parse_kvs($data);

				$paths=$kvs["paths"];
				$mimetype=$kvs["mimetype"];
				$res=array();
				for($i=1;$i<=$paths;++$i) {
					$path=$kvs["path".$i];
					$res[]=$path;
				}
				return array("mimetype"=>$mimetype,"paths"=>$res);
			} else {
				// 响应为错误信息
				error_log("WARN: Got error response for GET_PATHS from Tracker: "
					.urldecode($data).", request ".$fh);
				return array("mimetype"=>false,"paths"=>array());
			}
		} else {
			// 响应获取失败
			error_log("WARN: Failed to get response for GET_PATHS from Tracker, request ".$fh);
			return array("mimetype"=>false,"paths"=>array());
		}
	}

	function parse_kvs($data)
	{
		$ps=array_filter(explode("&",$data));
		$kvs=array();
		foreach($ps as $pair) {
			list($key,$val)=explode("=",$pair,2);
			$kvs[strtolower($key)]=urldecode($val);
		}
		return $kvs;
	}

}
?>
