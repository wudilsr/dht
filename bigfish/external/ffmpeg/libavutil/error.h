/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * error code definitions
 */

#ifndef AVUTIL_ERROR_H
#define AVUTIL_ERROR_H

#include <errno.h>
#include "avutil.h"

/**
 * @addtogroup lavu_error
 *
 * @{
 */


/* error handling */
#if EDOM > 0
#define AVERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#define AVUNERROR(e) (-(e)) ///< Returns a POSIX error code from a library function error return value.
#else
/* Some platforms have E* and errno already negated. */
#define AVERROR(e) (e)
#define AVUNERROR(e) (e)
#endif

#define FFERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))

#define AVERROR_BSF_NOT_FOUND      (-MKTAG(0xF8,'B','S','F')) ///< Bitstream filter not found
#define AVERROR_BUG                (-MKTAG( 'B','U','G','!')) ///< Internal bug, also see AVERROR_BUG2
#define AVERROR_DECODER_NOT_FOUND  (-MKTAG(0xF8,'D','E','C')) ///< Decoder not found
#define AVERROR_DEMUXER_NOT_FOUND  (-MKTAG(0xF8,'D','E','M')) ///< Demuxer not found
#define AVERROR_ENCODER_NOT_FOUND  (-MKTAG(0xF8,'E','N','C')) ///< Encoder not found
#define AVERROR_EOF                (-MKTAG( 'E','O','F',' ')) ///< End of file
#define AVERROR_EXIT               (-MKTAG( 'E','X','I','T')) ///< Immediate exit was requested; the called function should not be restarted
#define AVERROR_FILTER_NOT_FOUND   (-MKTAG(0xF8,'F','I','L')) ///< Filter not found
#define AVERROR_INVALIDDATA        (-MKTAG( 'I','N','D','A')) ///< Invalid data found when processing input
#define AVERROR_MUXER_NOT_FOUND    (-MKTAG(0xF8,'M','U','X')) ///< Muxer not found
#define AVERROR_OPTION_NOT_FOUND   (-MKTAG(0xF8,'O','P','T')) ///< Option not found
#define AVERROR_PATCHWELCOME       (-MKTAG( 'P','A','W','E')) ///< Not yet implemented in FFmpeg, patches welcome
#define AVERROR_PROTOCOL_NOT_FOUND (-MKTAG(0xF8,'P','R','O')) ///< Protocol not found
#define AVERROR_STREAM_NOT_FOUND   (-MKTAG(0xF8,'S','T','R')) ///< Stream not found
//z00228123 add for mms protocol
#define AVERROR_FILE_PARSE_FAILED  (-MKTAG( 'F','I','L','E')) ///<File parse failed
/**
 * This is semantically identical to AVERROR_BUG
 * it has been introduced in Libav after our AVERROR_BUG and with a modified value.
 */
#define AVERROR_BUG2               (-MKTAG( 'B','U','G',' '))
#define AVERROR_UNKNOWN            (-MKTAG( 'U','N','K','N')) ///< Unknown error, typically from an external library

/* HTTP & RTSP errors */
#define AVERROR_HTTP_BAD_REQUEST   FFERRTAG(0xF8,'4','0','0')
#define AVERROR_HTTP_UNAUTHORIZED  FFERRTAG(0xF8,'4','0','1')
#define AVERROR_HTTP_FORBIDDEN     FFERRTAG(0xF8,'4','0','3')
#define AVERROR_HTTP_NOT_FOUND     FFERRTAG(0xF8,'4','0','4')
#define AVERROR_HTTP_OTHER_4XX     FFERRTAG(0xF8,'4','X','X')
#define AVERROR_HTTP_SERVER_ERROR  FFERRTAG(0xF8,'5','X','X')

/**
 * Put a description of the AVERROR code errnum in errbuf.
 * In case of failure the global variable errno is set to indicate the
 * error. Even in case of failure av_strerror() will print a generic
 * error message indicating the errnum provided to errbuf.
 *
 * @param errnum      error code to describe
 * @param errbuf      buffer to which description is written
 * @param errbuf_size the size in bytes of errbuf
 * @return 0 on success, a negative value if a description for errnum
 * cannot be found
 */
int av_strerror(int errnum, char *errbuf, size_t errbuf_size);

/**
 * @}
 */

#endif /* AVUTIL_ERROR_H */
