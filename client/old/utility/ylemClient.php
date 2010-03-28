<?php

define("HOST","ds1.platform.yahoo.com");
define("PORT",7001);
define("DOMAIN","myDomain");
define("CLASSNAME","my_class");

// putfile 上传文件
function putfile($key,$filename)
{
	$ylem_object = new YlemClient;
	$b = $ylem_object->connect(HOST,PORT,DOMAIN,CLASSNAME); 
	if(!$b){
		echo "connect err\n";
		return;
	}

	$b = $ylem_object->putfile($key,$filename);
	if($b){
		echo $key." -- OK \n";	
	}else{
		echo $key." -- fail \n";	
	}
}

// 删除所有文件
function deletefile($key)
{
	$ylem_object = new YlemClient;
	$b = $ylem_object->connect(HOST,PORT,DOMAIN,CLASSNAME); 
	if(!$b){
		echo "connect err\n";
		return;
	}

	if(strlen($key) == 0)
	{// 删除所有key
		while(1)
		{
			$data = $ylem_object->listkeys("","",1000);
			if($data == false)
			{
				echo "listkeys err..\n";
				break;
			}else{
				if(isset($data["key_count"]))
				{
					$data["key_count"]="";
					$data["next_after"]="";
					foreach($data as $key)
					{
						if($key == "")
							continue;
						$b = $ylem_object->delete($key);
						if($b)
							echo "delete file ".$key."\n";
						else
							echo "not delete file ".$key."\n";
					}
				}else{
					break;
				}
			}
		}
	}else{
		// 删除特定的key
		$b = $ylem_object->delete($key);
		if($b)
			echo "delete file ".$key."\n";
		else
			echo "not delete file ".$key."\n";
	}
}

// getpaths获取key的weburi
function getpaths($key)
{	
	$ylem_object = new YlemClient;
	$b = $ylem_object->connect(HOST,PORT,DOMAIN,CLASSNAME); 
	if(!$b){
		echo "connect err\n";
		return;
	}

	$data = $ylem_object->getpaths($key);
	if($data != false){
		echo "found ".$key."\n";
		print_r ($data);
	}
	else
		echo "not found ".$key."\n";
}

// getkeys获取key的weburi
function getkeys($key)
{	
	$ylem_object = new YlemClient;
	$b = $ylem_object->connect(HOST,PORT,DOMAIN,CLASSNAME); 
	if(!$b){
		echo "connect err\n";
		return;
	}

	$data = $ylem_object->listkeys($key,"",100);
	if($data != false)
		print_r ($data);
	else
		echo "not found ".$key."\n";
}

// listkeys获取key的列表
function listkeys($prfiex="",$limit=100)
{	
	$ylem_object = new YlemClient;
	$b = $ylem_object->connect(HOST,PORT,DOMAIN,CLASSNAME); 
	if(!$b){
		echo "connect err\n";
		return;
	}

	$data = $ylem_object->listkeys($prfiex,"",$limit);
	if($data != false)
		var_dump ($data);
	else
		echo "not found data from ylemServer\n";
}

function main($argc,$argv)
{
	if($argc < 2)
	{
		echo "sudo ./ylemClient method [key] [filepath] [num]\n";
		echo "method: ylemClient support 4 methods\n";
		echo "       delete: delete a key \n";
		echo "       put: put file to ylemServer\n";
		echo "       getpaths: get file paths\n";
		echo "       getkeys: get keys\n";
		echo "key: that key is string \n";
		echo "put method will use [filepath] and [num] paramerters\n";
		die();
	}else{
		$method = strtolower($argv[1]);
		if($method == "delete")       // delete 文件方法
		{
			$key="";
			if($argc > 2)
				$key = $argv[2];
			deletefile($key);
		}else if($method == "put"){   // put 上传文件
			if($argc < 4)
				die("put method err\n");
			$key = $argv[2];
			$filename = $argv[3];
			$iNum=0;
			if($argc == 5)
				$iNum = $argv[4];
			if($iNum == 0)
				putfile($key,$filename);
			else{
				for($i=0; $i < $iNum; ++$i)
				{
					$filekey = $key.$i;
					putfile($filekey,$filename);
				}
			}
		}else if($method == "getpaths"){
			if($argc < 3){
				die( "method getpaths err\n");
			}
			$Num = 0;
			if($argc == 4)
				$Num = $argv[3];
			if($Num == 0)
				getpaths($argv[2]);
			else{
				for($i=0; $i<$Num; ++$i)
				{
					getpaths($argv[2].$i);
				}
			}
		}else if ($method == "getkeys"){
			if($argc < 3){
				die ("method getpaths err\n");
			}
			getkeys($argv[2]);
		}else{
			echo "list some keys\n";
			listkeys("",10);
			echo $method." no support method\n";
		}
	}
}

main($argc,$argv);

?>
