<?php
// vi:ft=php ts=4 sw=4
if(!defined("YLEM_FE_CONFIG")) {
	define("YLEM_FE_CONFIG",1);

	require("keydb.inc");

	define("YLEM_CACHE_HOST","127.0.0.1");
	define("YLEM_CACHE_PORT",11211);
	define("YLEM_CLMON_HOST","localhost");
	define("YLEM_CLMON_PORT",4100);
	define("YLEM_TRACKER_PORT",7001);
	define("YLEM_TRACKER_TIMEOUT",100);

}
?>
