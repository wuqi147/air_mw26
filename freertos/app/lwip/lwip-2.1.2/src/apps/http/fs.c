/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "lwip/opt.h"
#include "lwip/apps/httpd_opts.h"

#if LWIP_HTTPD
#include "FreeRTOS.h"
#include "lwip/def.h"
#include "lwip/apps/fs.h"
#include <string.h>
#if defined(AIR_MW_SUPPORT) && !defined(AIR_LITE_HTTPD)
#include "mw_tlv.h"
#include "osapi_string.h"
#include "html_config_customer.h"
#endif

#include HTTPD_FSDATA_FILE

/*-----------------------------------------------------------------------------------*/

#if LWIP_HTTPD_CUSTOM_FILES
int fs_open_custom(struct fs_file *file, const char *name);
void fs_close_custom(struct fs_file *file);
#if LWIP_HTTPD_FS_ASYNC_READ
u8_t fs_canread_custom(struct fs_file *file);
u8_t fs_wait_read_custom(struct fs_file *file, fs_wait_cb callback_fn, void *callback_arg);
int fs_read_async_custom(struct fs_file *file, char *buffer, int count, fs_wait_cb callback_fn, void *callback_arg);
#else /* LWIP_HTTPD_FS_ASYNC_READ */
int fs_read_custom(struct fs_file *file, char *buffer, int count);
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
#endif /* LWIP_HTTPD_CUSTOM_FILES */

/*-----------------------------------------------------------------------------------*/
err_t
fs_open(struct fs_file *file, const char *name)
{
	const struct fsdata_file *f;

	if ((file == NULL) || (name == NULL)) {
		return ERR_ARG;
	}

#if defined(AIR_MW_SUPPORT) && !defined(AIR_LITE_HTTPD)
    if (0 == osapi_strcmp(name, TLV_DATA_LOGO_NAME))
    {
        UI32_T   logo_addr = 0x0;
        UI16_T   logo_len = 0;

        if(0 != (CUSTOMIZE_ITEM_LOAD_TLV_DATA & HTML_CUSTOMIZE_ITEM_BITMASK))
        {
            /* Load flash logo */
            if(MW_E_OK == mw_tlv_get_logo_info(&logo_addr, &logo_len))
            {
                file->data = (const char *)(logo_addr + TLV_DATA_LOGO_NAME_HEX_SIZE);
                file->len = (logo_len - TLV_DATA_LOGO_NAME_HEX_SIZE);
                file->index = (logo_len - TLV_DATA_LOGO_NAME_HEX_SIZE);
                file->pextension = NULL;
                file->flags = FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT | FS_FILE_FLAGS_HEADER_HTTPVER_1_1;
#if HTTPD_PRECALCULATED_CHECKSUM
                file->chksum_count = 0;
                file->chksum = NULL;
#endif /* HTTPD_PRECALCULATED_CHECKSUM */
#if LWIP_HTTPD_FILE_STATE
                file->state = fs_state_init(file, name);
#endif /* #if LWIP_HTTPD_FILE_STATE */
                return ERR_OK;
            }
            return ERR_VAL;
        }
    }
#endif /* AIR_MW_SUPPORT && !AIR_LITE_HTTPD */

#if LWIP_HTTPD_CUSTOM_FILES
	if (fs_open_custom(file, name)) {
		file->is_custom_file = 1;
		return ERR_OK;
	}
	file->is_custom_file = 0;
#endif /* LWIP_HTTPD_CUSTOM_FILES */

	for (f = FS_ROOT; f != NULL; f = f->next) {
		if (!strcmp(name, (const char *)f->name)) {
			file->data = (const char *)f->data;
			file->len = f->len;
			file->index = f->len;
			file->pextension = NULL;
			file->flags = f->flags;
#if HTTPD_PRECALCULATED_CHECKSUM
			file->chksum_count = f->chksum_count;
			file->chksum = f->chksum;
#endif /* HTTPD_PRECALCULATED_CHECKSUM */
#if LWIP_HTTPD_FILE_STATE
			file->state = fs_state_init(file, name);
#endif /* #if LWIP_HTTPD_FILE_STATE */
			return ERR_OK;
		}
	}
	/* file not found */
	return ERR_VAL;
}

