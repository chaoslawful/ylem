<?php

// 测试服务器host
define('YLEMSERVER',"http://h08-vm02.corp.cnb.yahoo.com");

// 下载文件接口
define('GETFILE',YLEMSERVER.":80/ylem/get?version=1.0");
// 删除文件接口
define('RMFILE',YLEMSERVER.":4080/ylem/rm?version=1.0");
// 上传文件接口
define('PUTFILE',YLEMSERVER.":4080/ylem/ul?version=1.0");

// 上传多个文件时边界标示字符串
define('BOUNDARY',"====")
?>
