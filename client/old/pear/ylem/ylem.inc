<?php

include("ylem/config.inc");
include("ylem/function.inc");
include("ylem/ylemcli.inc");

// 各个接口参数说明参考
// http://twiki.corp.cnb.yahoo.com:9999/twiki/bin/view/PlatForm/YlemFrontendInterface

// class ylemClient Packaging of the ylem system get,put,rm three Interface
class ylem{
	var $_format;
	var $response_format;
	function ylem($format="json")
	{
		$this->_format = $format;
		$this->response_format = "&ret=".$format;
	}

	// 上传文件接口
	// ns 上传文件所属的命名空间，需要预先申请
	// $files 上传文件内容，具体格式
	/*
		Array
		(
			[file1] => Array
			(
				[filename] => 0001w.jpg
				[mimetype] => pplication/octet-stream
				[content] => file content
			)

			[file2] => Array
			(
				[filename] => 0002w.jpg
				[mimetype] => pplication/octet-stream
				[content] => file content
			)

		)
	 */
	function put($ns,$files)
	{
		if(empty($files))
			return "files param error";	

		// 发送URL
		$url = PUTFILE;
		$url .= "&ns=".$ns;
		$url .= $this->response_format;
		// 发送消息头
		$header[] = "Content-Type: multipart/form-data; boundary=".BOUNDARY;
		// 生成发送数据
		$vars="";
		foreach($files as $file)
		{
			$vars.="\r\n--".BOUNDARY."\r\n";
			// 获取文件名
			if(isset($file['filename']))
			{
				$vars.="Content-Disposition: form-data; name=\"f12345[]\"; filename=\"".$file['filename']."\"\r\n";
			}else{
				return "files param lost filename";
			}
			// 获取MIMETYPE
			if(isset($file['mimetype']))
			{
				$vars.= "Content-Type:".$file['mimetype']."\r\n";
			}else{
				return "files param lost mimetype";
			}
			// 获取文件内容
			if(isset($file['content']))
			{
				$vars .= "\r\n";
				$vars.=$file['content']."\r\n";
				$vars .= "\r\n";
			}else{
				return "files param lost content";
			}
			$vars.="--".BOUNDARY;
		}
		$vars.="--\r\n\r\n";
//		print_r($vars);

		return post($url,$vars,$header);	
	}
	// 获取文件接口
	// ns 待下载文件所在的命名空间
	// id 待下载文件的完整id(卷id-文件id形式)
	// ct 指定相应头中Content-Type
	// fn 指定下载文件的文件名
	// rd 指定文件下载失败时重定向url
	function get($ns,$id,$ct=false,$fn=false,$rd=false)
	{
		$enc = ylem_gen_cipher($ns,$id,$ct,$fn,$rd);
		$url = GETFILE;
		$url .= "&enc=".$enc;
		return $url;
	}

	// 删除文件接口
	// $ns 待删除文件所属命名空间，需要预先申请
	// $id 待删除文件的完成(卷ID-文件ID的形式)列表，之间用逗号分割
	function remove($ns,$id)
	{
		$url = RMFILE;
		$url .= "&ns=".$ns;
		$url .= "&id=".$id;
		$url .= $this->response_format;

		return get($url);
	}
}

?>
