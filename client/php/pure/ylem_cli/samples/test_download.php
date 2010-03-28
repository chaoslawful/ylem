<?php
require("ylem_cli/ylem.inc");

$tracker_host = "127.0.0.1";
$volume = "0";
$namespace = defined($argv[1]) ? $argv[1] : "test";
$filename = defined($argv[2]) ? $argv[2] : "test.jpg";

$handle = tracker_connect($tracker_host);
if($handle) {
	$res = tracker_get_paths($handle, $namespace, $filename);
	if(count($res["paths"]) > 0) {
		// 显示 YLEM 保存的文件 MIME-Type
		echo "File MIME-Type: ", $res["mimetype"];

		$got_loc = false;
		$urls = $res["paths"];
		$url_cnt = count($urls);
		for($i = 0; $i < $url_cnt; ++$i) {
			$size = store_get_file_size($urls[$i]);
			if($size >= 0) {
				// 当前下载 URL 可用，下载文件内容并回显到 stdout
				store_echo_file($urls[$i]);
				$got_loc = true;
				break;
			}
		}

		if(!$got_loc) {
			echo "ERROR: Failed to download file $filename\n";
		}
	} else {
		echo "ERROR: Failed to get download URL(s) for file $filename\n";
	}

	tracker_close($handle);
} else {
	echo "ERROR: Failed to connect to YLEM tracker $tracker_host\n";
}

// vi:ft=php ts=4 sw=4
?>
