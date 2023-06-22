<?PHP

header('Content-type: text/plain; charset=utf8', true);

/* ===================================================================
 * Functions
   =================================================================== */

function check_header($name, $value = false) {
    if(!isset($_SERVER[$name])) {
        return false;
    }
    if($value && $_SERVER[$name] != $value) {
        return false;
    }
    return true;
}

function sendFile($path) {
    header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
    header('Content-Type: application/octet-stream', true);
    header('Content-Disposition: attachment; filename='.basename($path));
    header('Content-Length: '.filesize($path), true);
    header('x-MD5: '.md5_file($path), true);
    readfile($path);
}

function logMsg($logFile, $msg) {
    $log = "User: ".$_SERVER['REMOTE_ADDR'].' - '.date("F j, Y, g:i a").PHP_EOL.
    "Msg: ".$msg.PHP_EOL.
    "-------------------------".PHP_EOL;
    file_put_contents($logFile, $log, FILE_APPEND);
}

/* ===================================================================
 * Process request
   =================================================================== */
// Set file locations
$logFile = './logx_'.date("Y.n.j").'.log';
$filePath = "bin/";
$infoFile = "xremote_latest.txt";

// Check headers
if(!check_header('HTTP_USER_AGENT', 'ESP8266-http-Update')) {
    //echo "only for ESP8266 updater!\n";
    logMsg($logFile, "Invalid request.");
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    exit();
}

// Check ESP headers
if(
    !check_header('HTTP_X_ESP8266_STA_MAC') ||
    !check_header('HTTP_X_ESP8266_AP_MAC') ||
    !check_header('HTTP_X_ESP8266_FREE_SPACE') ||
    !check_header('HTTP_X_ESP8266_SKETCH_SIZE') ||
    !check_header('HTTP_X_ESP8266_SKETCH_MD5') ||
    !check_header('HTTP_X_ESP8266_CHIP_SIZE') ||
    !check_header('HTTP_X_ESP8266_SDK_VERSION')
) {
    //echo "only for ESP8266 updater! (header)\n";
    logMsg($logFile, "Invalid request. Missing headers.");
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
}

// Validate the requested version
$currVer = $_SERVER['HTTP_X_ESP8266_VERSION'];
if (strlen($currVer) < 5) {
    logMsg($logFile, "Invalid version: $currVer");
    header($_SERVER["SERVER_PROTOCOL"].' 500 no version for ESP MAC', true, 500);
    exit();
}
$hwVer = substr($currVer, -4);
$swVer = substr($currVer, 0, -5);

// Get the latest dimmer file in the bin directory
$latestInfo = json_decode(file_get_contents($filePath.$infoFile), true);

// Check latest info
if(!isset($latestInfo[$hwVer]) || !isset($latestInfo[$hwVer]['version']) || !isset($latestInfo[$hwVer]['file'])) {
    //echo "No version info!\n";
    logMsg($logFile, "Missing info for: $hwVer");
    header($_SERVER["SERVER_PROTOCOL"].' 500 no version for ESP MAC', true, 500);
    exit();
}

$binFile = $filePath.$latestInfo[$hwVer]['file'];

// Check file exists
if(!file_exists($binFile)) {
    //echo "Missing file!\n";
    logMsg($logFile, "Missing file: $binFile $hwVer $swVer = ".$latestInfo[$hwVer]['version']);
    header($_SERVER["SERVER_PROTOCOL"].' 500 no version for ESP MAC', true, 500);
    exit();
}


//if($latestInfo[$hwVer]['version'] != $swVer || $_SERVER["HTTP_X_ESP8266_SKETCH_MD5"] != md5_file($binFile)) {
if($latestInfo[$hwVer]['version'] != $swVer) {
    logMsg($logFile, var_export($_SERVER, true));
    sendFile($binFile);
} else {
    logMsg($logFile, "Up to date: ".$_SERVER["HTTP_X_ESP8266_STA_MAC"]."  HW: $hwVer SW: $swVer");
    header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
}
