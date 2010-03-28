
#ifndef PHP_YPHP_YLEM_H
#define PHP_YPHP_YLEM_H

extern zend_module_entry yphp_ylem_module_entry;
#define phpext_yphp_ylem_ptr &yphp_ylem_module_entry

#ifdef PHP_WIN32
#define PHP_YPHP_YLEM_API __declspec(dllexport)
#else
#define PHP_YPHP_YLEM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(yphp_ylem);
PHP_MSHUTDOWN_FUNCTION(yphp_ylem);
PHP_RINIT_FUNCTION(yphp_ylem);
PHP_RSHUTDOWN_FUNCTION(yphp_ylem);
PHP_MINFO_FUNCTION(yphp_ylem);

PHP_FUNCTION(ylem_connect);					/* 创建连接 */
PHP_FUNCTION(ylem_close);					/* 关闭连接 */
PHP_FUNCTION(ylem_delete);					/* 删除文件 */
PHP_FUNCTION(ylem_putfile);					/* 上传文件 */
PHP_FUNCTION(ylem_getpaths);				/* 获取文件 */
PHP_FUNCTION(ylem_listkeys);				/* 获取keys */
PHP_FUNCTION(ylem_listfids);				/* 获取list */

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(yphp_ylem)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(yphp_ylem)
*/

/* In every utility function you add that needs to use variables 
   in php_yphp_ylem_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as YPHP_YLEM_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define YPHP_YLEM_G(v) TSRMG(yphp_ylem_globals_id, zend_yphp_ylem_globals *, v)
#else
#define YPHP_YLEM_G(v) (yphp_ylem_globals.v)
#endif

#endif	/* PHP_YPHP_YLEM_H */

