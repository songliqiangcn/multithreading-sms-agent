<?php

if (!defined('_ENV_TYPE')){
	$_ENV_TYPE = 'live';
	
	$_HOME_PATH = '';
	$check_name = array('johnson', 'joe');//put your username on D01 into this array. Username exists in this array. Current dev environment is local. Othewise is live.
	if (preg_match("/\/home\/(\w*)/i", dirname(__FILE__), $matches)){
		if (isset($matches[1]) && !empty($matches[1]) && in_array($matches[1], $check_name)){
			$_ENV_TYPE = 'local';			
		}
		$_HOME_PATH = $matches[0];
	}
	if (empty($_HOME_PATH)){
		error_log("Failed to get home path.");
		exit(0);
	}
	
	define('_ENV_TYPE', $_ENV_TYPE);
	define('_HOME_PATH', $_HOME_PATH);
		
	
	/*
	$_CHECK_HOME_PATH = '/home/johnson';
	$_HOME_PATH       = $_CHECK_HOME_PATH;
	if ((PHP_SAPI == 'cli') && isset($_SERVER['HOME'])){
		$_HOME_PATH = $_SERVER['HOME'];
		if ($_SERVER['HOME'] == $_CHECK_HOME_PATH){
			$_ENV_TYPE  = 'local';
		}
	}

	if ((PHP_SAPI != 'cli') && isset($_SERVER['DOCUMENT_ROOT'])){
		if (substr($_SERVER['DOCUMENT_ROOT'], 0, strlen($_CHECK_HOME_PATH)) == $_CHECK_HOME_PATH){
			$_ENV_TYPE = 'local';
		}
		if (preg_match("/^\/home\/(\w+)/i", $_SERVER['DOCUMENT_ROOT'], $match)){
			$_HOME_PATH = isset($match[0])? $match[0] : $_CHECK_HOME_PATH;
		}
	}

	define('_ENV_TYPE', $_ENV_TYPE);
	define('_HOME_PATH', $_HOME_PATH);

	unset($_CHECK_HOME_PATH);
	unset($_ENV_TYPE);
	unset($_HOME_PATH);
	*/
}


if (!defined('BINU_SMS_ENGINE_PATH')){
	define('BINU_SMS_ENGINE_PATH', _HOME_PATH.'/send_sms_v2');
	/*
    if (_ENV_TYPE == 'live'){//live
        define('BINU_SMS_ENGINE_PATH', '/home/sms/send_sms_v2');    
    }
    else{//local
        define('BINU_SMS_ENGINE_PATH', '/home/johnson/send_sms_v2');
    }
    */    
}

if (!defined('BINU_SMS_ENGINE_WEB_PATH')){
	define('BINU_SMS_ENGINE_WEB_PATH', _HOME_PATH.'/public_html/send_sms');
	/*
    if (_ENV_TYPE == 'live'){//live
        define('BINU_SMS_ENGINE_WEB_PATH', '/home/sms/public_html/send_sms');
    }
    else{//local
        define('BINU_SMS_ENGINE_WEB_PATH', '/home/johnson/public_html/engines/send_sms');
    }
    */    
}

if (!defined('BINU_MESSAGER_PATH')){
	define('BINU_MESSAGER_PATH', _HOME_PATH.'/public_html/binu_messager');
	
	/*
    if (_ENV_TYPE == 'live'){//live
        define('BINU_MESSAGER_PATH', '/home/sms/public_html/binu_messager');
    }
    else{//local
        define('BINU_MESSAGER_PATH', '/home/johnson/public_html/binu_messager');
    }
    */
    
}

if (_ENV_TYPE == 'live'){//live
	date_default_timezone_set('UTC');
}



// config for notifications
define('PHP_AUTOLOADER_CONFIG_FILE', 'resources/AutoloadConfig.php');
define('BINU_LOG_CONFIG_FILE', 'resources/log4php.xml');

$APP_NOTIFICATION_SETTING = array('DisplayText' => 'Messenger', 'Icon' => 'http://static.binu.com/images/app_icons/messenger/messanger_logo64.png', 'AppUrl' => 'http://apps.binu.net/apps/binutxt/index.php?a=ib');

//---------------------------
//SMS engine setting
if (!defined('SMSAGENT_HOST')){
    define('SMSAGENT_HOST', '127.0.0.1');
}

