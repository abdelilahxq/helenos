/*
 * Copyright (c) 2013 Vojtech Horky
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup untar
 * @{
 */
/** @file
 */

#include <stdio.h>
#include <stdarg.h>
#include <untar.h>

const char *filename;

static int tar_open(tar_file_t *tar)
{
	FILE *file = fopen(filename, "rb");
	if (file == NULL)
		return errno;

	tar->data = (void *) file;
	return EOK;
}

static void tar_close(tar_file_t *tar)
{
	FILE *file = (FILE *) tar->data;
	fclose(file);
}

static size_t tar_read(tar_file_t *tar, void *data, size_t size)
{
	FILE *file = (FILE *) tar->data;
	return fread(data, 1, size, file);
}

static void tar_vreport(tar_file_t *tar, const char *fmt, va_list args)
{
	vfprintf(stderr, fmt, args);
}

tar_file_t tar = {
	.open = tar_open,
	.close = tar_close,

	.read = tar_read,
	.vreport = tar_vreport
};

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s tar-file\n", argv[0]);
		return 1;
	}

	filename = argv[1];
	return untar(&tar);
}

/** @}
 */
