/*
 * This file is part of libbluray
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2009-2010  John Stebbins
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(__MINGW32__)
/* ftello64() and fseeko64() prototypes from stdio.h */
#   undef __STRICT_ANSI__
#endif

#include "file.h"
#include "util/macro.h"
#include "util/logging.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "../hibluray/hi_bluray.h"

#ifdef WIN32
#include <windows.h>
#define	ftello  _ftelli64
#define	fseeko  _fseeki64
#endif  //  #ifdef WIN32

static void file_close_linux(BD_FILE_H *file)
{
    if (file) {
        fclose((FILE *)file->internal);

        BD_DEBUG(DBG_FILE, "Closed LINUX file (%p)\n", (void*)file);

        X_FREE(file);
    }
}

static int64_t file_seek_linux(BD_FILE_H *file, int64_t offset, int32_t origin)
{
#if defined(__MINGW32__)
    return fseeko64((FILE *)file->internal, offset, origin);
#else
#ifdef ANDROID_VERSION
    return lseek64((int)fileno((FILE *)file->internal),offset, origin);
#else
    return fseeko((FILE *)file->internal, offset, origin);
#endif
#endif
}

static int64_t file_tell_linux(BD_FILE_H *file)
{
#if defined(__MINGW32__)
    return ftello64((FILE *)file->internal);
#else
#ifdef ANDROID_VERSION
    return  file->filesize;
#else
    return ftello((FILE *)file->internal);
#endif

#endif
}
#define TEST_TIME 0
#if TEST_TIME
static int64_t getTimeOfDay()
{
    struct timeval tv;
    gettimeofday(&tv,0);
    return (int64_t)(tv.tv_sec%10000000*1000+tv.tv_usec/1000);
}
#endif
static int file_eof_linux(BD_FILE_H *file)
{
#ifdef ANDROID_VERSION
    return lseek64((int)file->internal, 0, SEEK_CUR) >= file->filesize;
#else
    return feof((FILE *)file->internal);
#endif
}

#define BD_MAX_SSIZE ((int64_t)(((size_t)-1)>>1))

static int64_t file_read_linux(BD_FILE_H *file, uint8_t *buf, int64_t size)
{
#ifdef ANDROID_VERSION
    int ret;
    ret = read((int)fileno((FILE *)file->internal),buf,size);
    return ret;
#else
    if (size > 0 && size < BD_MAX_SSIZE) {
        return (int64_t)fread(buf, 1, (size_t)size, (FILE *)file->internal);
    }

    BD_DEBUG(DBG_FILE | DBG_CRIT, "Ignoring invalid read of size %"PRId64" (%p)\n", size, (void*)file);
    return 0;
#endif
}

static int64_t file_write_linux(BD_FILE_H *file, const uint8_t *buf, int64_t size)
{
    if (size > 0 && size < BD_MAX_SSIZE) {
        return (int64_t)fwrite(buf, 1, (size_t)size, (FILE *)file->internal);
    }

    BD_DEBUG(DBG_FILE | DBG_CRIT, "Ignoring invalid write of size %"PRId64" (%p)\n", size, (void*)file);
    return 0;
}

static BD_FILE_H *file_open_linux(const char* filename, const char *mode)
{
#ifdef HI_BLURAY_EXTENSION
    if (!strncmp(filename, "udf://", 6)) {
        BD_FILE_H * file =  file_open_udf(filename, mode);
        if (file) {
            return file;
        }
    }
#endif

    FILE *fp = NULL;
    BD_FILE_H *file = malloc(sizeof(BD_FILE_H));
    memset(file, 0, sizeof(BD_FILE_H));

    BD_DEBUG(DBG_FILE, "Opening LINUX file %s... (%p)\n", filename, (void*)file);
    file->close = file_close_linux;
    file->seek = file_seek_linux;
    file->read = file_read_linux;
    file->write = file_write_linux;
    file->tell = file_tell_linux;
    file->eof = file_eof_linux;

#ifdef WIN32
    wchar_t wfilename[MAX_PATH], wmode[8];
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename, -1, wfilename, MAX_PATH) &&
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, mode, -1, wmode, 8) &&
        (fp = _wfopen(wfilename, wmode))) {
#else
    if ((fp = fopen(filename, mode))) {
#endif
        file->internal = fp;
#ifdef ANDROID_VERSION
        int fd = (int)fileno((FILE *)file->internal);
        int64_t pos = lseek64(fd, 0, SEEK_CUR);
        file->filesize =  lseek64(fd, 0, SEEK_END);
        lseek64(fd, pos, SEEK_SET);
#endif
        return file;
    }

    BD_DEBUG(DBG_FILE, "Error opening file! (%p)\n", (void*)file);

    X_FREE(file);

    return NULL;
}

BD_FILE_H* (*file_open)(const char* filename, const char *mode) = file_open_linux;

BD_FILE_OPEN file_open_default(void)
{
    return file_open_linux;
}
