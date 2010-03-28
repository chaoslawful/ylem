<?php
if(!defined("YLEM_FE_CACHE")) {
	define("YLEM_FE_CACHE",1);

	include("conf/config.inc");

	function cache_connect()
	{
		$mc=new Memcache;
		$mc->addServer(YLEM_CACHE_HOST,YLEM_CACHE_PORT);
		return $mc;
	}

	function cache_set($mc,$key,$val,$expire=3600)
	{
		return $mc->set($key,$val,0,$expire);
	}

	function cache_get($mc,$key)
	{
		return $mc->get($key);
	}

}

// vim:ft=php ts=4 sw=4
?>
