<?php
require("include.php");

// Returns x location of any given timestamp
function ts2x($ts)
	{
	global $timestamp, $width, $interval;
	return(($ts-$timestamp)*(($width-XOFFSET) / $interval) + XOFFSET);
	}

// If we have multiple IP's in a result set we need to total the average of each IP's samples
function AverageAndAccumulate()
	{
	global $Count, $total, $icmp, $udp, $tcp, $ftp, $http, $p2p, $YMax;
	global $a_total, $a_icmp, $a_udp, $a_tcp, $a_ftp, $a_http, $a_p2p;

	foreach ($Count as $key => $number)
    	{
	    $total[$key] /= $number;
    	$icmp[$key] /= $number;
    	$udp[$key] /= $number;
    	$tcp[$key] /= $number;
    	$ftp[$key] /= $number;
    	$http[$key] /= $number;
    	$p2p[$key] /= $number;
    	}

	foreach ($Count as $key => $number)
		{
		$a_total[$key] += $total[$key];
		$a_icmp[$key] += $icmp[$key];
		$a_udp[$key] += $udp[$key];
		$a_tcp[$key] += $tcp[$key];
		$a_ftp[$key] += $ftp[$key];
		$a_http[$key] += $http[$key];
		$a_p2p[$key] += $p2p[$key];

		if ($a_total[$key] > $YMax)
			$YMax = $a_total[$key];
		}

	unset($GLOBALS['total'], $GLOBALS['icmp'], $GLOBALS['udp'], $GLOBALS['tcp'], $GLOBALS['ftp'], $GLOBALS['http'], $GLOBALS['p2p'], $GLOBALS['Count']);

	$total = array();
	$icmp = array();
	$udp = array();
	$tcp = array();
	$ftp = array();
	$http = array();
	$p2p = array();
	$Count = array();
	}

$db = ConnectDb();

// Get parameters

if (isset($_GET['width']))
    $width = $_GET['width'];
else
	$width = DFLT_WIDTH;

if (isset($_GET['height']))
    $height = $_GET['height'];
else
	$height = DFLT_HEIGHT;

if (isset($_GET['interval']))
    $interval = $_GET['interval'];
else
	$interval = DFLT_INTERVAL;

if (isset($_GET['ip']))
    $ip = $_GET['ip'];
else
	exit(1);

if (isset($_GET['sensor_name']))
	$sensor_name = $_GET['sensor_name'];
else
	exit(1);

if (isset($_GET['timestamp']))
    $timestamp = $_GET['timestamp'];
else
	$timestamp = time() - $interval + (0.05*$interval);

if (isset($_GET['table']))
    $table = $_GET['table'];
else
	$table = "bd_rx_log";

if (isset($_GET['yscale']))
    $yscale = $_GET['yscale'];

$total = array();
$icmp = array();
$udp = array();
$tcp = array();
$ftp = array();
$http = array();
$p2p = array();
$Count = array();

// Accumulator
$a_total = array();
$a_icmp = array();
$a_udp = array();
$a_tcp = array();
$a_ftp = array();
$a_http = array();
$a_p2p = array();

$sql_subnet = prepare_sql_subnet($ip);
$sql = "select *, timestamp as ts from sensors, $table where sensors.sensor_id = ".$table.".sensor_id $sql_subnet and sensor_name = '$sensor_name' and timestamp > $timestamp and timestamp < ".($timestamp+$interval)." order by ip;";
//error_log($sql);
$result = $db->query($sql);

// The SQL statement pulls the data out of the database ordered by IP address, that way we can average each
// datapoint for each IP address to provide smoothing and then toss the smoothed value into the accumulator
// to provide accurate total traffic rate.

while ($row = $result->fetch())
	{
	if ($row['ip'] != $last_ip)
		{
		AverageAndAccumulate();
		$last_ip = $row['ip'];
		}

	$x = ($row['ts']-$timestamp)*(($width-XOFFSET)/$interval)+XOFFSET;
	$xint = (int) $x;

	//echo "xint: ".$xint."<br>";
	$Count[$xint]++;

	if ($row['total']/$row['sample_duration'] > $SentPeak)
		$SentPeak = $row['total']/$row['sample_duration'];
	$TotalSent += $row['total'];
	$total[$xint] += $row['total']/$row['sample_duration'];
	$icmp[$xint] += $row['icmp']/$row['sample_duration'];
	$udp[$xint] += $row['udp']/$row['sample_duration'];
	$tcp[$xint] += $row['tcp']/$row['sample_duration'];
	$ftp[$xint] += $row['ftp']/$row['sample_duration'];
	$http[$xint] += $row['http']/$row['sample_duration'];
	$p2p[$xint] += $row['p2p']/$row['sample_duration'];
	}

