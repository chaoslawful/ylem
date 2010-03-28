<?php

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
