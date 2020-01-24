/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_SOURCE_GUARD_H
#define PHP_SOURCE_GUARD_H

extern zend_module_entry source_guard_module_entry;
#define phpext_source_guard_ptr &source_guard_module_entry

#define PHP_SOURCE_GUARD_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_SOURCE_GUARD_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_SOURCE_GUARD_API __attribute__ ((visibility("default")))
#else
#	define PHP_SOURCE_GUARD_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(source_guard)
	zend_long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(source_guard)
*/

/* Always refer to the globals in your function as SOURCE_GUARD_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define SOURCE_GUARD_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(source_guard, v)

#if defined(ZTS) && defined(COMPILE_DL_SOURCE_GUARD)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#define PATH_SIZE	(128)

static const int PL_ENCRYPT_BLOCK_SIZE				= 16;
static const unsigned char PL_INITIAL_VECTOR[]		= "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";	// change ok
static const unsigned char PL_DEFAULT_CRYPTKEY[]	= "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";	// change ok
static const unsigned char PL_TOOL_NAME[]			= "<?php /* SOURCE_GUARD */ ?>";

enum {
  ERR_NOTHING = 0,
  ERR_NOT_ENCRYPT,
  ERR_NOT_SOURCE_GUARD,
  EVP_FAIL_EVP_NEW,
  ERR_FAIL_EVP_INIT,
  ERR_FAIL_EVP_UPDATE,
  ERR_FAIL_EVP_FINAL,
};

typedef struct {
	char *mode;
	char *filename;

	FILE *fp_r;
	FILE *fp_w;

	char *data_raw;
	char *data_enc;

	int len_raw;
	int len_enc;

} SourceCryptBuffer;

void source_init();
int source_decrypt_openssl(const unsigned char* data_enc, const int len_enc, char* data_raw, int *len_raw);
int source_decrypt();

#endif	/* PHP_SOURCE_GUARD_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