// One more time for the last IP
AverageAndAccumulate();

// Pull the data out of Accumulator
$total = $a_total;
$icmp = $a_icmp;
$udp = $a_udp;
$tcp = $a_tcp;
$ftp = $a_ftp;
$http = $a_http;
$p2p = $a_p2p;

$YMax += $YMax*0.05;    // Add an extra 5%

// if a y scale was specified override YMax
if (isset($yscale)&& $yscale > 0)
    $YMax = $yscale/8;

// Plot the data

header("Content-type: image/png");

$im = imagecreate($width, $height);
$white = imagecolorallocate($im, 255, 255, 255);
$purple = ImageColorAllocate($im, 255, 0, 255);
$green  = ImageColorAllocate($im, 0, 255, 0);
$blue   = ImageColorAllocate($im, 0, 0, 255);
$lblue  = ImageColorAllocate($im, 128, 128, 255);
$brown  = ImageColorAllocate($im, 128, 0, 0);
$red    = ImageColorAllocate($im, 255, 0, 0);
$black  = ImageColorAllocate($im, 0, 0, 0);

for($Counter=XOFFSET+1; $Counter < $width; $Counter++)
	{
	if (isset($total[$Counter]))
		{
		// Convert the bytes/sec to y coords
        $total[$Counter] = ($total[$Counter]*($height-YOFFSET))/$YMax;
		$tcp[$Counter] = ($tcp[$Counter]*($height-YOFFSET))/$YMax;
        $ftp[$Counter] = ($ftp[$Counter]*($height-YOFFSET))/$YMax;
		$http[$Counter] = ($http[$Counter]*($height-YOFFSET))/$YMax;
		$p2p[$Counter] = ($p2p[$Counter]*($height-YOFFSET))/$YMax;
        $udp[$Counter] = ($udp[$Counter]*($height-YOFFSET))/$YMax;
		$icmp[$Counter] = ($icmp[$Counter]*($height-YOFFSET))/$YMax;

		// Stack 'em up!
		// Total is stacked from the bottom
		// Icmp is on the bottom too
		// Udp is stacked on top of icmp
		$udp[$Counter] += $icmp[$Counter];
		// TCP and p2p are stacked on top of Udp
		$tcp[$Counter] += $udp[$Counter];
		$p2p[$Counter] += $udp[$Counter];
 		// Http is stacked on top of p2p
		$http[$Counter] += $p2p[$Counter];
		// Ftp is stacked on top of http
        $ftp[$Counter] += $http[$Counter];

		// Plot them!
		//echo "$Counter:".$Counter." (h-y)-t:".($height-YOFFSET) - $total[$Counter]." h-YO-1:".$height-YOFFSET-1;
        ImageLine($im, $Counter, ($height-YOFFSET) - $icmp[$Counter], $Counter, $height-YOFFSET-1, $red);
        ImageLine($im, $Counter, ($height-YOFFSET) - $udp[$Counter], $Counter, ($height-YOFFSET) - $icmp[$Counter] - 1, $brown);
        ImageLine($im, $Counter, ($height-YOFFSET) - $tcp[$Counter], $Counter, ($height-YOFFSET) - $udp[$Counter] - 1, $green);
        ImageLine($im, $Counter, ($height-YOFFSET) - $p2p[$Counter], $Counter, ($height-YOFFSET) - $udp[$Counter] - 1, $purple);
        ImageLine($im, $Counter, ($height-YOFFSET) - $http[$Counter], $Counter, ($height-YOFFSET) - $p2p[$Counter] - 1, $blue);
        ImageLine($im, $Counter, ($height-YOFFSET) - $ftp[$Counter], $Counter, ($height-YOFFSET) - $http[$Counter] - 1, $lblue);
		}
//	else
//		echo $Counter." not set<br>";
	}

// Margin Text
if ($SentPeak < 1024/8)
	$txtPeakSendRate = sprintf("Peak Send Rate: %.1f KBits/sec", $SentPeak*8);
