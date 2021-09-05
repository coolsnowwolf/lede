<?php
error_reporting(E_ALL & ~(E_NOTICE|E_STRICT));
define("INT_DAILY", 60*60*24*2);
define("INT_WEEKLY", 60*60*24*8);
define("INT_MONTHLY", 60*60*24*35);
define("INT_YEARLY", 60*60*24*400);

define("XOFFSET", 90);
define("YOFFSET", 45);

require("config.conf.php");

function ConnectDb() {
    global $db_connect_string;
    try {
        $db = new PDO($db_connect_string);
    } catch (PDOException $ex) {
        die("DB Error, could not connect to database: "  . $ex->getMessage());
    }
    return $db;
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

function ip2s32($ip) {
    $i = ip2long($ip);
    return ($i & 0x80000000 ? '-' . ((~$i & 0x7fffffff)+1) : ''. ($i & 0x7fffffff));
}

function prepare_sql_subnet($subnet) {
    list($snet, $smask) = explode('/', $subnet);
    $inet = ip2s32($snet);
    if($smask > 0 && $smask < 32) {
        $mask = -1 << (32 - (int)$smask);
        return "and (ip & $mask = $inet)";
    } elseif ($inet) {
        return "and ip = " . $inet;
    }
    return "";
}

$starttime = time();
set_time_limit(300);
?>
