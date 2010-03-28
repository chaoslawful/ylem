#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_yphp_ylem.h"
#include "YlemClient.h"

using namespace::ylem;

// ugg
/* If you declare any globals in php_yphp_ylem.h uncomment this:
   ZEND_DECLARE_MODULE_GLOBALS(yphp_ylem)
   */

/* True global resources - no need for thread safety here */
static int le_ylem_client;
static YlemClient ylem_client;
#define le_ylem_client_name "ylem YlemClient"
static zend_class_entry *ylem_class_entry_ptr;

/* {{{ yphp_ylem_functions[]
 *
 * Every user visible function must have an entry in yphp_ylem_functions[].
 */
zend_function_entry yphp_ylem_functions[] = {
	PHP_FE(ylem_connect,	NULL)					/* 创建连接 */
		PHP_FE(ylem_close,	NULL)						/* 关闭连接 */
		PHP_FE(ylem_delete,	NULL)						/* 删除文件 */
		PHP_FE(ylem_putfile,	NULL)					/* 上传文件 */
		PHP_FE(ylem_getpaths,	NULL)					/* 获取文件的DAV */
		PHP_FE(ylem_listkeys,	NULL)					/* 查找keys */
		PHP_FE(ylem_listfids,	NULL)					/* 查找fids */
		{NULL, NULL, NULL}	/* Must be the last line in yphp_ylem_functions[] */
};

static zend_function_entry php_ylem_class_functions[] = {
	PHP_FALIAS(connect, ylem_connect, NULL)
		PHP_FALIAS(close, ylem_close, NULL)
		PHP_FALIAS(delete, ylem_delete, NULL)
		PHP_FALIAS(putfile, ylem_putfile, NULL)
		PHP_FALIAS(getpaths, ylem_getpaths, NULL)
		PHP_FALIAS(listkeys, ylem_listkeys, NULL)
		PHP_FALIAS(listfids, ylem_listfids, NULL)
		{NULL, NULL, NULL}
};

/* }}} */

/* {{{ yphp_ylem_module_entry
*/

//static zend_module_dep yphp_ylem_module_deps[] = {
//	    {NULL, NULL, NULL, 0}
//};

zend_module_entry yphp_ylem_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
	//	yphp_ylem_module_deps,
#endif
	"yphp_ylem",
	yphp_ylem_functions,
	PHP_MINIT(yphp_ylem),
	PHP_MSHUTDOWN(yphp_ylem),
	PHP_RINIT(yphp_ylem),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(yphp_ylem),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(yphp_ylem),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_YPHP_YLEM
ZEND_GET_MODULE(yphp_ylem)
#endif

	/* {{{ internal function protos */
	YlemState ylem_parse_response_to_array(INTERNAL_FUNCTION_PARAMETERS, const char * const, int);
	/* }}} */

	/* {{{ ylem default_link */

	//static void ylem_set_default_link(int id TSRMLS_DC)
	//{
	//    if (MOGILEFS_G(default_link) != -1) {
	//        zend_list_delete(MOGILEFS_G(default_link));
	//    }
	//    MOGILEFS_G(default_link) = id;
	//    zend_list_addref(id);
	//}

	//static int ylem_get_default_link(INTERNAL_FUNCTION_PARAMETERS)
	//{
	//    return MOGILEFS_G(default_link);
	//}
	/* }}} */

	/* {{{ PHP_INI
	*/
	/* Remove comments and fill if you need to have entries in php.ini
	   PHP_INI_BEGIN()
	   STD_PHP_INI_ENTRY("yphp_ylem.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_yphp_ylem_globals, yphp_ylem_globals)
	   STD_PHP_INI_ENTRY("yphp_ylem.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_yphp_ylem_globals, yphp_ylem_globals)
	   PHP_INI_END()
	   */
	/* }}} */

	/* {{{ php_yphp_ylem_init_globals
	*/
	// Uncomment this function if you have INI entries
	//static void php_yphp_ylem_init_globals(zend_yphp_ylem_globals *yphp_ylem_globals)
	//{
	//	yphp_ylem_globals->global_value = 0;
	//	yphp_ylem_globals->global_string = NULL;
	//	yphp_ylem_globals->ylem_client;
	//}

	/* }}} */

	/* {{{ PHP_MINIT_FUNCTION
	*/
	//static void ylem_destructor_mogilefs_sock(zend_rsrc_list_entry * rsrc TSRMLS_DC)
	//{
	//	YlemClient *ylem_client = (YlemClient *) rsrc->ptr;
	//}

