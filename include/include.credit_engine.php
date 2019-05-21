<?php
//include_once(dirname(__FILE__).'/../class/api.binu_credits.php');
include_once(dirname(__FILE__).'/include.const.php');
include_once('biNu/api.binu_credits.php');

//send status to credit engine with transaction id.
function send_status_to_credit_engine($msgid = '', $from_accountid = '', $credit_transactionid = 0, $status = 2, $app_id = ''){
    if (empty($credit_transactionid) || ($credit_transactionid == 0)){
        return;
    }
    
    //$status: 1:report marked as success 2:report marked as failed,  3: no report reutrn after 24 hours.
    
    
    $binu_credits = new binu_credits();
    /* Initialise class variables */
    $binu_credits->developer_id = 1;
    $binu_credits->application_id = $app_id;    
    $binu_credits->api_key = 'biNu-$mS';

    /* Commit Credits */
    $binu_account_id = $from_accountid; // biNu Users Profile ID
    $credit_transaction_id    = $credit_transactionid; // Action transaction ID
    //$credit_status = ($status == 1)? 1 : 0; // Status of transaction (0 = fail, 1 = success) 
    $credit_status = ($status == 2)? 0 : 1; //If we can not get delivery report after 24 hours, we will still charge user.
    
    
    //testing
    $json_response = '';
    for ($try = 0; $try < 3; $try++){
	try{
    		$json_response = $binu_credits->commit_credits( $binu_account_id, $credit_transaction_id, $credit_status );
    		if ($json_response){
    			break;
    		}		 	
    	}
	catch (Exception $e) {
    		error_log("Error to commit credit with error ".$e->getMessage());
    	}
     }

    
    $json_response = objectToArray(json_decode($json_response));
    
    //print_r($json_response);
    $res = (isset($json_response['commit']))? $json_response['commit'] : array();
    credit_commit_log($res, $msgid, $from_accountid, $credit_transactionid, $status, $credit_status,  $app_id);
    return $res;
}

function credit_commit_log($commit_result, $msgid, $from_accountid, $credit_ID, $status, $credit_status, $appid){    
    $commit_result['message'] = isset($commit_result['message'])? $commit_result['message'] : '';
    
    //$log_title = ($status == 1 || $status == 2)? "REPORT_COMMIT" : "NO_REPORT_COMMIT";
    
    $log_message = '['.date("Y-m-d H:i:s")."] MsgID[".$msgid."] CreditTransID[".$credit_ID."] FromAccountID[".$from_accountid."] TransactionStatus[".$status."] CreditStatus[".$credit_status."] AppID[".$appid."] CommitMessage[".$commit_result['message']."]\n";
    
    $log_file = BINU_SMS_ENGINE_PATH . '/log/credit_commit_' . date('Ymd').'.log';                        
    my_commit_log($log_file, $log_message); 
}

function my_commit_log($logfile, $logmsg){
    $handle = fopen($logfile, "a");
    if ($handle){
        if (flock($handle, LOCK_EX)) { // do an exclusive lock
            fwrite($handle, $logmsg);
            flock($handle, LOCK_UN); // release the lock
        }
        fclose($handle);
    }
}

function objectToArray($d) {
    if (is_object($d)) {
        // Gets the properties of the given object
        // with get_object_vars function
        $d = get_object_vars($d);
    }
 
    if (is_array($d)) {
        /*
        * Return array converted to object
        * Using __FUNCTION__ (Magic constant)
        * for recursive call
        */
        return array_map(__FUNCTION__, $d);
    }
    else {
        // Return array
        return $d;
    }
 }
      
?>
