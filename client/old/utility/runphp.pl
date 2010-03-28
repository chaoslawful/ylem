#!/usr/bin/perl -w

#$filename = "filename";
  
for ($i = 0; $i < 1; $i+=1)
{
	system('php ylemClient.php put 2M'.$i.' /tmp/file/2m.JPG 10 > php'.$i.'.ini &');
}