PHP_MINIT_FUNCTION(yphp_ylem)
{
	zend_class_entry ylem_class_entry;
	INIT_CLASS_ENTRY(ylem_class_entry, "YlemClient", php_ylem_class_functions);
	ylem_class_entry_ptr = zend_register_internal_class(&ylem_class_entry TSRMLS_CC);

	//	le_ylem_client = zend_register_list_destructors_ex
	//		(ylem_destructor_mogilefs_sock, NULL, le_ylem_client_name, module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(yphp_ylem)
{
	//	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
*/
PHP_RINIT_FUNCTION(yphp_ylem)
{
	//	MOGILEFS_G(default_link) = -1
	return SUCCESS;
}
/* }}} */

// ugg
/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
*/
PHP_RSHUTDOWN_FUNCTION(yphp_ylem)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(yphp_ylem)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "yphp_ylem support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	   DISPLAY_INI_ENTRIES();
	   */
}
/* }}} */


YlemState ylem_parse_response_to_array(INTERNAL_FUNCTION_PARAMETERS, const char * const result, int result_len) { /* {{{ */
	char *l_key_val, *last, *token, *splited_key, *t_data, *cur_key = NULL;
	int t_data_len;

	if ((token = estrndup(result, result_len)) == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Out of memory");
		return YLEM_ERR;
	}
	array_init(return_value);

	for ((l_key_val = strtok_r(token, "&", &last)); l_key_val;
			(l_key_val = strtok_r(NULL, "&", &last))) {
		zval *data;

		if ((splited_key = estrdup(l_key_val)) == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Out of memory");
			return YLEM_ERR;
		}
		MAKE_STD_ZVAL(data);
		if ((splited_key = strtok(splited_key, "=")) == NULL) {
			// some return values can be null
			// return -1;
			splited_key ="\0";
		}
		asprintf(&cur_key, "%s", splited_key);
		if ((splited_key = strtok(NULL, "=")) == NULL) {
			// some return values can be null
			// return -1;
			splited_key ="\0";
		}
		t_data_len = spprintf(&t_data, 0, "%s", splited_key);
		ZVAL_STRINGL(data, t_data, t_data_len, 1);
		add_assoc_zval(return_value, cur_key, data);
		efree(t_data);
	}
	free(cur_key);
	return YLEM_OK;

}
/* }}} */

//YlemClient* ylem_client_server_init(char *host, unsigned short port, char * domain, char *classname) /* {{{ */
//{
//	YlemClient *ylem_client = new YlemClient();
//	if(ylem_client(host,port,domain,classname) == false)	
//	{
//		delete ylem_client;
//		ylem_client = NULL;
//	}
//	return ylem_client;
//}
/* }}} */

/* {{{ proto string ylem_connect(string host, string port ,string domain, string class)
 *     Initialize a new ylemServer Session */
PHP_FUNCTION(ylem_connect)
{
	/* 声明变量 */
	char *m_host = NULL, *m_domain = NULL,*m_class = NULL;
	int  m_host_len, m_port, m_domain_len, m_class_len, id;
	zval *mg_object = getThis();
	/* 获取参数 */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slss",
				&m_host, &m_host_len, &m_port,
				&m_domain, &m_domain_len, &m_class, &m_class_len) == FAILURE) {
		RETURN_FALSE;
	}

	/* 创建连接 */
	bool b=ylem_client.ylem_connect(m_host,m_port,m_domain,m_class);
	if(!b){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 
				"Can't connect to %s:%d", m_host, m_port);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string ylem_close()
   Close a MogileFS Session */

PHP_FUNCTION(ylem_close)
{
	/* 不做任何操作，直接返回 */
	RETURN_TRUE;
}

/* }}} */

/* {{{ proto string ylem_delete(string key)
   Delete a MogileFS file */

PHP_FUNCTION(ylem_delete)
{
	/* 声明变量 */
	char *m_key = NULL, *m_domain = NULL;
	int m_key_len, m_domain_len;
	/* 获取参数 */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|s",
				&m_key, &m_key_len, &m_domain, &m_domain_len) == FAILURE)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "parameters error");
		RETURN_FALSE;
	}

	/* 执行删除操作 */
	YlemState state;
	if(m_domain == NULL)
		state = ylem_client.ylem_delete(m_key);
	else
		state = ylem_client.ylem_delete(m_key,m_domain);

	if(state == YLEM_OK)
		RETURN_TRUE;

	RETURN_FALSE;

}
/*}}} */