else if ($SentPeak < (1024*1024)/8)
    $txtPeakSendRate = sprintf("Peak Send Rate: %.1f MBits/sec", ($SentPeak*8.0)/1024.0);
else
	$txtPeakSendRate = sprintf("Peak Send Rate: %.1f GBits/sec", ($SentPeak*8.0)/(1024.0*1024.0));

if ($TotalSent < 1024)
	$txtTotalSent = sprintf("Sent %.1f KBytes", $TotalSent);
else if ($TotalSent < 1024*1024)
	$txtTotalSent = sprintf("Sent %.1f MBytes", $TotalSent/1024.0);
else
	$txtTotalSent = sprintf("Sent %.1f GBytes", $TotalSent/(1024.0*1024.0));

ImageString($im, 2, XOFFSET+5,  $height-20, $txtTotalSent, $black);
ImageString($im, 2, $width/2+XOFFSET/2,  $height-20, $txtPeakSendRate, $black);

// Draw X Axis

ImageLine($im, 0, $height-YOFFSET, $width, $height-YOFFSET, $black);

// Day/Month Seperator bars

if ((24*60*60*($width-XOFFSET))/$interval > ($width-XOFFSET)/10)
	{
	$ts = getdate($timestamp);
	$MarkTime = mktime(0, 0, 0, $ts['mon'], $ts['mday'], $ts['year']);

    $x = ts2x($MarkTime);
    while ($x < XOFFSET)
    	{
        $MarkTime += (24*60*60);
	    $x = ts2x($MarkTime);
        }

    while ($x < ($width-10))
    	{
        // Day Lines
        ImageLine($im, $x, 0, $x, $height-YOFFSET, $black);
        ImageLine($im, $x+1, 0, $x+1, $height-YOFFSET, $black);

        $txtDate = strftime("%a, %b %d", $MarkTime);
        ImageString($im, 2, $x-30,  $height-YOFFSET+10, $txtDate, $black);

        // Calculate Next x
        $MarkTime += (24*60*60);
	    $x = ts2x($MarkTime);
        }
	}
else if ((24*60*60*30*($width-XOFFSET))/$interval > ($width-XOFFSET)/10)
	{
	// Monthly Bars
	$ts = getdate($timestamp);
	$month = $ts['mon'];
	$MarkTime = mktime(0, 0, 0, $month, 1, $ts['year']);

    $x = ts2x($MarkTime);
    while ($x < XOFFSET)
    	{
		$month++;
        $MarkTime = mktime(0, 0, 0, $month, 1, $ts['year']);
	    $x = ts2x($MarkTime);
        }

    while ($x < ($width-10))
    	{
        // Day Lines
        ImageLine($im, $x, 0, $x, $height-YOFFSET, $black);
        ImageLine($im, $x+1, 0, $x+1, $height-YOFFSET, $black);

        $txtDate = strftime("%b, %Y", $MarkTime);
        ImageString($im, 2, $x-25,  $height-YOFFSET+10, $txtDate, $black);

        // Calculate Next x
		$month++;
        $MarkTime = mktime(0, 0, 0, $month, 1, $ts['year']);
	    $x = ts2x($MarkTime);
        }
	}
else
	{
	// Year Bars
    $ts = getdate($timestamp);
    $year = $ts['year'];
    $MarkTime = mktime(0, 0, 0, 1, 1, $year);

    $x = ts2x($MarkTime);
    while ($x < XOFFSET)
        {
        $year++;
        $MarkTime = mktime(0, 0, 0, 1, 1, $year);
        $x = ts2x($MarkTime);
        }

    while ($x < ($width-10))
        {
        // Day Lines
        ImageLine($im, $x, 0, $x, $height-YOFFSET, $black);
        ImageLine($im, $x+1, 0, $x+1, $height-YOFFSET, $black);

        $txtDate = strftime("%b, %Y", $MarkTime);
        ImageString($im, 2, $x-25,  $height-YOFFSET+10, $txtDate, $black);

        // Calculate Next x
        $year++;
        $MarkTime = mktime(0, 0, 0, 1, 1, $year);
        $x = ts2x($MarkTime);
        }
	}

// Draw Major Tick Marks
if ((6*60*60*($width-XOFFSET))/$interval > 10) // pixels per 6 hours is more than 2
	$MarkTimeStep = 6*60*60; // Major ticks are 6 hours