/*-----------------------------------------------------------------------------------*/
void
fs_close(struct fs_file *file)
{
#if LWIP_HTTPD_CUSTOM_FILES
	if (file->is_custom_file) {
		fs_close_custom(file);
	}
#endif /* LWIP_HTTPD_CUSTOM_FILES */
#if LWIP_HTTPD_FILE_STATE
	fs_state_free(file, file->state);
#endif /* #if LWIP_HTTPD_FILE_STATE */
	LWIP_UNUSED_ARG(file);
}
/*-----------------------------------------------------------------------------------*/
#if LWIP_HTTPD_DYNAMIC_FILE_READ
#if LWIP_HTTPD_FS_ASYNC_READ
int
fs_read_async(struct fs_file *file, char *buffer, int count, fs_wait_cb callback_fn, void *callback_arg)
#else /* LWIP_HTTPD_FS_ASYNC_READ */
int
fs_read(struct fs_file *file, char *buffer, int count)
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
{
	int read;
	if (file->index == file->len) {
		return FS_READ_EOF;
	}
#if LWIP_HTTPD_FS_ASYNC_READ
	LWIP_UNUSED_ARG(callback_fn);
	LWIP_UNUSED_ARG(callback_arg);
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
#if LWIP_HTTPD_CUSTOM_FILES
	if (file->is_custom_file) {
#if LWIP_HTTPD_FS_ASYNC_READ
		return fs_read_async_custom(file, buffer, count, callback_fn, callback_arg);
#else /* LWIP_HTTPD_FS_ASYNC_READ */
		return fs_read_custom(file, buffer, count);
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
	}
#endif /* LWIP_HTTPD_CUSTOM_FILES */

	read = file->len - file->index;
	if (read > count) {
		read = count;
	}

	MEMCPY(buffer, (file->data + file->index), read);
	file->index += read;

	return (read);
}
#endif /* LWIP_HTTPD_DYNAMIC_FILE_READ */
/*-----------------------------------------------------------------------------------*/
#if LWIP_HTTPD_FS_ASYNC_READ
int
fs_is_file_ready(struct fs_file *file, fs_wait_cb callback_fn, void *callback_arg)
{
	if (file != NULL) {
#if LWIP_HTTPD_FS_ASYNC_READ
#if LWIP_HTTPD_CUSTOM_FILES
		if (!fs_canread_custom(file)) {
			if (fs_wait_read_custom(file, callback_fn, callback_arg)) {
				return 0;
			}
		}
#else /* LWIP_HTTPD_CUSTOM_FILES */
		LWIP_UNUSED_ARG(callback_fn);
		LWIP_UNUSED_ARG(callback_arg);
#endif /* LWIP_HTTPD_CUSTOM_FILES */
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
	}
	return 1;
}
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
/*-----------------------------------------------------------------------------------*/

static	unsigned char http_download_head_cfg[] = {
/* /romfile.cfg (13 chars) */
0x2f,0x72,0x6f,0x6d,0x66,0x69,0x6c,0x65,0x2e,0x63,0x66,0x67,0x00,0x00,0x00,0x00,

/* HTTP header */
/* "HTTP/1.0 200 OK
" (17 bytes) */
0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x30,0x20,0x32,0x30,0x30,0x20,0x4f,0x4b,0x0d,
0x0a,
/* "Server: lwIP/2.1.2 (http://savannah.nongnu.org/projects/lwip)
" (63 bytes) */
0x53,0x65,0x72,0x76,0x65,0x72,0x3a,0x20,0x6c,0x77,0x49,0x50,0x2f,0x32,0x2e,0x31,
0x2e,0x32,0x20,0x28,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x73,0x61,0x76,0x61,0x6e,
0x6e,0x61,0x68,0x2e,0x6e,0x6f,0x6e,0x67,0x6e,0x75,0x2e,0x6f,0x72,0x67,0x2f,0x70,
0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x2f,0x6c,0x77,0x69,0x70,0x29,0x0d,0x0a,
/* "Content-Length: 7
" (18+ bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x4c,0x65,0x6e,0x67,0x74,0x68,0x3a,0x20,
0x37,0x0d,0x0a,
/* "Content-Type: application/octet-stream

" (42 bytes) */
0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x54,0x79,0x70,0x65,0x3a,0x20,0x61,0x70,
0x70,0x6c,0x69,0x63,0x61,0x74,0x69,0x6f,0x6e,0x2f,0x6f,0x63,0x74,0x65,0x74,0x2d,
0x73,0x74,0x72,0x65,0x61,0x6d,0x0d,0x0a,0x0d,0x0a,

};


char *data = NULL;

err_t init_romfile_cfg_file(struct fs_file *file)
{
	int len = 0;

	if ((file == NULL)) {
		return ERR_ARG;
	}

#if LWIP_HTTPD_CUSTOM_FILES
	file->is_custom_file = 0;
#endif /* LWIP_HTTPD_CUSTOM_FILES */
        data = (char *)pvPortMalloc(1024, "lwip");
	memcpy(data, http_download_head_cfg, sizeof(http_download_head_cfg));
	len += snprintf(data + len + sizeof(http_download_head_cfg), 1024 - len - sizeof(http_download_head_cfg), "%s", "romfile=test");
	len += sizeof(http_download_head_cfg);
	file->data = data;
	file->len = len;
	file->index = len;
	file->pextension = NULL;
	file->flags = FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT;

	return ERR_OK;
}

int
fs_bytes_left(struct fs_file *file)
{
	return file->len - file->index;
}
#endif /* LWIP_HTTPD */
