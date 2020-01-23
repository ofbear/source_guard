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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_source_guard.h"

#include <openssl/evp.h>
#include <openssl/aes.h>

/* If you declare any globals in php_source_guard.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(source_guard)
*/

SourceCryptBuffer pl_buffer;

/* True global resources - no need for thread safety here */
static int le_source_guard;

// init
void source_init()
{
	source_free();

	pl_buffer.mode = NULL;
	pl_buffer.filename = NULL;

	pl_buffer.fp_r = NULL;
	pl_buffer.fp_w = NULL;

	pl_buffer.data_raw = NULL;
	pl_buffer.data_enc = NULL;

	pl_buffer.len_raw = 0;
	pl_buffer.len_enc = 0;
}

// free memory
void source_free()
{
	if (pl_buffer.fp_r != NULL)
	{
		fclose(pl_buffer.fp_r);
	}
	if (pl_buffer.data_raw != NULL)
	{
		efree(pl_buffer.data_raw);
	}
	if (pl_buffer.data_enc != NULL)
	{
		efree(pl_buffer.data_enc);
	}
}

// decrypt wrapper
int source_decrypt_openssl(const unsigned char *data_enc, const int len_enc, char *data_raw, int *len_raw)
{
	EVP_CIPHER_CTX *ctx;
	int len_pad = 0;
	int len_raw_0 = 0;
	int len_raw_1 = 0;

	// new
	if (!(ctx = EVP_CIPHER_CTX_new()))
	{
		EVP_CIPHER_CTX_free(ctx);
		return ERR_FAIL_EVP_INIT;
	}

	// init
	// if you want to change encrypt method, you change "EVP_aes_256_cbc"
	// https://www.openssl.org/docs/man1.1.0/crypto/EVP_aes_256_cbc.html
	if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, PL_DEFAULT_CRYPTKEY, PL_INITIAL_VECTOR))
	{
		EVP_CIPHER_CTX_free(ctx);
		return ERR_FAIL_EVP_INIT;
	}

	// Execute
	if (!EVP_DecryptUpdate(ctx, (unsigned char *)data_raw, &len_raw_0, data_enc, len_enc))
	{
		EVP_CIPHER_CTX_free(ctx);
		return ERR_FAIL_EVP_UPDATE;
	}

	// padding
	if (!EVP_DecryptFinal_ex(ctx, (unsigned char *)(data_raw + len_raw_0), &len_raw_1))
	{
		EVP_CIPHER_CTX_free(ctx);
		return ERR_FAIL_EVP_UPDATE;
	}
	*len_raw = len_raw_0 + len_raw_1;
	memset(data_raw + *len_raw, 0x00, len_enc - *len_raw);

	EVP_CIPHER_CTX_free(ctx);

	return ERR_NOTHING;
}

int source_decrypt()
{
	struct stat stat_buf;
	unsigned char resultfile[128];
	int dec_code = ERR_NOTHING;

	// get file data
	fstat(fileno(pl_buffer.fp_r), &stat_buf);
	pl_buffer.len_enc = stat_buf.st_size;
	pl_buffer.data_enc = ecalloc(1, sizeof(char) * pl_buffer.len_enc);
	fread(pl_buffer.data_enc, pl_buffer.len_enc, 1, pl_buffer.fp_r);

	// check php code
	if (strstr((const char *)pl_buffer.data_enc, "<?php") != NULL)
	{
		return ERR_NOT_ENCRYPT;
	}

	// prepare memory for decrypt data
	pl_buffer.len_raw = pl_buffer.len_enc;
	pl_buffer.data_raw = ecalloc(1, sizeof(char) * pl_buffer.len_raw);

	// decrypt
	if ((dec_code = source_decrypt_openssl(pl_buffer.data_enc, pl_buffer.len_enc, pl_buffer.data_raw, &pl_buffer.len_raw)) != ERR_NOTHING)
	{
		return dec_code;
	}

	// tag check
	if (strstr((const char *)pl_buffer.data_raw, PL_TOOL_NAME) == NULL)
	{
		return ERR_NOT_SOURCE_GUARD;
	}

	// make file by raw data
	pl_buffer.fp_w = tmpfile();
	fwrite(pl_buffer.data_raw, pl_buffer.len_raw, 1, pl_buffer.fp_w);

	// shift to top ptr
	rewind(pl_buffer.fp_w);

	return ERR_NOTHING;
}

// origin compiler
ZEND_API zend_op_array *(*org_compile_file)(zend_file_handle *file_handle, int type TSRMLS_DC);

// my compiler
ZEND_API zend_op_array *source_guard_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC)
{

	// init
	source_init();

	// check open
	pl_buffer.fp_r = fopen(file_handle->filename, "r");
	if (!pl_buffer.fp_r)
	{
		// init
		source_init();
		return org_compile_file(file_handle, type);
	}

	// decrypt execute
	if (source_decrypt() != ERR_NOTHING)
	{
		// init
		source_init();
		return org_compile_file(file_handle, type);
	}

	// close file_handle
	switch (file_handle->type)
	{
		case ZEND_HANDLE_FILENAME:
		case ZEND_HANDLE_STREAM:
		case ZEND_HANDLE_MAPPED:
		default:
			// do nothing
			break;
		case ZEND_HANDLE_FD:
			close(file_handle->handle.fd);
			break;
		case ZEND_HANDLE_FP:
			fclose(file_handle->handle.fp);
			break;
	}

	// override setting
	file_handle->type = ZEND_HANDLE_FP;
	file_handle->opened_path = NULL;
	file_handle->handle.fp = pl_buffer.fp_w;

	// init
	source_init();

	return org_compile_file(file_handle, type);
}

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("source_guard.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_source_guard_globals, source_guard_globals)
    STD_PHP_INI_ENTRY("source_guard.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_source_guard_globals, source_guard_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_source_guard_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_source_guard_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "source_guard", arg);

	RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_source_guard_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_source_guard_init_globals(zend_source_guard_globals *source_guard_globals)
{
	source_guard_globals->global_value = 0;
	source_guard_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(source_guard)
{
	CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;

	org_compile_file = zend_compile_file;
	zend_compile_file = source_guard_compile_file;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(source_guard)
{
	CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;
	zend_compile_file = org_compile_file;

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(source_guard)
{
#if defined(COMPILE_DL_SOURCE_GUARD) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(source_guard)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(source_guard)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "source_guard support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

// source_guard
ZEND_BEGIN_ARG_INFO_EX(arginfo_source_guard, 0, 0, 1)	 // 1 	定義名（PHP_FE() マクロの第一引数として指定する文字列）
														 // 2 	未使用
														 // 3 	返却フラグ（1.参照を返す　0.受け取るだけ）
														 // 4 	必須の引数の数
ZEND_ARG_INFO(0, txt)									 // 1 	1.参照渡し　0.値渡し
														 // 2 	仮引数名
ZEND_END_ARG_INFO()										 // 終了


/* {{{ source_guard_functions[]
 *
 * Every user visible function must have an entry in source_guard_functions[].
 */
const zend_function_entry source_guard_functions[] = {
	PHP_FE(confirm_source_guard_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in source_guard_functions[] */
};
/* }}} */

/* {{{ source_guard_module_entry
 */
zend_module_entry source_guard_module_entry = {
	STANDARD_MODULE_HEADER,
	"source_guard",
	source_guard_functions,
	PHP_MINIT(source_guard),
	PHP_MSHUTDOWN(source_guard),
	PHP_RINIT(source_guard),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(source_guard),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(source_guard),
	PHP_SOURCE_GUARD_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SOURCE_GUARD
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(source_guard)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
