<?php
include_once(dirname(__FILE__).'/include.const.php');

/*
 * Database connect
 */
 
function dbi_connect() {
	while(1){
		$mysqli = new mysqli(MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DB);
		
		if ($mysqli->connect_error) {
		    error_log('['.date("Y-m-d H:i:s")."] Could not connect MySQL: " .$mysqli->connect_error);
		    sleep(1);
		}
		else{
			if (function_exists('mysql_set_charset')) { // PHP 5 >= 5.2.3
                $mysqli->set_charset('utf8');    
            }
            
			if (($mysqli->select_db(MYSQL_DB)) == FALSE){
				$mysqli->close();
				error_log('['.date("Y-m-d H:i:s")."] Could not select db: ". $mysqli->connect_error);
			}
			break;
		}
	}
	return $mysqli;
}

 
?>