if (!defined('SMSAGENT_PORT')){
    define('SMSAGENT_PORT', '7001');
}

//biNu Account setting
if (!defined('BINU_ACCOUNT_APP_ID')){
	define('BINU_ACCOUNT_APP_ID', 3342);
}

//get SMS provider info
define('SMS_ENGINE_CONF_PATH', BINU_SMS_ENGINE_PATH.'/conf/sms_engine.xml');

$GBL_PROVIDER_LIST             = array();
$GBL_SMS_ENGINE_RULES          = array();
$GBL_MESSAGE_TYPE_SETTING      = array();
$GBL_COUNTRY_PROVIDERS_SETTING = array();
$GBL_DEFAULT_SETTING           = array();
$SMS_ENGINE_ALLOWED_IPS        = array();
$GBL_PREFIX_MSG_COUNTRY        = array();

/*
$GBL_COUNTRY_PROVIDERS      = array(); //use different country for individual country.
$SMS_ENGINE_ALLOWED_IPS     = array(); //allowd IP address
$GBL_PROVIDER_LIST          = array(); //default provider select list(order)
$GBL_MESSAGE_TYPE_SET       = array(); //we can define provider list by messagetype
$GBL_PREFIX_MSG_COUNTRY     = array(); //here is the country list we need frefix msg if sending SMS for people to people
*/
eg_get_config(SMS_ENGINE_CONF_PATH);

//testing
//$res = getNextProvider(array(1), '', '', 'PH' );

//print_r($res);

/*
print_r($GBL_PROVIDER_LIST);
print_r($GBL_SMS_ENGINE_RULES) ;
print_r($GBL_COUNTRY_PROVIDERS_SETTING);
print_r($GBL_MESSAGE_TYPE_SETTING);
print_r($GBL_DEFAULT_SETTING);
print_r($SMS_ENGINE_ALLOWED_IPS);
print_r($GBL_PREFIX_MSG_COUNTRY);
*/
//if (!defined('RESEND_MAX_MINUTES')){
//    define('RESEND_MAX_MINUTES', 1);
//    $RESEND_MAX_MINUTES = 60; //we give every provider max 1 hour to delivery SMS. Otherwise will try next one.
//}

if (!defined('STATUS_MAX_MINUTES')){
	define('STATUS_MAX_MINUTES', 1);
	$STATUS_MAX_MINUTES = 24 * 60;//max 24 hours, we will give up if still no status return;
}

//---------------------------
//PHP Curl setting
if (!defined('SMS_API_TIMEOUT')){
    define('SMS_API_TIMEOUT', 5);
}

if (!defined('SMS_API_USERAGENT')){
    define('SMS_API_USERAGENT', 'biNu agent (http://www.binu.com)');
}


//---------------------------
//MySQL Database
if (!defined('MYSQL_HOST')){
    if (_ENV_TYPE == 'live'){//live
        define('MYSQL_HOST', 'appsdb.c0iel6uw4qef.us-west-1.rds.amazonaws.com');
    }
    else{
        define('MYSQL_HOST', 'localhost');
    }    
}

if (!defined('MYSQL_USER')){
    if (_ENV_TYPE == 'live'){//live
        define('MYSQL_USER', 'apps');
    }
    else{
        define('MYSQL_USER', 'apps');
    }
}

if (!defined('MYSQL_PASSWORD')){
    if (_ENV_TYPE == 'live'){//live
        define('MYSQL_PASSWORD', 'H5ubt145DB01');
    }
    else{
        define('MYSQL_PASSWORD', 'devpass');
    }    
}

if (!defined('MYSQL_DB')){
    define('MYSQL_DB', 'binu_sms');
}


function eg_object2array($data) {
    if (is_object($data)) $data = get_object_vars($data);
    return is_array($data) ? array_map(__FUNCTION__, $data) : $data;
}

