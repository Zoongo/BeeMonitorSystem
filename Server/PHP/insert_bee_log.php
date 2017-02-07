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
	    $_tablename = $value;
	}
}

$username = "root";
$password = "jmkvktjkm";
$database = "bee_logger";
$localhost = "localhost";

// Check Connection to Database
if (mysql_connect($localhost, $username, $password))
  {
  	@mysql_select_db($database) or die ("Unable to select database");
  	if $_position is not null {
       $query = "INSERT INTO $_tablename (id, time_stamp, node, value, position) VALUES (0, now(),$_node, $_value, $_position)"; 	
  	} else {
       $query = "INSERT INTO $_tablename (id, time_stamp, node, value) VALUES (0, now(),$_node, $_value)";   	
  	}
  	$result = mysql_query($query);
  	if (!$result) {
        die('MySQL Error: ' . mysql_error());
    } else {
  	  die ('Unable to connect to database.');
     }
}
?>