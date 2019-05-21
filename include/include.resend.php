<?php
include_once(dirname(__FILE__).'/include.const.php');

function resend_sms($msid, $msg){
	$resend_res = -1;
	$resend_response = '';
    if (defined('BINU_SMS_ENGINE_WEB_PATH')){        
        $sms_class_file = BINU_SMS_ENGINE_WEB_PATH.'/class/class_sendsms.php';
        if (file_exists($sms_class_file)){
            include_once($sms_class_file);
            
            
            
            $sms_args['from']           = $msg['ms_from'];
    		$sms_args['from_account_id']= $msg['ms_from_account_id'];
    		$sms_args['to']             = $msg['ms_to'];    
    		$sms_args['to_account_id']  = $msg['ms_to_account_id'];
    
    		$sms_args['provider']       = '';
    		$sms_args['device_ip']      = $msg['ms_device_ip'];
    		$sms_args['device_id']      = $msg['ms_device_id'];
    
    		$sms_args['app_id']         = $msg['ms_app_id'];    
    		$sms_args['post_script']    = $msg['ms_pscript'];
    		$sms_args['binu_level']     = '';
    		$sms_args['dup_key']        = '';
    		$sms_args['message']        = $msg['ms_message'];
    		$sms_args['credit_id']      = '';
    
    		$sms_args['giveup']         = ''; 
    		$sms_args['msg_type']       = $msg['ms_msg_type'];
    		$sms_args['from_name']      = $msg['ms_from_name'];
    
    		$sms_args['msgid']          = $msg['ms_binu_msgid'];
    		$sms_args['sent_providers'] = $msg['ms_sent_provider'];
                        
            $sms_obj = new class_sendsms($sms_args);
            //very important, mark as this sms as resend
            //$sms_obj->reSendInitParams($msg['ms_binu_msgid'], $msg['ms_sent_provider']);
            $sms_obj->send_sms();
            $resend_res = $sms_obj->mResult;
            $resend_response = $sms_obj->mErrorMsg;
        }
        else{
			$resend_res = -1;
            $resend_response = 'Can not find class file '.$sms_class_file;
        }
    }
    else{
		$resend_res = -1;
		$resend_response = 'Can not find paramater SMS engine path.';
    }
    
    resend_sms_log($msid, $resend_res, $resend_response, $msg);    
    resend_UpdateResendTimestamp($msg['ms_binu_msgid']);
    return array($resend_res, $resend_response);
}

function resend_sms_log($msid, $resend_res, $resend_reason, $result_sms){
    if (empty($result_sms)){
        return;
    }    
    
    $message = preg_replace('/\s+/', ' ', $result_sms['ms_message']);
    $message = str_replace("\r", "", $message);
    $message = str_replace("\n", "", $message);
    $log_msg = '['.date("Y-m-d H:i:s")."] MsgID[".$result_sms['ms_binu_msgid']."] MsgType[".$result_sms['ms_msg_type']."] Msid[".$msid."] Provider[".$result_sms['ms_provider']."] ProviderType[".$result_sms['ms_provider_type']."] DeviceIP[".$result_sms['ms_device_ip']."] DeviceID[".$result_sms['ms_device_id']."] AppID[".$result_sms['ms_app_id']."] PostScript[".$result_sms['ms_pscript']."] SentProvider[".$result_sms['ms_sent_provider']."] ResendReturn[".$resend_res."] ResendReturnResponse[".$resend_reason."] Message[".$message."] ResponseMsg[".$result_sms['ms_response_msg']."]\n";
    $log_file = BINU_SMS_ENGINE_PATH .'/log/resendRequest_' . date('Ymd').'.log';
    resend_my_log($log_file, $log_msg);    
}
function resend_my_log($logfile, $logmsg){
    $handle = fopen($logfile, "a");
    if ($handle){
        if (flock($handle, LOCK_EX)) { // do an exclusive lock
            fwrite($handle, $logmsg);
            flock($handle, LOCK_UN); // release the lock
        }
        fclose($handle);
    }
}

