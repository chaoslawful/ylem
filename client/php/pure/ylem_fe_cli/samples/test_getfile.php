<?php
include("ylem_fe_cli/ylem.inc");

// 请求返回数据为xml格式，默认为json格式
$ylm = new ylem("xml");

// namespace需要提前申请，test只对测试环境有效
$namespace = "test";
// 卷ID(0)-文件ID(0002w.jpg)
$id = "0-0001w.jpg";

// 下载文件
// get函数参数含义，参考ylem/ylem.inc中get函数的定义
$data = $ylm->get($namespace,$id,false,false,"http://www.yahoo.com.cn",24*60*60*365*10);

// 显示图片,0-0001w.jpg是测试环境中上传得图片
// 实际应用中，需要为自己需要的形式
echo "<img src=".$data.">";

?>
