<?php
// vi:ft=php ts=4 sw=4
/**
	YLEM集群监控服务接口
 */
if(!defined("YLEM_FE_MONITOR")) {
	define("YLEM_FE_MONITOR",1);

	include("config.inc");

	/**
		从YLEM集群监控服务获取卷列表。
		@ret 获取成功时返回结果为：
		<pre>
		array(	// 按评分从高到低排列的所有卷
			array(
				"vol" => "卷ID",
				"tracker" => "卷内tracker的VIP域名",
				"score" => "卷利用率综合评分，评分越高的卷越闲，评分为0的卷不可写入，但一般情况下仍可读取",
			),
			array(
				// 其他卷的信息
			),
			...
		)
		</pre>
		获取失败时返回false。
	 */
	function monitor_get_volumes()
	{
		$fh=fsockopen(YLEM_CLMON_HOST,YLEM_CLMON_PORT,$errno,$errstr,0.1);
		if(!$fh) {
			return false;
		} else {
			$str=stream_get_contents($fh);
			list($state,$json_str)=explode(" ",$str,2);
			if(!strcasecmp($state,"OK")) {
				$arr=json_decode($json_str,true);
				$res=$arr["res"];
				return $res;
			} else {
				error_log("WARN: Failed to get volume status from cluster monitor");
				return false;
			}
		}
	}

	/**
		为给定的卷信息列表生成一个反向索引结构，以便快速从给定的卷ID定位到卷信息的下标。
		@param $vol_lst 卷信息列表数组。
		@ret 卷信息反向索引数组，键为卷ID，值为对应的卷信息在给定列表中的下标。
	 */
	function monitor_build_volumes_index($vol_lst)
	{
		$idx=array();
		for($i=0;$i<count($vol_lst);++$i) {
			$idx[$vol_lst[$i]["vol"]]=$i;
		}
		return $idx;
	}

}
?>
