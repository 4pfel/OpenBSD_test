/* ====================================================================
 * Copyright (c) 2010 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 */

#include <err.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/aes.h>
#include <openssl/modes.h>

/* XXX - something like this should be in the public headers. */
struct gcm128_context {
	uint64_t opaque[64];
};

struct gcm128_test {
	const uint8_t K[128];
	size_t K_len;
	const uint8_t IV[128];
	size_t IV_len;
	const uint8_t P[512];
	size_t P_len;
	const uint8_t A[128];
	size_t A_len;
	const uint8_t C[512];
	size_t C_len;
	const uint8_t T[16];
};

struct gcm128_test gcm128_tests[] = {
	{
		/* Test Case 1. */
		.K = {},
		.K_len = 16,
		.IV = {},
		.IV_len = 12,
		.P = {},
		.P_len = 0,
		.A = {},
		.A_len = 0,
		.C = {},
		.C_len = 0,
		.T = {
			0x58, 0xe2, 0xfc, 0xce, 0xfa, 0x7e, 0x30, 0x61,
			0x36, 0x7f, 0x1d, 0x57, 0xa4, 0xe7, 0x45, 0x5a,
		},
	},
	{
		/* Test Case 2. */
		.K = {},
		.K_len = 16,
		.IV = {},
		.IV_len = 12,
		.P = {},
		.P_len = 16,
		.A = {},
		.A_len = 0,
		.C = {
			0x03, 0x88, 0xda, 0xce, 0x60, 0xb6, 0xa3, 0x92,
			0xf3, 0x28, 0xc2, 0xb9, 0x71, 0xb2, 0xfe, 0x78,
		},
		.C_len = 16,
		.T = {
			0xab, 0x6e, 0x47, 0xd4, 0x2c, 0xec, 0x13, 0xbd,
			0xf5, 0x3a, 0x67, 0xb2, 0x12, 0x57, 0xbd, 0xdf,
		},
	},
	{
		/* Test Case 3. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
		},
		.K_len = 16,
		.IV = {
			0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
			0xde, 0xca, 0xf8, 0x88,
		},
		.IV_len = 12,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
		},
		.P_len = 64,
		.A = {},
		.A_len = 0,
		.C = {
			0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24,
			0x4b, 0x72, 0x21, 0xb7, 0x84, 0xd0, 0xd4, 0x9c,
			0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
			0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e,
			0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93, 0x1c,
			0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
			0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97,
			0x3d, 0x58, 0xe0, 0x91, 0x47, 0x3f, 0x59, 0x85,
		},
		.C_len = 64,
		.T = {
			0x4d, 0x5c, 0x2a, 0xf3, 0x27, 0xcd, 0x64, 0xa6,
			0x2c, 0xf3, 0x5a, 0xbd, 0x2b, 0xa6, 0xfa, 0xb4,
		}
	},
	{
		/* Test Case 4. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
		},
		.K_len = 16,
		.IV = {
			0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
			0xde, 0xca, 0xf8, 0x88,
		},
		.IV_len = 12,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39,
		},
		.P_len = 60,
		.A = {
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xab, 0xad, 0xda, 0xd2,
		},
		.A_len = 20,
		.C = {
			0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24,
			0x4b, 0x72, 0x21, 0xb7, 0x84, 0xd0, 0xd4, 0x9c,
			0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
			0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e,
			0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93, 0x1c,
			0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
			0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97,
			0x3d, 0x58, 0xe0, 0x91,
		},
		.C_len = 60,
		.T = {
			0x5b, 0xc9, 0x4f, 0xbc, 0x32, 0x21, 0xa5, 0xdb,
			0x94, 0xfa, 0xe9, 0x5a, 0xe7, 0x12, 0x1a, 0x47,
		},
	},
	{
		/* Test Case 5. */
		/* K, P, A are the same as TC4. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
		},
		.K_len = 16,
		.IV = {
			0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
		},
		.IV_len = 8,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39,
		},
		.P_len = 60,
		.A = {
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xab, 0xad, 0xda, 0xd2,
		},
		.A_len = 20,
		.C = {
			0x61, 0x35, 0x3b, 0x4c, 0x28, 0x06, 0x93, 0x4a,
			0x77, 0x7f, 0xf5, 0x1f, 0xa2, 0x2a, 0x47, 0x55,
			0x69, 0x9b, 0x2a, 0x71, 0x4f, 0xcd, 0xc6, 0xf8,
			0x37, 0x66, 0xe5, 0xf9, 0x7b, 0x6c, 0x74, 0x23,
			0x73, 0x80, 0x69, 0x00, 0xe4, 0x9f, 0x24, 0xb2,
			0x2b, 0x09, 0x75, 0x44, 0xd4, 0x89, 0x6b, 0x42,
			0x49, 0x89, 0xb5, 0xe1, 0xeb, 0xac, 0x0f, 0x07,
			0xc2, 0x3f, 0x45, 0x98,
		},
		.C_len = 60,
		.T = {
			0x36, 0x12, 0xd2, 0xe7, 0x9e, 0x3b, 0x07, 0x85,
			0x56, 0x1b, 0xe1, 0x4a, 0xac, 0xa2, 0xfc, 0xcb,
		},
	},
	{
		/* Test Case 6. */
		/* K, P, A are the same as TC4. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
		},
		.K_len = 16,
		.IV = {
			0x93, 0x13, 0x22, 0x5d, 0xf8, 0x84, 0x06, 0xe5,
			0x55, 0x90, 0x9c, 0x5a, 0xff, 0x52, 0x69, 0xaa,
			0x6a, 0x7a, 0x95, 0x38, 0x53, 0x4f, 0x7d, 0xa1,
			0xe4, 0xc3, 0x03, 0xd2, 0xa3, 0x18, 0xa7, 0x28,
			0xc3, 0xc0, 0xc9, 0x51, 0x56, 0x80, 0x95, 0x39,
			0xfc, 0xf0, 0xe2, 0x42, 0x9a, 0x6b, 0x52, 0x54,
			0x16, 0xae, 0xdb, 0xf5, 0xa0, 0xde, 0x6a, 0x57,
			0xa6, 0x37, 0xb3, 0x9b,
		},
		.IV_len = 60,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39,
		},
		.P_len = 60,
		.A = {
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xab, 0xad, 0xda, 0xd2,
		},
		.A_len = 20,
		.C = {
			0x8c, 0xe2, 0x49, 0x98, 0x62, 0x56, 0x15, 0xb6,
			0x03, 0xa0, 0x33, 0xac, 0xa1, 0x3f, 0xb8, 0x94,
			0xbe, 0x91, 0x12, 0xa5, 0xc3, 0xa2, 0x11, 0xa8,
			0xba, 0x26, 0x2a, 0x3c, 0xca, 0x7e, 0x2c, 0xa7,
			0x01, 0xe4, 0xa9, 0xa4, 0xfb, 0xa4, 0x3c, 0x90,
			0xcc, 0xdc, 0xb2, 0x81, 0xd4, 0x8c, 0x7c, 0x6f,
			0xd6, 0x28, 0x75, 0xd2, 0xac, 0xa4, 0x17, 0x03,
			0x4c, 0x34, 0xae, 0xe5,
		},
		.C_len = 60,
		.T = {
			0x61, 0x9c, 0xc5, 0xae, 0xff, 0xfe, 0x0b, 0xfa,
			0x46, 0x2a, 0xf4, 0x3c, 0x16, 0x99, 0xd0, 0x50,
		},
	},
	{
		/* Test Case 7. */
		.K = {},
		.K_len = 24,
		.IV = {},
		.IV_len = 12,
		.P = {},
		.P_len = 0,
		.A = {},
		.A_len = 0,
		.C = {},
		.C_len = 0,
		.T = {
			0xcd, 0x33, 0xb2, 0x8a, 0xc7, 0x73, 0xf7, 0x4b,
			0xa0, 0x0e, 0xd1, 0xf3, 0x12, 0x57, 0x24, 0x35,
		},
	},
	{
		/* Test Case 8. */
		.K = {},
		.K_len = 24,
		.IV = {},
		.IV_len = 12,
		.P = {},
		.P_len = 16,
		.A = {},
		.A_len = 0,
		.C = {
			0x98, 0xe7, 0x24, 0x7c, 0x07, 0xf0, 0xfe, 0x41,
			0x1c, 0x26, 0x7e, 0x43, 0x84, 0xb0, 0xf6, 0x00,
		},
		.C_len = 16,
		.T = {
			0x2f, 0xf5, 0x8d, 0x80, 0x03, 0x39, 0x27, 0xab,
			0x8e, 0xf4, 0xd4, 0x58, 0x75, 0x14, 0xf0, 0xfb,
		},
	},
	{
		/* Test Case 9. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
		},
		.K_len = 24,
		.IV = {
			0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
			0xde, 0xca, 0xf8, 0x88,
		},
		.IV_len = 12,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
		},
		.P_len = 64,
		.A = {},
		.A_len = 0,
		.C = {
			0x39, 0x80, 0xca, 0x0b, 0x3c, 0x00, 0xe8, 0x41,
			0xeb, 0x06, 0xfa, 0xc4, 0x87, 0x2a, 0x27, 0x57,
			0x85, 0x9e, 0x1c, 0xea, 0xa6, 0xef, 0xd9, 0x84,
			0x62, 0x85, 0x93, 0xb4, 0x0c, 0xa1, 0xe1, 0x9c,
			0x7d, 0x77, 0x3d, 0x00, 0xc1, 0x44, 0xc5, 0x25,
			0xac, 0x61, 0x9d, 0x18, 0xc8, 0x4a, 0x3f, 0x47,
			0x18, 0xe2, 0x44, 0x8b, 0x2f, 0xe3, 0x24, 0xd9,
			0xcc, 0xda, 0x27, 0x10, 0xac, 0xad, 0xe2, 0x56,
		},
		.C_len = 64,
		.T = {
			0x99, 0x24, 0xa7, 0xc8, 0x58, 0x73, 0x36, 0xbf,
			0xb1, 0x18, 0x02, 0x4d, 0xb8, 0x67, 0x4a, 0x14,
		},
	},
	{
		/* Test Case 10. */
		/* K and IV are the same as TC9. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
		},
		.K_len = 24,
		.IV = {
			0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
			0xde, 0xca, 0xf8, 0x88,
		},
		.IV_len = 12,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39,
		},
		.P_len = 60,
		.A = {
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xab, 0xad, 0xda, 0xd2,
		},
		.A_len = 20,
		.C = {
			0x39, 0x80, 0xca, 0x0b, 0x3c, 0x00, 0xe8, 0x41,
			0xeb, 0x06, 0xfa, 0xc4, 0x87, 0x2a, 0x27, 0x57,
			0x85, 0x9e, 0x1c, 0xea, 0xa6, 0xef, 0xd9, 0x84,
			0x62, 0x85, 0x93, 0xb4, 0x0c, 0xa1, 0xe1, 0x9c,
			0x7d, 0x77, 0x3d, 0x00, 0xc1, 0x44, 0xc5, 0x25,
			0xac, 0x61, 0x9d, 0x18, 0xc8, 0x4a, 0x3f, 0x47,
			0x18, 0xe2, 0x44, 0x8b, 0x2f, 0xe3, 0x24, 0xd9,
			0xcc, 0xda, 0x27, 0x10,
		},
		.C_len = 60,
		.T = {
			0x25, 0x19, 0x49, 0x8e, 0x80, 0xf1, 0x47, 0x8f,
			0x37, 0xba, 0x55, 0xbd, 0x6d, 0x27, 0x61, 0x8c,
		},
	},
	{
		/* Test Case 11. */
		/* K is the same as TC9, P and A are the same as TC10. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
		},
		.K_len = 24,
		.IV = {
			0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
		},
		.IV_len = 8,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39,
		},
		.P_len = 60,
		.A = {
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xab, 0xad, 0xda, 0xd2,
		},
		.A_len = 20,
		.C = {
			0x0f, 0x10, 0xf5, 0x99, 0xae, 0x14, 0xa1, 0x54,
			0xed, 0x24, 0xb3, 0x6e, 0x25, 0x32, 0x4d, 0xb8,
			0xc5, 0x66, 0x63, 0x2e, 0xf2, 0xbb, 0xb3, 0x4f,
			0x83, 0x47, 0x28, 0x0f, 0xc4, 0x50, 0x70, 0x57,
			0xfd, 0xdc, 0x29, 0xdf, 0x9a, 0x47, 0x1f, 0x75,
			0xc6, 0x65, 0x41, 0xd4, 0xd4, 0xda, 0xd1, 0xc9,
			0xe9, 0x3a, 0x19, 0xa5, 0x8e, 0x8b, 0x47, 0x3f,
			0xa0, 0xf0, 0x62, 0xf7
		},
		.C_len = 60,
		.T = {
			0x65, 0xdc, 0xc5, 0x7f, 0xcf, 0x62, 0x3a, 0x24,
			0x09, 0x4f, 0xcc, 0xa4, 0x0d, 0x35, 0x33, 0xf8,
		},
	},
	{
		/* Test Case 12. */
		/* K is the same as TC9, P and A are the same as TC10. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
		},
		.K_len = 24,
		.IV = {
			0x93, 0x13, 0x22, 0x5d, 0xf8, 0x84, 0x06, 0xe5,
			0x55, 0x90, 0x9c, 0x5a, 0xff, 0x52, 0x69, 0xaa,
			0x6a, 0x7a, 0x95, 0x38, 0x53, 0x4f, 0x7d, 0xa1,
			0xe4, 0xc3, 0x03, 0xd2, 0xa3, 0x18, 0xa7, 0x28,
			0xc3, 0xc0, 0xc9, 0x51, 0x56, 0x80, 0x95, 0x39,
			0xfc, 0xf0, 0xe2, 0x42, 0x9a, 0x6b, 0x52, 0x54,
			0x16, 0xae, 0xdb, 0xf5, 0xa0, 0xde, 0x6a, 0x57,
			0xa6, 0x37, 0xb3, 0x9b,
		},
		.IV_len = 60,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39,
		},
		.P_len = 60,
		.A = {
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xab, 0xad, 0xda, 0xd2,
		},
		.A_len = 20,
		.C = {
			0xd2, 0x7e, 0x88, 0x68, 0x1c, 0xe3, 0x24, 0x3c,
			0x48, 0x30, 0x16, 0x5a, 0x8f, 0xdc, 0xf9, 0xff,
			0x1d, 0xe9, 0xa1, 0xd8, 0xe6, 0xb4, 0x47, 0xef,
			0x6e, 0xf7, 0xb7, 0x98, 0x28, 0x66, 0x6e, 0x45,
			0x81, 0xe7, 0x90, 0x12, 0xaf, 0x34, 0xdd, 0xd9,
			0xe2, 0xf0, 0x37, 0x58, 0x9b, 0x29, 0x2d, 0xb3,
			0xe6, 0x7c, 0x03, 0x67, 0x45, 0xfa, 0x22, 0xe7,
			0xe9, 0xb7, 0x37, 0x3b,
		},
		.C_len = 60,
		.T = {
			0xdc, 0xf5, 0x66, 0xff, 0x29, 0x1c, 0x25, 0xbb,
			0xb8, 0x56, 0x8f, 0xc3, 0xd3, 0x76, 0xa6, 0xd9,
		},
	},
	{
		/* Test Case 13. */
		.K = {},
		.K_len = 32,
		.IV = {},
		.IV_len = 12,
		.P = {},
		.P_len = 0,
		.A = {},
		.A_len = 0,
		.C = {},
		.C_len = 0,
		.T = {
			0x53, 0x0f, 0x8a, 0xfb, 0xc7, 0x45, 0x36, 0xb9,
			0xa9, 0x63, 0xb4, 0xf1, 0xc4, 0xcb, 0x73, 0x8b,
		},
	},
	{
		/* Test Case 14. */
		.K = {},
		.K_len = 32,
		.IV = {},
		.IV_len = 12,
		.P = {},
		.P_len = 16,
		.A = {},
		.A_len = 0,
		.C = {
			0xce, 0xa7, 0x40, 0x3d, 0x4d, 0x60, 0x6b, 0x6e,
			0x07, 0x4e, 0xc5, 0xd3, 0xba, 0xf3, 0x9d, 0x18,
		},
		.C_len = 16,
		.T = {
			0xd0, 0xd1, 0xc8, 0xa7, 0x99, 0x99, 0x6b, 0xf0,
			0x26, 0x5b, 0x98, 0xb5, 0xd4, 0x8a, 0xb9, 0x19,
		},
	},
	{
		/* Test Case 15. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
		},
		.K_len = 32,
		.IV = {
			0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
			0xde, 0xca, 0xf8, 0x88,
		},
		.IV_len = 12,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
		},
		.P_len = 64,
		.A = {},
		.A_len = 0,
		.C = {
			0x52, 0x2d, 0xc1, 0xf0, 0x99, 0x56, 0x7d, 0x07,
			0xf4, 0x7f, 0x37, 0xa3, 0x2a, 0x84, 0x42, 0x7d,
			0x64, 0x3a, 0x8c, 0xdc, 0xbf, 0xe5, 0xc0, 0xc9,
			0x75, 0x98, 0xa2, 0xbd, 0x25, 0x55, 0xd1, 0xaa,
			0x8c, 0xb0, 0x8e, 0x48, 0x59, 0x0d, 0xbb, 0x3d,
			0xa7, 0xb0, 0x8b, 0x10, 0x56, 0x82, 0x88, 0x38,
			0xc5, 0xf6, 0x1e, 0x63, 0x93, 0xba, 0x7a, 0x0a,
			0xbc, 0xc9, 0xf6, 0x62, 0x89, 0x80, 0x15, 0xad,
		},
		.C_len = 64,
		.T = {
			0xb0, 0x94, 0xda, 0xc5, 0xd9, 0x34, 0x71, 0xbd,
			0xec, 0x1a, 0x50, 0x22, 0x70, 0xe3, 0xcc, 0x6c,
		},
	},
	{
		/* Test Case 16. */
		/* K and IV are the same as TC15. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
		},
		.K_len = 32,
		.IV = {
			0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
			0xde, 0xca, 0xf8, 0x88,
		},
		.IV_len = 12,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39,
		},
		.P_len = 60,
		.A = {
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xab, 0xad, 0xda, 0xd2,
		},
		.A_len = 20,
		.C = {
			0x52, 0x2d, 0xc1, 0xf0, 0x99, 0x56, 0x7d, 0x07,
			0xf4, 0x7f, 0x37, 0xa3, 0x2a, 0x84, 0x42, 0x7d,
			0x64, 0x3a, 0x8c, 0xdc, 0xbf, 0xe5, 0xc0, 0xc9,
			0x75, 0x98, 0xa2, 0xbd, 0x25, 0x55, 0xd1, 0xaa,
			0x8c, 0xb0, 0x8e, 0x48, 0x59, 0x0d, 0xbb, 0x3d,
			0xa7, 0xb0, 0x8b, 0x10, 0x56, 0x82, 0x88, 0x38,
			0xc5, 0xf6, 0x1e, 0x63, 0x93, 0xba, 0x7a, 0x0a,
			0xbc, 0xc9, 0xf6, 0x62,
		},
		.C_len = 60,
		.T = {
			0x76, 0xfc, 0x6e, 0xce, 0x0f, 0x4e, 0x17, 0x68,
			0xcd, 0xdf, 0x88, 0x53, 0xbb, 0x2d, 0x55, 0x1b,
		},
	},
	{
		/* Test Case 17. */
		/* K is the same as TC15, P and A are the same as TC 16. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
		},
		.K_len = 32,
		.IV = {
			0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
		},
		.IV_len = 8,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39,
		},
		.P_len = 60,
		.A = {
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xab, 0xad, 0xda, 0xd2,
		},
		.A_len = 20,
		.C = {
			0xc3, 0x76, 0x2d, 0xf1, 0xca, 0x78, 0x7d, 0x32,
			0xae, 0x47, 0xc1, 0x3b, 0xf1, 0x98, 0x44, 0xcb,
			0xaf, 0x1a, 0xe1, 0x4d, 0x0b, 0x97, 0x6a, 0xfa,
			0xc5, 0x2f, 0xf7, 0xd7, 0x9b, 0xba, 0x9d, 0xe0,
			0xfe, 0xb5, 0x82, 0xd3, 0x39, 0x34, 0xa4, 0xf0,
			0x95, 0x4c, 0xc2, 0x36, 0x3b, 0xc7, 0x3f, 0x78,
			0x62, 0xac, 0x43, 0x0e, 0x64, 0xab, 0xe4, 0x99,
			0xf4, 0x7c, 0x9b, 0x1f,
		},
		.C_len = 60,
		.T = {
			0x3a, 0x33, 0x7d, 0xbf, 0x46, 0xa7, 0x92, 0xc4,
			0x5e, 0x45, 0x49, 0x13, 0xfe, 0x2e, 0xa8, 0xf2,
		},
	},
	{
		/* Test Case 18. */
		/* K is the same as TC15, P and A are the same as TC 16. */
		.K = {
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
			0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
			0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
		},
		.K_len = 32,
		.IV = {
			0x93, 0x13, 0x22, 0x5d, 0xf8, 0x84, 0x06, 0xe5,
			0x55, 0x90, 0x9c, 0x5a, 0xff, 0x52, 0x69, 0xaa,
			0x6a, 0x7a, 0x95, 0x38, 0x53, 0x4f, 0x7d, 0xa1,
			0xe4, 0xc3, 0x03, 0xd2, 0xa3, 0x18, 0xa7, 0x28,
			0xc3, 0xc0, 0xc9, 0x51, 0x56, 0x80, 0x95, 0x39,
			0xfc, 0xf0, 0xe2, 0x42, 0x9a, 0x6b, 0x52, 0x54,
			0x16, 0xae, 0xdb, 0xf5, 0xa0, 0xde, 0x6a, 0x57,
			0xa6, 0x37, 0xb3, 0x9b,
		},
		.IV_len = 60,
		.P = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39,
		},
		.P_len = 60,
		.A = {
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
			0xab, 0xad, 0xda, 0xd2,
		},
		.A_len = 20,
		.C = {
			0x5a, 0x8d, 0xef, 0x2f, 0x0c, 0x9e, 0x53, 0xf1,
			0xf7, 0x5d, 0x78, 0x53, 0x65, 0x9e, 0x2a, 0x20,
			0xee, 0xb2, 0xb2, 0x2a, 0xaf, 0xde, 0x64, 0x19,
			0xa0, 0x58, 0xab, 0x4f, 0x6f, 0x74, 0x6b, 0xf4,
			0x0f, 0xc0, 0xc3, 0xb7, 0x80, 0xf2, 0x44, 0x45,
			0x2d, 0xa3, 0xeb, 0xf1, 0xc5, 0xd8, 0x2c, 0xde,
			0xa2, 0x41, 0x89, 0x97, 0x20, 0x0e, 0xf8, 0x2e,
			0x44, 0xae, 0x7e, 0x3f,
		},
		.C_len = 60,
		.T = {
			0xa4, 0x4a, 0x82, 0x66, 0xee, 0x1c, 0x8e, 0xb0,
			0xc8, 0xb5, 0xd4, 0xcf, 0x5a, 0xe9, 0xf1, 0x9a,
		},
	},
	{
		/* Test Case 19. */
		.K = {},
		.K_len = 16,
		.IV = {},
		.IV_len = 12,
		.P = {},
		.P_len = 0,
		.A = {
			0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
			0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
			0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
			0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
			0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
			0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
			0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
			0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55,
			0x52, 0x2d, 0xc1, 0xf0, 0x99, 0x56, 0x7d, 0x07,
			0xf4, 0x7f, 0x37, 0xa3, 0x2a, 0x84, 0x42, 0x7d,
			0x64, 0x3a, 0x8c, 0xdc, 0xbf, 0xe5, 0xc0, 0xc9,
			0x75, 0x98, 0xa2, 0xbd, 0x25, 0x55, 0xd1, 0xaa,
			0x8c, 0xb0, 0x8e, 0x48, 0x59, 0x0d, 0xbb, 0x3d,
			0xa7, 0xb0, 0x8b, 0x10, 0x56, 0x82, 0x88, 0x38,
			0xc5, 0xf6, 0x1e, 0x63, 0x93, 0xba, 0x7a, 0x0a,
			0xbc, 0xc9, 0xf6, 0x62, 0x89, 0x80, 0x15, 0xad,
		},
		.A_len = 128,
		.C = {},
		.C_len = 0,
		.T = {
			0x5f, 0xea, 0x79, 0x3a, 0x2d, 0x6f, 0x97, 0x4d,
			0x37, 0xe6, 0x8e, 0x0c, 0xb8, 0xff, 0x94, 0x92,
		},
	},
	{
		/* Test Case 20. */
		.K = {},
		.K_len = 16,
		.IV = {
			/* This results in 0xff in counter LSB. */
			0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		},
		.IV_len = 64,
		.P = {},
		.P_len = 288,
		.A = {},
		.A_len = 0,
		.C = {
			0x56, 0xb3, 0x37, 0x3c, 0xa9, 0xef, 0x6e, 0x4a,
			0x2b, 0x64, 0xfe, 0x1e, 0x9a, 0x17, 0xb6, 0x14,
			0x25, 0xf1, 0x0d, 0x47, 0xa7, 0x5a, 0x5f, 0xce,
			0x13, 0xef, 0xc6, 0xbc, 0x78, 0x4a, 0xf2, 0x4f,
			0x41, 0x41, 0xbd, 0xd4, 0x8c, 0xf7, 0xc7, 0x70,
			0x88, 0x7a, 0xfd, 0x57, 0x3c, 0xca, 0x54, 0x18,
			0xa9, 0xae, 0xff, 0xcd, 0x7c, 0x5c, 0xed, 0xdf,
			0xc6, 0xa7, 0x83, 0x97, 0xb9, 0xa8, 0x5b, 0x49,
			0x9d, 0xa5, 0x58, 0x25, 0x72, 0x67, 0xca, 0xab,
			0x2a, 0xd0, 0xb2, 0x3c, 0xa4, 0x76, 0xa5, 0x3c,
			0xb1, 0x7f, 0xb4, 0x1c, 0x4b, 0x8b, 0x47, 0x5c,
			0xb4, 0xf3, 0xf7, 0x16, 0x50, 0x94, 0xc2, 0x29,
			0xc9, 0xe8, 0xc4, 0xdc, 0x0a, 0x2a, 0x5f, 0xf1,
			0x90, 0x3e, 0x50, 0x15, 0x11, 0x22, 0x13, 0x76,
			0xa1, 0xcd, 0xb8, 0x36, 0x4c, 0x50, 0x61, 0xa2,
			0x0c, 0xae, 0x74, 0xbc, 0x4a, 0xcd, 0x76, 0xce,
			0xb0, 0xab, 0xc9, 0xfd, 0x32, 0x17, 0xef, 0x9f,
			0x8c, 0x90, 0xbe, 0x40, 0x2d, 0xdf, 0x6d, 0x86,
			0x97, 0xf4, 0xf8, 0x80, 0xdf, 0xf1, 0x5b, 0xfb,
			0x7a, 0x6b, 0x28, 0x24, 0x1e, 0xc8, 0xfe, 0x18,
			0x3c, 0x2d, 0x59, 0xe3, 0xf9, 0xdf, 0xff, 0x65,
			0x3c, 0x71, 0x26, 0xf0, 0xac, 0xb9, 0xe6, 0x42,
			0x11, 0xf4, 0x2b, 0xae, 0x12, 0xaf, 0x46, 0x2b,
			0x10, 0x70, 0xbe, 0xf1, 0xab, 0x5e, 0x36, 0x06,
			0x87, 0x2c, 0xa1, 0x0d, 0xee, 0x15, 0xb3, 0x24,
			0x9b, 0x1a, 0x1b, 0x95, 0x8f, 0x23, 0x13, 0x4c,
			0x4b, 0xcc, 0xb7, 0xd0, 0x32, 0x00, 0xbc, 0xe4,
			0x20, 0xa2, 0xf8, 0xeb, 0x66, 0xdc, 0xf3, 0x64,
			0x4d, 0x14, 0x23, 0xc1, 0xb5, 0x69, 0x90, 0x03,
			0xc1, 0x3e, 0xce, 0xf4, 0xbf, 0x38, 0xa3, 0xb6,
			0x0e, 0xed, 0xc3, 0x40, 0x33, 0xba, 0xc1, 0x90,
			0x27, 0x83, 0xdc, 0x6d, 0x89, 0xe2, 0xe7, 0x74,
			0x18, 0x8a, 0x43, 0x9c, 0x7e, 0xbc, 0xc0, 0x67,
			0x2d, 0xbd, 0xa4, 0xdd, 0xcf, 0xb2, 0x79, 0x46,
			0x13, 0xb0, 0xbe, 0x41, 0x31, 0x5e, 0xf7, 0x78,
			0x70, 0x8a, 0x70, 0xee, 0x7d, 0x75, 0x16, 0x5c,
		},
		.C_len = 288,
		.T = {
			0x8b, 0x30, 0x7f, 0x6b, 0x33, 0x28, 0x6d, 0x0a,
			0xb0, 0x26, 0xa9, 0xed, 0x3f, 0xe1, 0xe8, 0x5f,
		},
	},
};

