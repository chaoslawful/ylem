<?php
// vi:ft=php ts=4 sw=4
/**
	公用函数库
 */
if(!defined("YLEM_FE_COMMON")) {
	define("YLEM_FE_COMMON",1);

	require("XXTEA.php");

	/**
		去除参数前后的空白，将无内容的参数统一置为false
		@param $param 待处理的参数。
		@ret 处理完毕的参数。
	 */
	function normalize_param($param)
	{
		if($param) {
			$param=trim($param);
			if(!strlen($param)) {
				$param=false;
			}
		} else {
			$param=false;
		}
		return $param;
	}

	/**
		判断给定的ID是否为完整的资源ID(即"vol_id-file_id"的形式)。
		@param $resid 待判断的ID。
		@ret 给定ID为完整的资源ID时返回true，否则返回false。
	 */
	function is_canonical_resid($resid)
	{
		if($resid) {
			return preg_match("/^[^-]+-[^-]+$/",$resid);
		} else {
			return false;
		}
	}

	/**
		YLEM前端可识别MIME type列表
	 */
	$ylem_mimetypes=array(
		"application/futuresplash"=>".spl",
		"application/ogg"=>".ogg",
		"application/pdf"=>".pdf",
		"application/pgp-signature"=>".sig",
		"application/postscript"=>".ps",
		"application/x-bittorrent"=>".torrent",
		"application/x-bzip"=>".bz2",
		"application/x-bzip-compressed-tar"=>".tar.bz2",
		"application/x-dvi"=>".dvi",
		"application/x-gzip"=>".gz",
		"application/x-java-archive"=>".jar",
		"application/x-ns-proxy-autoconfig"=>".pac",
		"application/x-shockwave-flash"=>".swf",
		"application/x-tar"=>".tar",
		"application/x-tgz"=>".tgz",
		"application/zip"=>".zip",
		"audio/mpeg"=>".mp3",
		"audio/x-mpegurl"=>".m3u",
		"audio/x-ms-wax"=>".wax",
		"audio/x-ms-wma"=>".wma",
		"audio/x-wav"=>".wav",
		"image/gif"=>".gif",
		"image/jpg"=>".jpg",
		"image/jpeg"=>".jpg",
		"image/png"=>".png",
		"image/x-xbitmap"=>".xbm",
		"image/x-xpixmap"=>".xpm",
		"image/x-xwindowdump"=>".xwd",
		"text/css"=>".css",
		"text/html"=>".htm",
		"text/javascript"=>".js",
		"text/plain"=>".txt",
		"text/xml"=>".xml",
		"video/mpeg"=>".mpg",
		"video/quicktime"=>".qt",
		"video/x-ms-asf"=>".asx",
		"video/x-msvideo"=>".avi",
		"video/x-ms-wmv"=>".wmv",
# default filename suffix
		"application/octet-stream"=>".bin"
	);

	/**
		对于给定的MIME type，获得一个对应的后缀名称(前面带有分隔点)
		@param $mimetype 给定的MIME type字符串，大小写不敏感。
		@ret 对应的后缀名称(例如".txt")，若给定MIME type不可识别则等同于application/octet-stream的后缀名称。
	 */
	function get_suffix_for_mimetype($mimetype)
	{
		global $ylem_mimetypes;
		$mimetype=strtolower($mimetype);
		if(!isset($ylem_mimetypes[$mimetype])) {
			return $ylem_mimetypes["application/octet-stream"];
		}
		return $ylem_mimetypes[$mimetype];
	}

	/**
		将给定的字符串进行URL安全的Base64编码(即除去结尾的=，并将+和/分别替换为*和-)。
		
		ref:http://zh.wikipedia.org/wiki/Base64

		@param $data 待编码的字符串。
		@retval 编码结果。
	 */
	function base64_encode_urlsafe($data)
	{
		return strtr(
			base64_encode($data),
			array(
				'='=>'',
				'+'=>'*',
				'/'=>'-'
			)
		);
	}

	/**
		将URL安全的Base64字符串解码为原始数据。
		
		ref:http://zh.wikipedia.org/wiki/Base64

		@param $enc 待解码的Base64字符串。
		@retval 原始数据字符串。
	 */
	function base64_decode_urlsafe($enc)
	{
		return base64_decode(
			strtr(
				$enc,
				array(
					'*'=>'+',
					'-'=>'/'
				)
			)
		);
	}

}
?>