function resend_UpdateResendTimestamp($msgid){
	//After we call function resend_sms(), script sms_resend_and_commit_credit.php will go through all resend list again, we need update the timestamp to now. Otherwise will duplicate to resend again. Because script sms_provider.php need a few seconds to handle the resend request, then update the timestamp. During this period the duplicate resend could be happen if we didn't update the timestamp as soon as possible. It is very important!
	//When we finished to send SMS and waitting for the status report return up to 2 hours.
	//If status report return with success. Then this transaction finishe for ever.
	//If status report return with failed within 2 hours after sent, then we will try next provider, until success.
	
	//So we need update the filed bm_time in table bmessager_xxxxxx as soon as possible. Because script sms_scan_and_commit.php
	//is scaning table bmessager_xxxxxx always and check the filed bm_time if filed bm_status is 0, also script
	//sms_provider.php need some time to start sending SMS after you sent the request.
	//Otherwise we could be resend this SMS multiple time if we not update the filed bm_time as soon as possible.
	
	
	list($lastMonth, $thisMonth, $nextMonth) = resendCommit_GetMonthsByMsgid($msgid);
	
	if (!empty($thisMonth)){
		$table = 'bmessager_'.$thisMonth;
		if (resendCommit_CheckTableExists($table) == 1){
			$query = "update $table set bm_time = now() where bm_msgid = '$msgid'";
			//echo $query."\n";
			$result = mysql_query($query);
			$num_rows = mysql_affected_rows();
	    	if ($num_rows >= 1){
	    		return;
	    	}	
		}		
	}
	
	if (!empty($lastMonth)){
		$table = 'bmessager_'.$lastMonth;
		if (resendCommit_CheckTableExists($table) == 1){
			$query = "update $table set bm_time = now() where bm_msgid = '$msgid'";
			//echo $query."\n";
			$result = mysql_query($query);
			$num_rows = mysql_affected_rows();
	    	if ($num_rows >= 1){
	    		return;
	    	}	
		}		
	}
	
	if (!empty($nextMonth)){
		$table = 'bmessager_'.$nextMonth;
		if (resendCommit_CheckTableExists($table) == 1){
			$query = "update $table set bm_time = now() where bm_msgid = '$msgid'";
			//echo $query."\n";
			$result = mysql_query($query);
			$num_rows = mysql_affected_rows();
	    	if ($num_rows >= 1){
	    		return;
	    	}	
		}		
	}	
}

function resendCommit_CheckTableExists($table){
    $query = "SHOW TABLES LIKE '$table'";
    //print "$query \n";
    $result = mysql_query($query);
    $num_rows = mysql_num_rows($result);
    if ($num_rows >= 1){
        return 1;
    }
    return 0;
}

function resendCommit_GetMonthsByMsgid($msgid = ''){
	$thisMonth = '';
	$lastMonth = '';
	$nextMonth = '';
	
	if (!empty($msgid)){
		if (substr($msgid, 0, 1) == 'e' || substr($msgid, 0, 1) == 'm'){
            $sec = substr($msgid, 1, 10);
        }
        else{
            $sec = substr($msgid, 0, 10);
        }
        
        $thisMonth = date('Ym', strtotime('+0 month', $sec));
		$lastMonth = date('Ym', strtotime('-1 month', $sec));
		$nextMonth = date('Ym', strtotime('+1 month', $sec));
	}
	
	return array($lastMonth, $thisMonth, $nextMonth);
}