#define N_TESTS (sizeof(gcm128_tests) / sizeof(*gcm128_tests))

static int
do_gcm128_test(int test_no, struct gcm128_test *tv)
{
	GCM128_CONTEXT ctx;
	AES_KEY key;
	uint8_t *out;
	size_t out_len;
	int ret = 1;

	out_len = tv->P_len;
	out = malloc(out_len);
	if (out == NULL)
		err(1, "malloc");

	AES_set_encrypt_key(tv->K, tv->K_len * 8, &key);

	memset(out, 0, out_len);
	CRYPTO_gcm128_init(&ctx, &key, (block128_f)AES_encrypt);
	CRYPTO_gcm128_setiv(&ctx, tv->IV, tv->IV_len);
	if (tv->A_len > 0)
		CRYPTO_gcm128_aad(&ctx, tv->A, tv->A_len);
	if (tv->P_len > 0)
		CRYPTO_gcm128_encrypt(&ctx, tv->P, out, out_len);
	if (CRYPTO_gcm128_finish(&ctx, tv->T, 16)) {
		fprintf(stderr, "TEST %i: CRYPTO_gcm128_finish failed\n",
		    test_no);
		goto fail;
	}
	if (tv->C_len > 0 && memcmp(out, tv->C, out_len)) {
		fprintf(stderr, "TEST %i: encrypt failed\n", test_no);
		goto fail;
	}

	memset(out, 0, out_len);
	CRYPTO_gcm128_setiv(&ctx, tv->IV, tv->IV_len);
	if (tv->A_len > 0)
		CRYPTO_gcm128_aad(&ctx, tv->A, tv->A_len);
	if (tv->C_len > 0)
		CRYPTO_gcm128_decrypt(&ctx, tv->C, out, out_len);
	if (CRYPTO_gcm128_finish(&ctx, tv->T, 16)) {
		fprintf(stderr, "TEST %i: CRYPTO_gcm128_finish failed\n",
		    test_no);
		goto fail;
	}
	if (tv->P_len > 0 && memcmp(out, tv->P, out_len)) {
		fprintf(stderr, "TEST %i: decrypt failed\n", test_no);
		goto fail;
	}

	ret = 0;

fail:
	free(out);
	return (ret);
}

int
main(int argc, char **argv)
{
	int ret = 0;
	size_t i;

	for (i = 0; i < N_TESTS; i++)
		ret |= do_gcm128_test(i + 1, &gcm128_tests[i]);

	return ret;
}
