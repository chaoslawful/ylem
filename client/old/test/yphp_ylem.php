<?php

define("HOST","ds1.platform.yahoo.com");
define("PORT",6061);
define("DOMAIN","myDomain");
define("CLASSNAME","my_class");

// 使用函数形式访问ylem
function useFunction()
{
	// 列出phpext导出函数

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
	 */
	echo "ylem_connect...\n";
	$b = ylem_connect(HOST,PORT,DOMAIN,CLASSNAME); 
	if(!$b){
		die("ylem_connect err\n");
	}

	echo "ylem_putfile...\n";	
	$b = ylem_putfile("phpextylem_putfile","yphp_ylem.php");
	if(!$b){
		die("ylem_putfile err\n");
	}

	echo "ylem_getpaths...\n";
	$data = ylem_getpaths("phpextylem_putfile");
	if($data != false)
		var_dump ($data);
	else
		die("ylem_getpaths err\n");

	echo "ylem_delete...\n";
	$b = ylem_delete("phpextylem_putfile");
	if(!$b)
		die("ylem_delete err\n");

	echo "ylem_listkeys...\n";
	$data = ylem_listkeys("","",5);
	if($data != false)
		var_dump($data);
	else
		die("ylem_listkeys err\n");

	echo "ylem_listfids...\n";
	$data = ylem_listfids(0,5);
	if($data != false)
		var_dump($data);
	else
		die("ylem_listfids err\n");


	echo "ylem_close...\n";
	ylem_close();
}
// 使用类对象访问ylem
function useObject()
{
	$ylem_object = new YlemClient;
	echo "connect...\n";
	$b = $ylem_object->connect(HOST,PORT,DOMAIN,CLASSNAME); 
	if(!$b){
		die("connect err\n");
	}

	echo "putfile...\n";	
	$b = $ylem_object->putfile("phpextylem_putfile","yphp_ylem.php");
	if(!$b){
		die("putfile err\n");
	}

	echo "getpaths...\n";
	$data = $ylem_object->getpaths("phpextylem_putfile");
	if($data != false)
		var_dump ($data);
	else
		die("getpaths err\n");

	echo "delete...\n";
	$b = $ylem_object->delete("phpextylem_putfile");
	if(!$b)
		die("delete err\n");

	echo "listkeys...\n";
	$data = $ylem_object->listkeys("","",5);
	if($data != false)
		var_dump ($data);
	else
		die("listkeys err\n");

	echo "listfids...\n";
	$data = $ylem_object->listfids(0,5);
	if($data != false)
		var_dump ($data);
	else
		die("listfids err\n");

	echo "ylem_close...\n";
	$ylem_object->close();
}

function main($argc,$argv)
{
	if($argc == 2)
	{
		if($argv[1] == "f" || $argv[1] == "F")
			useFunction();
	}else{
		useObject();
	}
}

main($argc,$argv);

?>