function eg_get_config($conf){
	global $GBL_PROVIDER_LIST;
	global $GBL_SMS_ENGINE_RULES;
	global $GBL_COUNTRY_PROVIDERS_SETTING;
	global $GBL_MESSAGE_TYPE_SETTING;
	global $GBL_DEFAULT_SETTING;
	global $SMS_ENGINE_ALLOWED_IPS;
	global $GBL_PREFIX_MSG_COUNTRY;
	
	if (file_exists($conf)){
		$xml        = simplexml_load_file($conf);
		$conf_array = eg_object2array($xml);
		$provider_hash = array();
		
		//print_r($conf_array);
		
		//provider list
		if (isset($conf_array['provider_list']['provider']) && is_array($conf_array['provider_list']['provider'])){
			for($i = 0; $i < count($conf_array['provider_list']['provider']); $i++){
				if (strtolower($conf_array['provider_list']['provider'][$i]['status']) == 'active'){
					$GBL_PROVIDER_LIST[] = array($conf_array['provider_list']['provider'][$i]['name'] => $conf_array['provider_list']['provider'][$i]);
					$provider_hash[$conf_array['provider_list']['provider'][$i]['name']] = 1;
				}
			}
		}
		//print_r($provider_hash);
		
		//global rules 
		foreach ($conf_array['SMS_ENGINE_RULES'] as $rule => $v){
			if (strtolower($rule) == 'description'){
				continue;
			}
			$GBL_SMS_ENGINE_RULES[] = $rule;
		}
		//print_r($GBL_SMS_ENGINE_RULES);
		
		//country provider conf
		if (isset($conf_array['SMS_ENGINE_RULES']['country_providers_setting']['country']) && is_array($conf_array['SMS_ENGINE_RULES']['country_providers_setting']['country'])){
			$country_list = $conf_array['SMS_ENGINE_RULES']['country_providers_setting']['country'];
			
			if (isset($country_list['country_name']) && is_string($country_list['country_name'])){//only one country setting
				$cname    = $country_list['country_name'];
				$provider = $country_list['provider'];
				$status   = strtolower($country_list['status']);
				$give_up  = strtolower($country_list['@attributes']['give_up']);
				
				if ($status == 'active'){
					$country_tmp = array();
					if (is_string($provider)){
						if (isset($provider_hash[$provider])){
							$country_tmp[0] = $provider;
						}
					}
					else{
						for($i = 0; $i < count($provider); $i++){
							$pr = $provider[$i];
							if (isset($provider_hash[$pr])){
								$country_tmp[] = $pr;
							}
						}
					}
					
					if (!empty($country_tmp)){						
						$GBL_COUNTRY_PROVIDERS_SETTING[$cname]['provider'] = $country_tmp;
						$GBL_COUNTRY_PROVIDERS_SETTING[$cname]['give_up']  = $give_up;
					}
					unset($country_tmp);
				}
			}			
			else{
				for($j = 0; $j < count($country_list); $j++){
					$set = $country_list[$j];
						
					$cname    = $set['country_name'];
					$provider = $set['provider'];
					$status   = strtolower($set['status']);
					$give_up  = strtolower($set['@attributes']['give_up']);
					
					if ($status == 'active'){
						$country_tmp = array();
						if (is_string($provider)){
							if (isset($provider_hash[$provider])){
								$country_tmp[0] = $provider;
							}
						}
						else{
							for($i = 0; $i < count($provider); $i++){
								$pr = $provider[$i];
								if (isset($provider_hash[$pr])){
									$country_tmp[] = $pr;
								}
							}
						}
						
						if (!empty($country_tmp)){
							$GBL_COUNTRY_PROVIDERS_SETTING[$cname]['provider'] = $country_tmp;
							$GBL_COUNTRY_PROVIDERS_SETTING[$cname]['give_up']  = $give_up;
						}
						unset($country_tmp);
					}
				}
			}			
		}
		
		//print_r($GBL_COUNTRY_PROVIDERS_SETTING);
		//print_r($conf_array);exit(0);
		//get provider list by message type
		if (isset($conf_array['SMS_ENGINE_RULES']['message_type_setting']) && is_array($conf_array['SMS_ENGINE_RULES']['message_type_setting'])){
			$message_type_setting = $conf_array['SMS_ENGINE_RULES']['message_type_setting'];
			
			foreach ($message_type_setting as $type => $set){
				$type     = strtolower($type);
				$give_up  = strtolower($set['@attributes']['give_up']);
				$provider = $set['provider'];
				$status   = strtolower($set['status']);
				$country_t= isset($set['country'])?  $set['country'] : array();
				
				if ($status == 'active'){
					$msg_type_def_provider     = array();					
					$msg_type_country_provider = array();
					
					$msg_type_def_provider_tmp = array();
					//default provider list for this message type
					if (is_string($provider)){
						if (isset($provider_hash[$provider])){
							$msg_type_def_provider_tmp[0] = $provider;
						}
					}
					else{
						for($i = 0; $i < count($provider); $i++){
							$pr = $provider[$i];
							if (isset($provider_hash[$pr])){
								$msg_type_def_provider_tmp[] = $pr;
							}
						}
					}
					
					if (!empty($msg_type_def_provider_tmp)){
						$msg_type_def_provider['provider'] = $msg_type_def_provider_tmp;
						$msg_type_def_provider['give_up']  = $give_up;
					}
					unset($msg_type_def_provider_tmp);
					
					
					//specify the country providers for this message type					
					if (isset($country_t['country_name']) && is_string($country_t['country_name'])){//only one country setting
						$cname    = $country_t['country_name'];
						$provider = $country_t['provider'];
						$status   = strtolower($country_t['status']);
						$give_up  = strtolower($country_t['@attributes']['give_up']);
						
						$msg_type_country_provider_tmp = array();
						
						if ($status == 'active'){							
							if (is_string($provider)){
								if (isset($provider_hash[$provider])){
									$msg_type_country_provider_tmp[0] = $provider;
								}
							}
							else{
								for($i = 0; $i < count($provider); $i++){
									$pr = $provider[$i];
									if (isset($provider_hash[$pr])){
										$msg_type_country_provider_tmp[] = $pr;
									}
								}
							}
								
							if (!empty($msg_type_country_provider_tmp)){
								$msg_type_country_provider[$cname]['provider'] = $msg_type_country_provider_tmp;
								$msg_type_country_provider[$cname]['give_up']  = $give_up;
							}
							unset($msg_type_country_provider_tmp);
						}
					}
					else{
						for($j = 0; $j < count($country_t); $j++){
							$set = $country_t[$j];
						
							$cname    = $set['country_name'];
							$provider = $set['provider'];
							$status   = strtolower($set['status']);
							$give_up  = strtolower($set['@attributes']['give_up']);
								
							if ($status == 'active'){
								$msg_type_country_provider_tmp = array();
								if (is_string($provider)){
									if (isset($provider_hash[$provider])){
										$msg_type_country_provider_tmp[0] = $provider;
									}
								}
								else{
									for($i = 0; $i < count($provider); $i++){
										$pr = $provider[$i];
										if (isset($provider_hash[$pr])){
											$msg_type_country_provider_tmp[] = $pr;
										}
									}
								}
						
								if (!empty($msg_type_country_provider_tmp)){
									$msg_type_country_provider[$cname]['provider'] = $msg_type_country_provider_tmp;
									$msg_type_country_provider[$cname]['give_up']  = $give_up;
								}
								unset($msg_type_country_provider_tmp);
							}
						}
					}
					
					
					if (!empty($msg_type_def_provider)){
						$GBL_MESSAGE_TYPE_SETTING[$type] = $msg_type_def_provider;
					}
					unset($msg_type_def_provider);
					
					if (!empty($msg_type_country_provider)){
						$GBL_MESSAGE_TYPE_SETTING[$type]['country'] = $msg_type_country_provider;
					}
					unset($msg_type_country_provider);					
				}
			}			
		}
		
		//print_r($GBL_MESSAGE_TYPE_SETTING);
		
		//default rules setting
		if (isset($conf_array['SMS_ENGINE_RULES']['default_rules_setting']) && is_array($conf_array['SMS_ENGINE_RULES']['default_rules_setting'])){
			$default_setting = $conf_array['SMS_ENGINE_RULES']['default_rules_setting'];
			
			if (isset($default_setting['provider'])){
				$provider = $default_setting['provider'];
				
				$def_tmp = array();
				if (is_string($provider)){
					if (isset($provider_hash[$provider])){
						$def_tmp[0] = $provider;
					}
				}
				else{
					for($i = 0; $i < count($provider); $i++){
						$pr = $provider[$i];
						if (isset($provider_hash[$pr])){
							$def_tmp[] = $pr;
						}
					}
				}
				
				if (!empty($def_tmp)){
					$GBL_DEFAULT_SETTING['provider'] = $def_tmp;
				}
				unset($def_tmp);
			}
			
		}
		//print_r($GBL_DEFAULT_SETTING);
		
		//allowed_ips
		if (isset($conf_array['allowed_ips']['ip_address']) && is_array($conf_array['allowed_ips']['ip_address'])){
			$SMS_ENGINE_ALLOWED_IPS = $conf_array['allowed_ips']['ip_address'];
		}
		
		//prefix message country list
		if (isset($conf_array['prefix_msg_setting'])){
			foreach ($conf_array['prefix_msg_setting'] as $type => $set){
				$type = strtolower($type);
		
				//by provider
				if (isset($set['provider'])){
					if (is_string($set['provider'])){
						$GBL_PREFIX_MSG_COUNTRY[$type]['provider'][0] = $set['provider'];
					}
					else{
						$GBL_PREFIX_MSG_COUNTRY[$type]['provider'] = $set['provider'];
					}
				}
		
				//by country
				if (isset($set['country'])){
					$country_list = $set['country'];
					if (isset($country_list['country_name'])){//only one country
						$cname    = $country_list['country_name'];
						$status   = $country_list['status'];
						if (strtolower($status) == 'active'){
							$GBL_PREFIX_MSG_COUNTRY[$type]['country'][0] = $cname;
						}
					}
					else{
						foreach ($country_list as $country => $set){
							$cname    = $set['country_name'];
							$status   = $set['status'];
							if (strtolower($status) == 'active'){
								$GBL_PREFIX_MSG_COUNTRY[$type]['country'][] = $cname;
							}
						}
					}
				}
			}
		}
		//print_r($GBL_PREFIX_MSG_COUNTRY);
		
	}
	else{
		die("Error, can not found sms engine config file!");
	}	
}

