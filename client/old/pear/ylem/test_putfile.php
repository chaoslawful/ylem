<?php

// 各个接口参数说明参考
// http://twiki.corp.cnb.yahoo.com:9999/twiki/bin/view/PlatForm/YlemFrontendInterface

include("ylem/ylem.inc");

// 请求返回数据为xml格式，默认为json格式
$ylm = new ylem("xml");

$namespace = "test";

// 上传文件

$files = array();
////////////////////////////////////////////////////
// filename,mimetype,content为内部关键字，不可修改。
$files['file1']['filename']="0007w.jpg";
$files['file1']['mimetype']="application/octet-stream";
// 确保/tmp/example/abc.jpg存在
$file = '/tmp/example/abc.jpg';
$fh = fopen($file, 'rb');
$contents = fread($fh, filesize($file));
fclose($fh);
$files['file1']['content']=$contents;

$files['file2']['filename']="0008w.jpg";
$files['file2']['mimetype']="application/octet-stream";
// 确保/tmp/example/abc.jpg存在
$file = '/tmp/example/abc.jpg';
$fh = fopen($file, 'rb');
$contents = fread($fh, filesize($file));
fclose($fh);
$files['file2']['content']=$contents;

//////////////////////////////////////////////////////

// 上传文件，并打印上传结果
// 具体应用中需要分析返回结果，记录vol的ID通过vol ID和文件名，
// 确定文件唯一ID
echo $ylm->put($namespace,$files)."\n";

?>
