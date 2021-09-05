<?php
define("INT_DAILY", 60*60*24*2);
define("INT_WEEKLY", 60*60*24*8);
define("INT_MONTHLY", 60*60*24*35);
define("INT_YEARLY", 60*60*24*400);

define("XOFFSET", 90);
define("YOFFSET", 45);

require("config.conf");

function ConnectDb()
    {
	global $db_connect_string;

    $db = pg_pconnect($db_connect_string);
    if (!$db)
        {
        printf("DB Error, could not connect to database");
        exit(1);
        }
    return($db);
    }
                                                                                                                             
function fmtb($kbytes)
	{
	$Max = 1024;
	$Output = $kbytes;
	$Suffix = 'K';

	if ($Output > $Max)
		{
		$Output /= 1024;
		$Suffix = 'M';
		}

	if ($Output > $Max)
		{
		$Output /= 1024;
		$Suffix = 'G';
		}

	if ($Output > $Max)
		{
		$Output /= 1024;
		$Suffix = 'T';
		}

	return(sprintf("<td align=right><tt>%.1f%s</td>", $Output, $Suffix));
	}

$starttime = time();
set_time_limit(300);
?>