/*






function resendCommit_CheckSentProvider($sent_providers){
	global $GBL_PROVIDER_LIST;
	
	//Still has any provider left need to send?
    $used_provider = (!empty($sent_providers))?  explode(",", $sent_providers) : array();
    $flag = 0;
    for($i = 0; $i < count($GBL_PROVIDER_LIST); $i++){
		$p_array = $GBL_PROVIDER_LIST[$i];
		list($key, $val) = each($p_array);
		if($val['TYPE'] == 'GLOBAL'){
			if (!in_array($val['ID'], $used_provider)){
				$flag = 1;
				break;
			}
		}
	}
	return $flag;
}

function resendCommit_SearchSMSWithBmTable($msgid, $bm_table){
	$search_result = array();
	$query = "select * from $bm_table where bm_msgid = '$msgid'";
	$result = mysql_query($query);
	if ($result){
		$num = mysql_num_rows($result);
		if ($num > 0){		
			$search_result = mysql_fetch_assoc($result);
			mysql_free_result($result);
		}
	}
	
	return $search_result;
}
		
function resendCommit_SearchSMSWithTable($msgid, $sms_table){
	$search_result = array();
	$query = "select * from $sms_table where ms_binu_msgid = '$msgid'";
	$result = mysql_query($query);
	if ($result){
		$num = mysql_num_rows($result);
		if ($num > 0){		
			$search_result = mysql_fetch_assoc($result);
			mysql_free_result($result);
		}
	}
	
	return $search_result;	
}

function resendCommit_CheckResend($step_minutes, $msg){
	global $GBL_PROVIDER_LIST;
	global $RESEND_MAX_MINUTES;
	
	//Don't need resend if this is hard code provider.
	if ($msg['ms_provider_type'] == 2){
        //print "hard code provider , don't need resend \n";
        return 0;
    }
    
    //Still has any provider left need to send?
    $used_provider = (!empty($msg['ms_sent_provider']))?  explode(",", $msg['ms_sent_provider']) : array();
    $flag = 0;
    for($i = 0; $i < count($GBL_PROVIDER_LIST); $i++){
		$p_array = $GBL_PROVIDER_LIST[$i];
		list($key, $val) = each($p_array);
		if($val['TYPE'] == 'GLOBAL'){
			if (!in_array($val['ID'], $used_provider)){
				$flag = 1;
				break;
			}
		}
	}
	
	if ($flag == 1){
		//print "Still has provider is available to be used.\n";
		//$past_minutes = (strtotime("now") - strtotime($msg['ms_time'])) / 60; //minutes
		$past_minutes = $step_minutes;
		if (isset($RESEND_MAX_MINUTES) && ($RESEND_MAX_MINUTES < $past_minutes)){
			//print "over $RESEND_MAX_MINUTES, need resend now\n";
			return 1;			
		}
		else{
			//print "within $RESEND_MAX_MINUTES , still need to waite for the status report. Until over $RESEND_MAX_MINUTES \n";
			return 2;
		}
	}
	else{
		//print "We tried all provider already. Finished.\n";
		return 0;
	}
}


function resendCommit_SearchSMS(&$table_array, $bm_msgid){
	$today = date("Y-m-d");
	$search_result = array();
	$table = '';
	list($lastMonth, $thisMonth, $nextMonth) = resendCommit_GetMonthsByMsgid($bm_msgid);
	
	if (!empty($thisMonth)){
		$table = 'message_'.$thisMonth;
		$table_array[$today] = resendCommit_CheckRecordTable($table_array[$today], $table);
		$search_result = resendCommit_SearchTable($bm_msgid, $table);
	}
	
	if (empty($search_result) && !empty($nextMonth)){
		$table = 'message_'.$nextMonth;
		$table_array[$today] = resendCommit_CheckRecordTable($table_array[$today], $table);
		$search_result = resendCommit_SearchTable($bm_msgid, $table);
	}
	
	if (empty($search_result) && !empty($lastMonth)){
		$table = 'message_'.$lastMonth;
		$table_array[$today] = resendCommit_CheckRecordTable($table_array[$today], $table);
		$search_result = resendCommit_SearchTable($bm_msgid, $table);
	}
	return array($table, $search_result);
}

function resendCommit_CheckRecordTable($table_array, $check_table){
    if (!isset($table_array[$check_table])){//check table 
        if (resendCommit_CheckTableExists($check_table)){
            $table_array[$check_table] = 1;//table exists
        }
        else{
            //print "Transaction table $check_table doesn't exists!\n";
            $table_array[$check_table] = 2;//table doesn't exist
        }        
    }
    return $table_array;       
}

function resendCommit_SearchTable($msgid, $table){
	$row = array();
	$query = "select * from $table where ms_binu_msgid = '$msgid'"; 
	$result = mysql_query($query);
	$num_rows = mysql_num_rows($result);
	if ($num_rows >= 1){
		$row = mysql_fetch_assoc($result);		
	}
	return $row;
}



function resendCommit_GetLastProcessRecord($conf){
    list($result, $table, $id) = resendCommit_ReardConfigFile($conf);
    
    //print "readconfig $result, $table, $id \n";
    if ($result != 0){
        $table = 'bmessager_'.date("Ym");
        
        $id = 0;
        if (resendCommit_CheckTableExists($table)){            
            resendCommit_WriteConfigFile($conf, $table, $id);
        }
        else{
            return array('', '');
        }                
    }
    return array($table, $id);
}

function resendCommit_WriteConfigFile($conf, $table, $id){
    $buf = $table.':'.$id;
    //print "Write new config data $buf \n";
    if (!$handle = fopen($conf, 'w')) {
        return -1;
    }
    if (flock($handle, LOCK_EX)) { // do an exclusive lock
        if (fwrite($handle, $buf) === FALSE) {
            flock($handle, LOCK_UN); // release the lock
            return -1;
        }    
        flock($handle, LOCK_UN); // release the lock
    }    
    fclose($handle);
    return 0;   
}

function resendCommit_ReardConfigFile($conf){
    $table ='';
    $id = '';
    if (file_exists($conf)){
        $handle = @fopen($conf, "r");
        if ($handle) {
            while (!feof($handle)) {
                $buffer = fgets($handle, 4096);
                $buffer = str_replace(array("\n", "\r"), "", $buffer); 
                list($table, $id) = explode(":", $buffer);
                return array(0, $table, $id);
            }
            fclose($handle);
        }
        else{
            return array(-1, $table, $id);
        }
    }
    return array(-1, $table, $id);
}



function resendCommit_MyLog($log_file, $message){
    $handle = fopen($log_file, "a");
    if ($handle){
        if (flock($handle, LOCK_EX)) { // do an exclusive lock
            fwrite($handle, $message);
            flock($handle, LOCK_UN); // release the lock
        }
    fclose($handle);
    }
}
*/
?>
