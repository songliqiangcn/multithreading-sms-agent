<?php
include_once(dirname(__FILE__).'/include.const.php');

/*
 * Database connect
 */
function db_connect() {
    $link = 0;
    
    while(1){
        $link = @mysql_connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD);        
        if (!$link) {
            //echo "Could not connect MySQL: ". mysql_error()."\n";
            error_log('['.date("Y-m-d H:i:s")."] Could not connect MySQL: ". mysql_error());
            sleep(1);
        }
        else{
            if (function_exists('mysql_set_charset')) { // PHP 5 >= 5.2.3
                mysql_set_charset('utf8');    
            }
            
            if (!(@mysql_select_db(MYSQL_DB))) {
                //echo "Could not select db: ". mysql_error()."\n";
                error_log('['.date("Y-m-d H:i:s")."] Could not select db: ". mysql_error());
                db_close();
            }
            break;
        }
    }
    return $link;
}

function db_close($link = null){
    if (is_null($link))
	@mysql_close();
    else
	@mysql_close($link);
}

?>