/* {{{ proto string ylem_putfile(string key, string file, string domain ,string class)
 *     Get MogileFS path */
PHP_FUNCTION(ylem_putfile)
{
	/* 声明变量 */
	char *m_key = NULL, *m_file = NULL,*m_domain = NULL, *m_class = NULL;
	int m_key_len, m_file_len,m_domain_len,m_class_len;
	/* 获取参数 */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss|ss",
				&m_key, &m_key_len, &m_file, &m_file_len,
				&m_domain, &m_domain_len, &m_class, &m_class_len) == FAILURE)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "parameters error");
		RETURN_FALSE;
	}

	string strDomain, strClass;
	if(m_domain != NULL)
		strDomain = m_domain;
	if(m_class != NULL)
		strClass = m_class;
	/* 上传文件操作 */
	YlemState state = ylem_client.ylem_putfile
		(m_key, m_file,strDomain, strClass);
	if(state == YLEM_OK)
		RETURN_TRUE;

	RETURN_FALSE;

}
/* }}} */

/* {{{ proto string ylem_getpaths(string& key, string& domain)
 *     Get ylemServer hosts */
PHP_FUNCTION(ylem_getpaths)
{
	/* 声明变量 */
	char *m_key = NULL, *m_domain = NULL;
	int m_key_len,m_domain_len;
	/* 获取参数 */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|s",
				&m_key, &m_key_len,&m_domain, &m_domain_len) == FAILURE)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "parameters error");
		RETURN_FALSE;
	}

	string strDomain, strClass;
	if(m_domain != NULL)
		strDomain = m_domain;
	/* 执行get操作 */
	string strResponse;
	string mimetype;
	YlemState state = ylem_client.ylem_getpaths(strResponse,mimetype,m_key,strDomain);

	if(state == YLEM_ERR)
		RETURN_FALSE; 

	/* 创建返回数组 */
	const char* response=strResponse.c_str();
	if(strResponse.c_str()[0]=='O')
		response+=3;

	if ((state = ylem_parse_response_to_array(INTERNAL_FUNCTION_PARAM_PASSTHRU, 
					response, strResponse.length())) != YLEM_OK)
	{
		RETURN_FALSE;
	}

}
/* }}} */

/* {{{ proto string ylem_list_keys(string prefix, string after, int limit, string domain)
 *     Get ylemServer file keys */
PHP_FUNCTION(ylem_listkeys)
{
	/* 声明变量 */
	char *m_prefix = NULL, *m_after = NULL,*m_domain = NULL;
	int m_prefix_len,m_after_len,m_domain_len;
	int m_limit=100;

	/* 获取参数 */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ssl|s",
				&m_prefix, &m_prefix_len,
				&m_after,&m_after_len,&m_limit,
				&m_domain, &m_domain_len) == FAILURE)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "parameters error");
		RETURN_FALSE;
	}

	string strDomain;
	if(m_domain != NULL)
		strDomain = m_domain;
	/* 执行list keys操作 */
	string strResponse;
	YlemState state = ylem_client.ylem_listkeys(strResponse,
			m_prefix,m_after,m_limit,strDomain);

	if(state == YLEM_ERR)
		RETURN_FALSE; 

	/* 创建返回数组 */
	const char* response=strResponse.c_str();
	if(strResponse.c_str()[0]=='O')
		response+=3;

	if ((state = ylem_parse_response_to_array(INTERNAL_FUNCTION_PARAM_PASSTHRU, 
					response, strResponse.length())) != YLEM_OK)
	{
		RETURN_FALSE;
	}
}

/* }}} */

/* {{{ proto string ylem_list_fids(long from,long to)
 *     Get ylemServer file ids */
PHP_FUNCTION(ylem_listfids)
{
	/* 声明变量 */
	int m_from=0,m_to=100;

	/* 获取参数 */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|ll",
				&m_from, &m_to) == FAILURE)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "parameters error");
		RETURN_FALSE;
	}

	/* 执行list keys操作 */
	string strResponse;
	YlemState state = ylem_client.ylem_listfids(strResponse,m_from,m_to);

	if(state == YLEM_ERR)
		RETURN_FALSE; 

	/* 创建返回数组 */
	const char* response=strResponse.c_str();
	if(strResponse.c_str()[0]=='O')
		response+=3;

	if ((state = ylem_parse_response_to_array(INTERNAL_FUNCTION_PARAM_PASSTHRU, 
					response, strResponse.length())) != YLEM_OK)
	{
		RETURN_FALSE;
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
