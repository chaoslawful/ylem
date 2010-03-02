<?php
require("ylemcli/common.inc");

// vim:ft=php ts=4 sw=4
function ylem_gen_cipher($ns,$id,$ct=false,$fn=false,$rd=false,$ex=false)
{
	$data=array(
		"v"=>"1.0",
		"ns"=>$ns,
		"id"=>$id
	);
	if($ct) {
		$data["ct"]=$ct;
	}
	if($fn) {
		$data["fn"]=$fn;
	}
	if($rd) {
		$data["rd"]=$rd;
	}
	if($ex) {
		$data["ex"]=$ex;
	}
	$pairs=array();
	foreach($data as $k=>$v) {
		$pairs[]=$k."\001".$v;
	}
	$plain=implode("\002",$pairs);
	$secure=new Crypt_XXTEA();
	$secure->setKey(YLEM_URL_SECURE_KEY);
	$cipher=base64_encode_urlsafe($secure->encrypt($plain));
	return $cipher;
}

// get function
function get($url, $timeout=5) 
{
    $ch = curl_init() or die("function curl_init error\n");
    curl_setopt($ch, CURLOPT_URL,$url);
    curl_setopt($ch, CURLOPT_HEADER, 0); 
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
    curl_setopt($ch, CURLOPT_TIMEOUT,$timeout);
    $data = curl_exec($ch);
    curl_close($ch);
    return $data;
}

// post function
function post($url, $vars, $header,$timeout=5) 
{
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
    curl_setopt($ch, CURLOPT_URL,$url);
	curl_setopt($ch, CURLOPT_POST, 1 );
	curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $vars);
    curl_setopt($ch, CURLOPT_TIMEOUT,$timeout);
    $data = curl_exec($ch);
//		echo "\nGet Code is ".curl_getinfo($ch,CURLINFO_HTTP_CODE)."\n";
    curl_close($ch);
    return $data;
}

?>
