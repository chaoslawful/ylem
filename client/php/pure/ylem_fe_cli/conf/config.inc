<?php
require("keydb.inc");

define("YLEMSERVER_EXT","http://first.wdtong.com");
define('YLEMSERVER',"http://127.0.0.1");

define('GETFILE',YLEMSERVER_EXT."/ylem/get?version=1.0");	// 下载文件接口
// 内部下载文件接口
define('GETFILE_INT',YLEMSERVER.":4080/ylem/get?version=1.0");
// 删除文件接口
define('RMFILE',YLEMSERVER.":4080/ylem/rm?version=1.0");
// 上传文件接口
define('PUTFILE',YLEMSERVER.":4080/ylem/ul?version=1.0");

// 上传多个文件时边界标示字符串
define('BOUNDARY',"====");

// vi:ft=php ts=4 sw=4
?>
