#!/usr/bin/perl -w

#$filename = "filename";

#上传文件 
for ($i = 0; $i < 1; $i+=1)
{
	system('./ylemClient put 2M'.$i.' /tmp/file/2m.JPG 10 > cpp'.$i.'.ini &');
}
