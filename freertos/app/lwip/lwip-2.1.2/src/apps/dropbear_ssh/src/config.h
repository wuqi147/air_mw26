/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */
#ifndef DROPBEAR_CONFIG_H_
#define DROPBEAR_CONFIG_H_
#include "default_options.h"
#define	DROPBEAR_SERVER					1
#define DISABLE_ZLIB					1
#define	BUNDLED_LIBTOM					1
#define	HAVE_STRUCT_SOCKADDR_STORAGE	1
#define	HAVE_STRUCT_IN6_ADDR			1
#define	HAVE_STRUCT_ADDRINFO			1
#define	HAVE_DAEMON						1
#define	HAVE_GETUSERSHELL				1
#define	HAVE_FREEADDRINFO				1
#define	DISABLE_SYSLOG					1
#define	HAVE_GETADDRINFO				1
#define	HAVE_GAI_STRERROR				1
#define	HAVE_GETNAMEINFO				1
#define HAVE_CRYPT 						1
#define LTC_SOURCE 						1
#define BN_MP_INIT_C 					1
#define AF_UNIX 						999

#endif /* DROPBEAR_CONFIG_H_ */