else if ((24*60*60*($width-XOFFSET))/$interval > 10)
	$MarkTimeStep = 24*60*60; // Major ticks are 24 hours;
else if ((24*60*60*30*($width-XOFFSET))/$interval > 10)
	{
	// Major tick marks are months
	$MarkTimeStep = 0; // Skip the standard way of drawing major tick marks below

    $ts = getdate($timestamp);
    $month = $ts['mon'];
    $MarkTime = mktime(0, 0, 0, $month, 1, $ts['year']);

    $x = ts2x($MarkTime);
    while ($x < XOFFSET)
        {
        $month++;
        $MarkTime = mktime(0, 0, 0, $month, 1, $ts['year']);
        $x = ts2x($MarkTime);
        }

    while ($x < ($width-10))
        {
        // Day Lines
		$date = getdate($MarkTime);
		if ($date['mon'] != 1)
			{
	        ImageLine($im, $x, $height-YOFFSET-5, $x, $height-YOFFSET+5, $black);
    	    $txtDate = strftime("%b", $MarkTime);
        	ImageString($im, 2, $x-5,  $height-YOFFSET+10, $txtDate, $black);
          	}

        // Calculate Next x
        $month++;
        $MarkTime = mktime(0, 0, 0, $month, 1, $ts['year']);
        $x = ts2x($MarkTime);
        }
	}
else
	$MarkTimeStep = 0; // Skip Major Tick Marks

if ($MarkTimeStep)
	{
	$ts = getdate($timestamp);
	$MarkTime = mktime(0, 0, 0, $ts['mon'], $ts['mday'], $ts['year']);
	$x = ts2x($MarkTime);

	while ($x < ($width-10))
		{
    	if ($x > XOFFSET)
			{
    	    ImageLine($im, $x, $height-YOFFSET-5, $x, $height-YOFFSET+5, $black);
	        }
		$MarkTime += $MarkTimeStep;
	    $x = ts2x($MarkTime);
		}
	}

// Draw Minor Tick marks
if ((60*60*($width-XOFFSET))/$interval > 4) // pixels per hour is more than 2
	$MarkTimeStep = 60*60;  // Minor ticks are 1 hour
else if ((6*60*60*($width-XOFFSET))/$interval > 4)
	$MarkTimeStep = 6*60*60; // Minor ticks are 6 hours
else if ((24*60*60*($width-XOFFSET))/$interval > 4)
	$MarkTimeStep = 24*60*60;
else
	$MarkTimeStep = 0; // Skip minor tick marks

if ($MarkTimeStep)
	{
	$ts = getdate($timestamp);
	$MarkTime = mktime(0, 0, 0, $ts['mon'], $ts['mday'], $ts['year']);
	$x = ts2x($MarkTime);

	while ($x < ($width-10))
		{
    	if ($x > XOFFSET)
			{
    	    ImageLine($im, $x, $height-YOFFSET, $x, $height-YOFFSET+5, $black);
	        }
		$MarkTime += $MarkTimeStep;
	    $x = ts2x($MarkTime);
		}
	}

// Draw Y Axis
ImageLine($im, XOFFSET, 0, XOFFSET, $height, $black);

$YLegend = 'k';
$Divisor = 1;
if ($YMax*8 > 1024*2)
	{
    $Divisor = 1024;    // Display in m
    $YLegend = 'm';
    }

if ($YMax*8 > 1024*1024*2)
	{
    $Divisor = 1024*1024; // Display in g
    $YLegend = 'g';
	}

if ($YMax*8 > 1024*1024*1024*2)
	{
    $Divisor = 1024*1024*1024; // Display in t
    $YLegend = 't';
    }

$YStep = $YMax/10;
if ($YStep < 1)
	$YStep=1;
$YTic=$YStep;

while ($YTic <= ($YMax - $YMax/10))
	{
    $y = ($height-YOFFSET)-(($YTic*($height-YOFFSET))/$YMax);
	ImageLine($im, XOFFSET, $y, $width, $y, $black);
    $txtYLegend = sprintf("%4.1f %sbits/s", (8.0*$YTic)/$Divisor, $YLegend);
    ImageString($im, 2, 3, $y-7, $txtYLegend, $black);
	$YTic += $YStep;
	}

imagepng($im);
imagedestroy($im);
