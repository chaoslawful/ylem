<?php
require("ylem_cli/ylem.inc");

$tracker_host = "127.0.0.1";
$volume = "0";
$namespace = isset($argv[1]) ? $argv[1] : "test";
$filename = isset($argv[2]) ? $argv[2] : "test.jpg";

$handle = tracker_connect($tracker_host);
if($handle) {
	if(tracker_delete($handle, $namespace, $filename)) {
		echo "SUCCESS: File removed: namespace='$namespace', filename='$filename'\n";
	} else {
		echo "ERROR: Failed to remove file $filename\n";
	}

	tracker_close($handle);
} else {
	echo "ERROR: Failed to connect to YLEM tracker $tracker_host\n";
}

// vi:ft=php ts=4 sw=4
?>
