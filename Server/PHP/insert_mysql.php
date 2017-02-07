<?php
foreach ($_REQUEST as $key => $value)
{
	if ($key == "node") {
		$_node = $value;
	} else if ($key == "value") {
	    $_value = $value;
	} else if ($key == "position") {
	    $_position = $value;
	} else if ($key == "table") {
	    $_table = $value;
	}
}
// EDIT: Your mysql database account information
$username = "root";
$password = "jmkvktjkm";
$database = "bee_logger";
$localhost = "localhost";
// Check Connection to Database
if (mysql_connect($localhost, $username, $password))
  {
  	@mysql_select_db($database) or die ("Unable to select database");
    if (is_null($_position) || $_position == "0") {
    	$query = "INSERT INTO $_table (id, time_stamp, node, value) VALUES (0, now(),$_node, $_value)";
    }
    else {
        $query = "INSERT INTO $_table (id, time_stamp, node, value, position) VALUES (0, now(),$_node, $_value, $_position)";
    }
  	$result = mysql_query($query);
  	if (!$result) {
        die('MySQL Error: ' . mysql_error());
    } else {
        die('OK');
    }
  } else {
  	echo('Unable to connect to database.');
  }
?>