function eg_get_prefix_message($from_name = '', $from = ''){	
	return (((!empty($from_name))? $from_name.' ' : '') . ((!empty($from))? '+'.$from.': ' : ''));
}


function getNextProvider($sent_provider = array(), $select_provider = '', $message_type = '', $dest_country = ''){
	global $GBL_PROVIDER_LIST;
	global $GBL_SMS_ENGINE_RULES;
	global $GBL_MESSAGE_TYPE_SETTING;
	global $GBL_COUNTRY_PROVIDERS_SETTING;
	global $GBL_DEFAULT_SETTING;
	
	$res = array('provider' => '', 'by_rules' => '', 'give_up' => 'no');
	
	//get all available provider in the list
	$new_provider_list = array();
	$provider_hash     = array();
	for($i = 0; $i < count($GBL_PROVIDER_LIST); $i++){
		$p_array         = $GBL_PROVIDER_LIST[$i];
		list($key, $val) = each($p_array);
		$provider_id     = $val['id'];
		$provider_name   = $val['name'];
		
		if (!in_array($provider_id, $sent_provider)){
			$new_provider_list[]           = $provider_name;
			$provider_hash[$provider_name] = $val;
		}
	}
	//print_r($new_provider_list);
	if (empty($new_provider_list)){
		$res['by_rules'] = 'Tried all providers then give up.';
		$res['give_up']  = 'yes';
		return $res;
	}
	
	
	//try to get available provider for hard code provider
	if (!empty($select_provider) && isset($provider_hash[$select_provider])){
		$res['provider'] = $select_provider;
		$res['by_rules'] = "Get provider $select_provider by selected provider rule.";
		return $res;
	}
	
	//try to get provider by SMS engine rules.
	for($i = 0; $i < count($GBL_SMS_ENGINE_RULES); $i++){
		$rules_type = strtolower($GBL_SMS_ENGINE_RULES[$i]);
		
		if ($rules_type === 'message_type_setting'){//by message type
			if (!empty($GBL_MESSAGE_TYPE_SETTING) && !empty($message_type)){
				$msgtype = strtolower($message_type);
				if (isset($GBL_MESSAGE_TYPE_SETTING[$msgtype]) && !empty($GBL_MESSAGE_TYPE_SETTING[$msgtype])){
					
					//try specify country provider first
					if (isset($GBL_MESSAGE_TYPE_SETTING[$msgtype]['country']) && !empty($GBL_MESSAGE_TYPE_SETTING[$msgtype]['country']) && !empty($dest_country)){
						$country_t = $GBL_MESSAGE_TYPE_SETTING[$msgtype]['country'];
						
						if (isset($country_t[$dest_country]) && !empty($country_t[$dest_country])){
							$pr_l      = $country_t[$dest_country]['provider'];
							$give_up_l = isset($country_t[$dest_country]['give_up'])? strtolower($country_t[$dest_country]['give_up']) : 'no';
							
							//try to get one provider which is not used before.
							for($j = 0; $j < count($pr_l); $j++){
								$pr_t = $pr_l[$j];
									
								if (isset($provider_hash[$pr_t])){
									$res['provider'] = $pr_t;
									$res['give_up']  = $give_up_l;
									$res['by_rules'] = "Get provider $pr_t by message type $msgtype with country code $dest_country selected.";
									return $res;
								}
							}
							
							if ($give_up_l === 'yes'){
								$res['by_rules'] = "Failed to get provider by message type setting with country code $dest_country then give up.";
								$res['give_up'] = 'yes';
								return $res;
							}
						}						
					}
					
					//then try the default provider
					if (isset($GBL_MESSAGE_TYPE_SETTING[$msgtype]['provider']) && !empty($GBL_MESSAGE_TYPE_SETTING[$msgtype]['provider'])){
						$pr_l      = $GBL_MESSAGE_TYPE_SETTING[$msgtype]['provider'];
						$give_up_l = isset($GBL_MESSAGE_TYPE_SETTING[$msgtype]['give_up'])? strtolower($GBL_MESSAGE_TYPE_SETTING[$msgtype]['give_up']) : 'no';
						
						//try to get one provider which is not used before.
						for($j = 0; $j < count($pr_l); $j++){
							$pr_t = $pr_l[$j];
							
							if (isset($provider_hash[$pr_t])){
								$res['provider'] = $pr_t;
								$res['give_up']  = $give_up_l;
								$res['by_rules'] = "Get provider $pr_t by message type $msgtype selected.";
								return $res;
							}
						}
						
						if ($give_up_l === 'yes'){
							$res['by_rules'] = 'Failed to get provider by message type setting then give up.';
							$res['give_up'] = 'yes';
							return $res;
						}
					}					
				}
			}			
		}
		else if ($rules_type == 'country_providers_setting'){//by counry
			if (!empty($GBL_COUNTRY_PROVIDERS_SETTING) && !empty($dest_country)){
				$dest_country = strtoupper($dest_country);
				if (isset($GBL_COUNTRY_PROVIDERS_SETTING[$dest_country]) && !empty($GBL_COUNTRY_PROVIDERS_SETTING[$dest_country])){
					if (isset($GBL_COUNTRY_PROVIDERS_SETTING[$dest_country]['provider']) && !empty($GBL_COUNTRY_PROVIDERS_SETTING[$dest_country]['provider'])){
						$pr_l      = $GBL_COUNTRY_PROVIDERS_SETTING[$dest_country]['provider'];
						$give_up_l = isset($GBL_COUNTRY_PROVIDERS_SETTING[$dest_country]['give_up'])? strtolower($GBL_COUNTRY_PROVIDERS_SETTING[$dest_country]['give_up']) : 'no';
						
						//try to get one provider which is not used before.
						for($j = 0; $j < count($pr_l); $j++){
							$pr_t = $pr_l[$j];
								
							if (isset($provider_hash[$pr_t])){
								$res['provider'] = $pr_t;
								$res['give_up']  = $give_up_l;
								$res['by_rules'] = "Get provider $pr_t by country $dest_country selected.";
								return $res;
							}
						}
						
						if ($give_up_l === 'yes'){
							$res['by_rules'] = 'Failed to get provider by country provider setting then give up.';
							$res['give_up']  = 'yes';
							return $res;
						}
					}
				} 
			}			
		}	
		else{//by default
			if (isset($GBL_DEFAULT_SETTING['provider']) && !empty($GBL_DEFAULT_SETTING['provider'])){
				$pr_l = $GBL_DEFAULT_SETTING['provider'];
				for($j = 0; $j < count($pr_l); $j++){
					$pr_t = $pr_l[$j];
					if (isset($provider_hash[$pr_t])){
						$res['provider'] = $pr_t;
						$res['by_rules'] = "Get provider $pr_t by default rules.";
						return $res;
					}
				}				
			}
		}	
	}
	
	$res['by_rules'] = 'Failed to get provider by default rules then give up.';
	$res['give_up']  = 'yes';
	return $res;
}


?>