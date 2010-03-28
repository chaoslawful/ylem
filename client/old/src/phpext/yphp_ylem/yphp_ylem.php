<?php

$object = new YlemClient;
$b = $object->connect("10.62.162.75",6061,'myDomain', 'my_class');
var_dump($b);
$data = ylem_listkeys('','',100);
var_dump($data);
$object->close();

//$b = ylem_connect('10.62.162.75', 6061, 'myDomain', 'my_class');
//var_dump($b);

//$data = ylem_listfids(0,100);
//var_dump($data);
//$data = ylem_listkeys('','',100);
//$data = ylem_listfids(0,100);
//$b = ylem_put("abc","/tmp/file/2m.JPG");
//var_dump($b);
//$data = ylem_get("abc");
//var_dump($data);

//ylem_close();

/*
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('yphp_ylem')) {
	dl('yphp_ylem.' . PHP_SHLIB_SUFFIX);
}
$module = 'yphp_ylem';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
//$function = 'confirm_' . $module . '_compiled';
//if (extension_loaded($module)) {
//	$str = $function($module);
//} else {
//	$str = "Module $module is not compiled into PHP";
//}
//echo "$str\n";
//
 */
?>
