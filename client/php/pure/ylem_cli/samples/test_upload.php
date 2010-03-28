<?php
require("ylem_cli/ylem.inc");

$tracker_host = "127.0.0.1";
$volume = "0";
$namespace = isset($argv[1]) ? $argv[1] : "test";
$filename = isset($argv[2]) ? $argv[2] : "test.jpg";
$mimetype = isset($argv[3]) ? $argv[3] : "application/octet-stream";

$file_size = filesize($filename);

$handle = tracker_connect($tracker_host);
if($handle) {
	list($fid, $dev_cands, $st_cands) = tracker_create_open($handle, $namespace, $filename);

	$st_cand_cnt = count($st_cands);
	if($st_cand_cnt > 0) {
		// YLEM 中有可用的上传存储位置
		$st_idx = -1;
		for($i = 0; $i < $st_cand_cnt; ++$i) {
			if(store_upload($st_cands[$i], $filename, $file_size)) {
				// 文件成功上传至 YLEM 存储节点
				$st_idx = $i;
				break;
			}
		}

		if($st_idx != -1) {
			if(tracker_create_close(
				$handle,
				$namespace,
				$fid,
				$dev_cands[$st_idx],
				$st_cands[$st_idx],
				$filename,
				$mimetype,
				$file_size
			)) {
				// YLEM 文件创建成功结束
				echo "SUCCESS: File uploaded: filename='$filename', namespace='$namespace', mimetype='$mimetype'\n";
			} else {
				echo "ERROR: CREATE_CLOSE operation failed for file $filename\n";
			}
		} else {
			echo "ERROR: No usable storage position to save file $filename\n";
		}
	} else {
		echo "ERROR: CREATE_OPEN operation failed for file $filename\n";
	}

	tracker_close($handle);
} else {
	echo "ERROR: Failed to connect to YLEM tracker ", $tracker_host, "\n";
}

// vi:ft=php ts=4 sw=4
?>
