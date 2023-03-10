/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * - Neither name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/*!
 * \file
 *
 * Purpose: This file defines the functionality making use of the http.
 * It defines functions to receive messages, process messages, send messages.
 */

/*lint -save -e* */
#include <assert.h>
#include <stdarg.h>


#ifdef WIN32
    #include <malloc.h>
    #define fseeko fseek
#else
    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <sys/wait.h>
    #include <sys/utsname.h>
#endif
/*lint -restore * */


#include "config.h"

#include "httpreadwrite.h"

#include "unixutil.h"
#include "upnp.h"
#include "upnpapi.h"
#include "membuffer.h"
#include "uri.h"
#include "statcodes.h"
#include "sock.h"
#include "UpnpInet.h"
#include "UpnpIntTypes.h"
#include "UpnpStdInt.h"
#include "webserver.h"

/*
 * Please, do not change these to const int while MSVC cannot understand
 * const int in array dimensions.
 */
/*
const int CHUNK_HEADER_SIZE = 10;
const int CHUNK_TAIL_SIZE = 10;
*/
#define CHUNK_HEADER_SIZE 10
#define CHUNK_TAIL_SIZE 10

//#ifdef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS

/* in seconds */
#define DEFAULT_TCP_CONNECT_TIMEOUT 5

/*!
 * \brief Checks socket connection and wait if it is not connected.
 * It should be called just after connect.
 *
 * \return 0 if successful, else -1.
 */
static int Check_Connect_And_Wait_Connection(
    /*! [in] socket. */
    SOCKET sock,
    /*! [in] result of connect. */
    int connect_res,
    int timeout)
{
    struct timeval tmvTimeout;// = {0, 500 * 1000};
//        struct timeval tmvTimeout = {DEFAULT_TCP_CONNECT_TIMEOUT, 0};

    int result;
    tmvTimeout.tv_sec = timeout;
    tmvTimeout.tv_usec = 0;
#ifdef WIN32
    struct fd_set fdSet;
#else
    fd_set fdSet;
#endif
    FD_ZERO(&fdSet);
    FD_SET(sock, &fdSet);

    if (connect_res < 0) {
#ifdef WIN32
        if (WSAEWOULDBLOCK == WSAGetLastError() ) {
#else
        if (EINPROGRESS == errno ) {
#endif
            result = select(sock + 1, NULL, &fdSet, NULL, &tmvTimeout);
            if (result < 0) {
#ifdef WIN32
                /* WSAGetLastError(); */
#else
                /* errno */
#endif
                return -1;
            } else if (result == 0) {
                /* timeout */
                return -1;
#ifndef WIN32
            } else {
                int valopt = 0;
                socklen_t len = 0;
                if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *) &valopt, &len) < 0) {
                    /* failed to read delayed error */
                    return -1;
                } else if (valopt) {
                    /* delayed error = valopt */
                    return -1;
                }
#endif
            }
        }
    }

    return 0;
}
//#endif /* UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS */

static int private_connect(
    SOCKET sockfd,
    const struct sockaddr *serv_addr,
    socklen_t addrlen,
    int block,    //* t00204177, for DMP exit slowly
    int timeout)  //* 0,no block,timeout value to break; -1,block
{
//#ifdef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
    int ret;
    if( 0 == block)
    {
        ret = sock_make_no_blocking(sockfd);
        if (ret != - 1) {
            ret = connect(sockfd, serv_addr, addrlen);
            ret = Check_Connect_And_Wait_Connection(sockfd, ret, timeout);
            if (ret != - 1) {
                ret = sock_make_blocking(sockfd);
            }
        }
    }else
    {
        ret = connect(sockfd, serv_addr, addrlen);
    }
    return ret;
//#else
    //return connect(sockfd, serv_addr, addrlen);
//#endif /* UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS */
}

int http_FixUrl(IN uri_type *url, OUT uri_type *fixed_url)
{
    const char *temp_path = "/";

    *fixed_url = *url;
    if (token_string_casecmp(&fixed_url->scheme, "http") != 0) {
        return UPNP_E_INVALID_URL;
    }
    if( fixed_url->hostport.text.size == 0 ) {
        return UPNP_E_INVALID_URL;
    }
    /* set pathquery to "/" if it is empty */
    if (fixed_url->pathquery.size == 0) {
        fixed_url->pathquery.buff = temp_path;
        fixed_url->pathquery.size = 1;
    }

    return UPNP_E_SUCCESS;
}

int http_FixStrUrl(
    IN const char *urlstr,
    IN size_t urlstrlen,
    OUT uri_type *fixed_url)
{
    uri_type url;

    if (parse_uri(urlstr, urlstrlen, &url) != HTTP_SUCCESS) {
        return UPNP_E_INVALID_URL;
    }

    return http_FixUrl(&url, fixed_url);
}

/************************************************************************
 * Function: http_Connect
 *
 * Parameters:
 *    IN uri_type* destination_url;    URL containing destination information
 *    OUT uri_type *url;        Fixed and corrected URL
 *
 * Description:
 *    Gets destination address from URL and then connects to the remote end
 *
 *  Returns:
 *    socket descriptor on success
 *    UPNP_E_OUTOF_SOCKET
 *    UPNP_E_SOCKET_CONNECT on error
 ************************************************************************/
SOCKET http_Connect(
    IN uri_type *destination_url,
    OUT uri_type *url)
{
    SOCKET connfd;
    socklen_t sockaddr_len;
    int ret_connect;

    (void)http_FixUrl(destination_url, url);

    connfd = socket(url->hostport.IPaddress.ss_family, SOCK_STREAM, 0);
    if (connfd == -1) {
        return UPNP_E_OUTOF_SOCKET;
    }
    sockaddr_len = (socklen_t)(url->hostport.IPaddress.ss_family == AF_INET6 ?
        sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in));
    ret_connect = private_connect(connfd,
        (struct sockaddr *)&url->hostport.IPaddress, sockaddr_len,0,5 );//-1, -1);
    if (ret_connect == -1) {
#ifdef WIN32
        DLNA_LOGE("[VPPDLNA]:connect error: %d\n", WSAGetLastError());
#endif
        (void)shutdown(connfd, SD_BOTH);
        UpnpCloseSocket(connfd);
        return UPNP_E_SOCKET_CONNECT;
    }

    return connfd;
}

/*!
 * \brief Get the data on the socket and take actions based on the read data to
 * modify the parser objects buffer.
 *
 * If an error is reported while parsing the data, the error code is passed in
 * the http_errr_code parameter.
 *
 * Parameters:
 *    IN SOCKINFO *info;            Socket information object
 *    OUT http_parser_t* parser;        HTTP parser object
 *    IN http_method_t request_method;    HTTP request method
 *    IN OUT int* timeout_secs;        time out
 *    OUT int* http_error_code;        HTTP error code returned
 *
 * \return
 *      UPNP_E_SUCCESS
 *     UPNP_E_BAD_HTTPMSG
 */
int http_RecvMessage(
    IN SOCKINFO *info,
    OUT http_parser_t *parser,
    IN http_method_t request_method,
    IN OUT int *timeout_secs,
    OUT int *http_error_code,
    OUT int *pbIsMsgRecvComplete)
{
    int ret = UPNP_E_SUCCESS;
    parse_status_t status;
    int num_read;
    int ok_on_close = FALSE;
    char buf[2 * 1024];

    if (request_method == HTTPMETHOD_UNKNOWN) {
        parser_request_init(parser);
    } else {
        parser_response_init(parser, request_method);
    }


    while (TRUE) {
        //printf("[VPPDLNA][%s][%d]:In while loop-- info->socket = %d \n",__FUNCTION__,__LINE__, info->socket );
        //DLNA_LOGE("[VPPDLNA][%s][%d]:info->socket = %d \n",__FUNCTION__,__LINE__, info->socket );
        num_read = sock_read(info, buf, sizeof buf, timeout_secs);
      //  printf("[VPPDLNA][%s][%d]:In while loop-- num_read = %d \n",__FUNCTION__,__LINE__, num_read );
        if (num_read > 0) {
            /* got data */
            status = parser_append(parser, buf, (size_t)num_read);
            //printf("[VPPDLNA][%s][%d]:info->socket = %d \n",__FUNCTION__,__LINE__, info->socket );
            //DLNA_LOGE("[VPPDLNA][%s][%d]:status = %d \n",__FUNCTION__,__LINE__, status );
            if (status == PARSE_SUCCESS) {
                //printf("[VPPDLNA][%s][%d]:info->socket = %d & in PARSE_SUCCESS \n",__FUNCTION__,__LINE__, info->socket );
                DLNA_LOGV("[VPPDLNA][%s][%d]:<<< (RECVD) <<<\n%s\n------------"
                    " -----\n",__FUNCTION__,__LINE__, parser->msg.msg.buf );
                print_http_headers( &parser->msg );
                if (g_maxContentLength > 0 && parser->content_length > (unsigned int)g_maxContentLength) {
                    //printf("[VPPDLNA][%s][%d]:info->socket = %d & in PARSE_SUCCESS exiting \n",__FUNCTION__,__LINE__, info->socket );
                    DLNA_LOGE("[VPPDLNA][%s][%d]:Received Content length is  "
                        "More than the allowd. Received-%d Allowed-%d\n",
                        __FUNCTION__ ,__LINE__,parser->content_length,
                        g_maxContentLength);
                    *http_error_code = HTTP_REQ_ENTITY_TOO_LARGE;
                    ret = UPNP_E_OUTOF_BOUNDS;
                    goto ExitFunction;
                }
                ret = 0;
                goto ExitFunction;
            } else if (status == PARSE_FAILURE) {
          //  printf("[VPPDLNA][%s][%d]:info->socket = %d \n",__FUNCTION__,__LINE__, info->socket );
                DLNA_LOGE("[VPPDLNA][%s][%d]:Failed To Parse The Received HTTP"
                    " Message-%s\n",__FUNCTION__,__LINE__, parser->msg.msg.buf);
                *http_error_code = parser->http_error_code;
                ret = UPNP_E_BAD_HTTPMSG;
                goto ExitFunction;
            } else if (status == PARSE_INCOMPLETE) {
           // printf("[VPPDLNA][%s][%d]:info->socket = %d \n",__FUNCTION__,__LINE__, info->socket );
           // DLNA_LOGE("[VPPDLNA][%s][%d]:status = %d \n",__FUNCTION__,__LINE__, status );
                /* need more data to continue */
                if (pbIsMsgRecvComplete)
                {
                    /* Message receive is in progress */
                    *pbIsMsgRecvComplete = UPNP_FALSE;
                  //  printf("[VPPDLNA][%s][%d]:info->socket = %d & NOT exiting \n",__FUNCTION__,__LINE__, info->socket );
                }
            } else if (status == PARSE_INCOMPLETE_ENTITY) {
           // printf("[VPPDLNA][%s][%d]:info->socket = %d & & NOT exiting1 \n",__FUNCTION__,__LINE__, info->socket );
          //  DLNA_LOGE("[VPPDLNA][%s][%d]:status = %d \n",__FUNCTION__,__LINE__, status );
                /* read until close */
                ok_on_close = TRUE;
            } else if (status == PARSE_CONTINUE_1) {
         //   printf("[VPPDLNA][%s][%d]:info->socket = %d \n",__FUNCTION__,__LINE__, info->socket );
         //   DLNA_LOGE("[VPPDLNA][%s][%d]:status = %d \n",__FUNCTION__,__LINE__, status );
                /* Web post request. */
                ret = PARSE_SUCCESS;
                goto ExitFunction;
            }
        } else if (num_read == 0) {
            if (ok_on_close) {
                //printf("[VPPDLNA][%s][%d]:info->socket = %d \n",__FUNCTION__,__LINE__, info->socket );
            //    DLNA_LOGE("[VPPDLNA][%s][%d]:status = %d \n",__FUNCTION__,__LINE__, status );
                DLNA_LOGV("[VPPDLNA]:<<< (RECVD) <<<\n%s\n-----------------\n",
                    parser->msg.msg.buf );
                print_http_headers(&parser->msg);
                ret = 0;
                goto ExitFunction;
            } else {

         //   DLNA_LOGE("[VPPDLNA][%s][%d]:status = %d \n",__FUNCTION__,__LINE__, status );
                /* partial msg */
                /*
                    This is chnaged for persistent connection handling.
                */
              //  printf("[VPPDLNA][%s][%d]:info->socket = %d \n",__FUNCTION__,__LINE__, info->socket );
                DLNA_LOGV("[VPPDLNA][%s][%d]:The HTTP Message is being received"
                    " Partially\n",__FUNCTION__,__LINE__);
                *http_error_code = 0;    /* or response */
                ret = UPNP_E_BAD_HTTPMSG;
                goto ExitFunction;
            }
        }
        else if(num_read == UPNP_E_TIMEDOUT)
        {
            DLNA_LOGV("[VPPDLNA][%s][%d]:Select Timeout",__FUNCTION__,__LINE__);
            ret = num_read;
            goto ExitFunction;

        }
        else {
      //  printf("[VPPDLNA][%s][%d]:info->socket = %d \n",__FUNCTION__,__LINE__, info->socket );
      //  DLNA_LOGE("[VPPDLNA][%s][%d]:status = %d \n",__FUNCTION__,__LINE__, status );
            *http_error_code = parser->http_error_code;
            ret = num_read;
            goto ExitFunction;
        }
    }

ExitFunction:
    if (ret != UPNP_E_SUCCESS && *http_error_code != 0) {

        DLNA_LOGV("[VPPDLNA][%s][%d]: Error %d, http_error_code = %d.\n",
            __FUNCTION__,__LINE__,ret,*http_error_code);
    }
    //printf("[VPPDLNA][%s][%d]:info->socket = %d & FINALLY leaving\n",__FUNCTION__,__LINE__, info->socket );
    return ret;

}

int http_SendMessage(SOCKINFO *info, int *TimeOut, const char *fmt, ...)
{
    FILE *Fp;
    va_list argp;
    struct SendInstruction *Instr = NULL;
    char *buf = NULL;
    char *filename = NULL;
    char *file_buf = NULL;
    char *ChunkBuf = NULL;
    char Chunk_Header[CHUNK_HEADER_SIZE];
    char c;
    int nw;
    int RetVal = 0;
    int dtcpFlag = 0;// for dtcp
    size_t dtcpLen = 0;// for dtcp
    size_t buf_length;
    size_t num_read;
    size_t num_written = 0;
    off64_t amount_to_be_read = 0;
    /* 10 byte allocated for chunk header. */
    off64_t Data_Buf_Size = WEB_SERVER_BUF_SIZE;

    va_start(argp, fmt);
    while ((c = *fmt++) != 0) {
        if (c == 'I') {
            Instr = va_arg(argp, struct SendInstruction *);
            if (Instr->ReadSendSize >= 0)
                amount_to_be_read = Instr->ReadSendSize;
            else
                amount_to_be_read = Data_Buf_Size;
            if (amount_to_be_read < WEB_SERVER_BUF_SIZE)
            {
                // for dtcp
                //Data_Buf_Size = amount_to_be_read;
                Data_Buf_Size = WEB_SERVER_BUF_SIZE;
            }
            ChunkBuf = UPNP_MALLOC((size_t)
                (Data_Buf_Size + CHUNK_HEADER_SIZE +
                CHUNK_TAIL_SIZE));
            if (!ChunkBuf) {
                DLNA_LOGE("[VPPDLNA][%s][%d]: Memory Allocation Failed\n",
                        __FUNCTION__,__LINE__);
                RetVal = UPNP_E_OUTOF_MEMORY;
                goto ExitFunction;
            }
            file_buf = ChunkBuf + CHUNK_HEADER_SIZE;
        } else if (c == 'f') {
            /* file name */
            filename = va_arg(argp, char *);
            if (Instr && Instr->IsVirtualFile)
                Fp = (virtualDirCallback.open)(filename, UPNP_READ);
            else
                Fp = fopen(filename, "rb");

            if (Fp == NULL) {
                DLNA_LOGE("[VPPDLNA][%s][%d]: Failed To Open The File-%s\n",
                        __FUNCTION__,__LINE__,filename);
                RetVal = UPNP_E_FILE_READ_ERROR;
                goto ExitFunction;
            }
            if (Instr && Instr->IsRangeActive && Instr->IsVirtualFile) {
                if (virtualDirCallback.seek(Fp, Instr->RangeOffset,
                    SEEK_CUR) == -1) { /* Defect fix - DTS2011112200234 */
                    DLNA_LOGE("[VPPDLNA][%s][%d]: Seek Operaion Failed\n",
                        __FUNCTION__,__LINE__);
                    RetVal = UPNP_E_FILE_READ_ERROR;
                    goto ExitFunction;
                }
            } else if (Instr && Instr->IsRangeActive) {
                if (fseeko(Fp, Instr->RangeOffset, SEEK_CUR) != 0) {
                    DLNA_LOGE("[VPPDLNA][%s][%d]: fseeko Operaion Failed\n",
                        __FUNCTION__,__LINE__);
                    RetVal = UPNP_E_FILE_READ_ERROR;
                    goto ExitFunction;
                }
            }
            while (amount_to_be_read) {
                assert(file_buf != NULL); /*Lint fix - v70917*/
                if (Instr) {
                    int nr;
                    size_t n = amount_to_be_read >= Data_Buf_Size ?
                            Data_Buf_Size : amount_to_be_read;
                    memset(file_buf,0,WEB_SERVER_BUF_SIZE);
                    if (Instr->IsVirtualFile) {
                        nr = virtualDirCallback.read(Fp, file_buf, &n);
                        num_read = (size_t)nr;
                        dtcpFlag = 1;  // need to change
                        dtcpLen = n;
                    } else {
                        num_read = fread(file_buf, 1, n, Fp);
                    }
                    amount_to_be_read -= num_read;
                    if (Instr->ReadSendSize < 0) {
                        /* read until close */
                        amount_to_be_read = Data_Buf_Size;
                    }
                } else {
                    num_read = fread(file_buf, 1, Data_Buf_Size, Fp);
                }
                if (num_read == 0) {
                    /* EOF so no more to send. */
                    if (Instr && Instr->IsChunkActive) {
                        const char *str = "0\r\n\r\n";
                        nw = sock_write(info, str,
                                   strlen(str),
                                   TimeOut);
                    } else {
                        DLNA_LOGE("[VPPDLNA][%s][%d]: Read Operaion Failed\n",
                        __FUNCTION__,__LINE__);
                        RetVal = UPNP_E_FILE_READ_ERROR;
                    }
                    goto Cleanup_File;
                }
                /* Create chunk for the current buffer. */
                if (Instr && Instr->IsChunkActive) {
                    /* Copy CRLF at the end of the chunk */
                    assert(file_buf != NULL); /*Lint fix - v70917*/
                    memcpy(file_buf + num_read, "\r\n", 2);
                    /* Hex length for the chunk size. */
                    memset(Chunk_Header,0,sizeof(Chunk_Header));
                    snprintf(Chunk_Header, sizeof(Chunk_Header),"%zx", num_read);
                    /*itoa(num_read,Chunk_Header,16);  */
                    HISTRCAT(Chunk_Header, "\r\n");
                    /* Copy the chunk size header  */
                    memcpy(file_buf - strlen(Chunk_Header),
                           Chunk_Header,
                           strlen(Chunk_Header));
                    /* on the top of the buffer. */
                    /*file_buf[num_read+strlen(Chunk_Header)] = NULL; */
                    /*log_info("Sending %s\n",file_buf-strlen(Chunk_Header)); */
                    nw = sock_write(info,
                        file_buf - strlen(Chunk_Header),
                        num_read + strlen(Chunk_Header) + 2,
                        TimeOut);
                    num_written = (size_t)nw;
                    if (nw <= 0 || num_written != num_read + strlen(Chunk_Header) + 2)
                        /* Send error nothing we can do. */
                        goto Cleanup_File;
                } else {
                    /* write data */
                    if(dtcpFlag == 0)
                    {
                        nw = sock_write(info, file_buf, num_read, TimeOut);
                        num_written = (size_t)nw;

                        /*  MODIFIED by s00902670 & k00900440
                        DLNA_LOGI("[VPPDLNA]:>>> (SENT) >>>\n%.*s\n------------\n",
                               nw, file_buf); */
                        DLNA_LOGV("[VPPDLNA]:>>> (SENT) >>>\n%.*s\n buflen=%d num_write=%d\n",
                            ( int )num_written, file_buf, num_read,  num_written);

                        /* Send error nothing we can do */
                        num_written = (size_t)nw;
                        if (nw <= 0 || num_written != num_read) {
                            goto Cleanup_File;
                        }
                    }
                    else
                    {
                        nw = sock_write(info, file_buf, dtcpLen, TimeOut);
                        num_written = (size_t)nw;
                        if (nw <= 0 || num_written != dtcpLen) {
                        goto Cleanup_File;
                        }
                    }
                }
            } /* while */

            if (amount_to_be_read == 0) {
                /* EOF so no more to send. */
                if (Instr && Instr->IsChunkActive) {
                    const char *str = "0\r\n\r\n";
                    nw = sock_write(info, str,
                               strlen(str),
                               TimeOut);
                }
            }

Cleanup_File:
            if (Instr && Instr->IsVirtualFile) {
                (void)virtualDirCallback.close(Fp);
            } else {
                fclose(Fp);
            }
            goto ExitFunction;
        } else if (c == 'b') {
            /* memory buffer */
            buf = va_arg(argp, char *);
            buf_length = va_arg(argp, size_t);
            if (buf_length > 0) {
                nw = sock_write(info, buf, buf_length, TimeOut);
                num_written = (size_t)nw;
                DLNA_LOGV("[VPPDLNA]:>>> (SENT) >>>\n"
                       "%.*s\nbuf_length=%zd, num_written=%zd\n""------------\n",
                       (int)buf_length, buf, buf_length, num_written);
                if (num_written != buf_length) {
                    RetVal = 0;
                    goto ExitFunction;
                }
            }
        }
    }

ExitFunction:
    va_end(argp);
    UPNP_FREE(ChunkBuf);
    return RetVal;
}


/************************************************************************
 * Function: http_RequestAndResponse
 *
 * Parameters:
 *    IN uri_type* destination;    Destination URI object which contains
 *                    remote IP address among other elements
 *    IN const char* request;        Request to be sent
 *    IN size_t request_length;    Length of the request
 *    IN http_method_t req_method;    HTTP Request method
 *    IN int timeout_secs;        time out value
 *    OUT http_parser_t* response;    Parser object to receive the repsonse
 *
 * Description:
 *    Initiates socket, connects to the destination, sends a
 *    request and waits for the response from the remote end
 *
 * Returns:
 *    UPNP_E_SOCKET_ERROR
 *     UPNP_E_SOCKET_CONNECT
 *    Error Codes returned by http_SendMessage
 *    Error Codes returned by http_RecvMessage
 ************************************************************************/
int http_RequestAndResponse(
    IN uri_type *destination,
    IN const char *request,
    IN size_t request_length,
    IN http_method_t req_method,
    IN int timeout_secs,
    OUT http_parser_t *response)
{
    SOCKET tcp_connection;
    int ret_code;
    size_t sockaddr_len;
    int http_error_code;
    SOCKINFO info;

    tcp_connection = socket(
        destination->hostport.IPaddress.ss_family, SOCK_STREAM, 0);
    if (tcp_connection == -1) {
        parser_response_init(response, req_method);
        return UPNP_E_SOCKET_ERROR;
    }
    if (sock_init(&info, tcp_connection) != UPNP_E_SUCCESS) {
        parser_response_init(response, req_method);
        ret_code = UPNP_E_SOCKET_ERROR;
        goto end_function;
    }
    /* connect */
    sockaddr_len = destination->hostport.IPaddress.ss_family == AF_INET6 ?
        sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
    ret_code = private_connect(info.socket,
        (struct sockaddr *)&(destination->hostport.IPaddress),
        (socklen_t)sockaddr_len, 0, 1);
    if (ret_code == -1) {
        parser_response_init(response, req_method);
        ret_code = UPNP_E_SOCKET_CONNECT;
        goto end_function;
    }
    /* send request */
    ret_code = http_SendMessage(&info, &timeout_secs, "b",
        request, request_length);
    if (ret_code != 0) {
        parser_response_init(response, req_method);
        goto end_function;
    }

    /* recv response
       r72104-: To support the DLNA Guide Line
       "UPnP CP Must understands the class of a status code, even if the
       specific code is not understood. In this case it should treat
       the status code as a x00 status."
    */

    do{
        ret_code = http_RecvMessage(&info, response, req_method,
                &timeout_secs, &http_error_code, NULL);
        if (UPNP_HTTP_1xx_RESPONSE(response->msg.status_code)){
            httpmsg_destroy(&(response->msg));
        }
        else
        {
            break;
        }
    }while(1);


    if (UPNP_HTTP_2xx_RESPONSE(response->msg.status_code)){
        response->msg.status_code = HTTP_OK;
    }

end_function:
    /* should shutdown completely */
    (void)sock_destroy(&info, SD_BOTH);

    return ret_code;
}


/************************************************************************
 * Function: http_Download
 *
 * Parameters:
 *    IN const char* url_str;    String as a URL
 *    IN int timeout_secs;    time out value
 *    OUT char** document;    buffer to store the document extracted
 *                from the donloaded message.
 *    OUT int* doc_length;    length of the extracted document
 *    OUT char* content_type;    Type of content
 *
 * Description:
 *    Download the document message and extract the document
 *    from the message.
 *
 * Return: int
 *    UPNP_E_SUCCESS
 *    UPNP_E_INVALID_URL
 ************************************************************************/
int http_Download( IN const char *url_str,
               IN int timeout_secs,
               OUT char **document,
               OUT size_t *doc_length,
               OUT char *content_type )
{
    int ret_code;
    uri_type url;
    char *msg_start;
    char *entity_start;
    char *hoststr;
    char *temp;
    http_parser_t response;
    size_t msg_length;
    size_t hostlen;
    memptr ctype;
    size_t copy_len;
    membuffer request;
    char *urlPath = alloca(strlen(url_str) + 1);
        if (NULL == urlPath)
        {
            return UPNP_E_OUTOF_MEMORY;
        }

    /*ret_code = parse_uri( (char*)url_str, strlen(url_str), &url ); */
    DLNA_LOGI("[VPPDLNA]:DOWNLOAD URL : %s\n", url_str);
    ret_code = http_FixStrUrl((char *)url_str, strlen(url_str), &url);
    if (ret_code != UPNP_E_SUCCESS)
        return ret_code;
    /* make msg */
    membuffer_init(&request);
    HISTRCPY(urlPath, url_str);
    hoststr = strstr(urlPath, "//");
    if (hoststr == NULL)
        return UPNP_E_INVALID_URL;
    hoststr += 2;
    temp = strchr(hoststr, '/');
    if (temp == NULL)
        return UPNP_E_INVALID_URL;
    *temp = '\0';
    hostlen = strlen(hoststr);
    *temp = '/';
    DLNA_LOGI("[VPPDLNA]:HOSTNAME : %s Length : %" PRIzu "\n", hoststr, hostlen);
    ret_code = http_MakeMessage(&request, 1, 1,
                    "Q" "s" "bcDCUc",
                    HTTPMETHOD_GET, url.pathquery.buff,
                    url.pathquery.size, "HOST: ", hoststr,
                    hostlen);
    if (ret_code != 0) {
        DLNA_LOGE("[VPPDLNA]:HTTP Makemessage failed\n");
        membuffer_destroy(&request);
        return ret_code;
    }
    DLNA_LOGV("[VPPDLNA]:HTTP Buffer:\n%s\n" "----------END--------\n", request.buf);
    /* get doc msg */
    ret_code =
        http_RequestAndResponse(&url, request.buf, request.length,
                    HTTPMETHOD_GET, timeout_secs, &response);

    if (ret_code != 0) {
        httpmsg_destroy(&response.msg);
        membuffer_destroy(&request);
        return ret_code;
    }
    DLNA_LOGI("[VPPDLNA]:Response\n");
    print_http_headers(&response.msg);
    /* optional content-type */
    if (content_type) {
        if (httpmsg_find_hdr(&response.msg, HDR_CONTENT_TYPE, &ctype) ==
            NULL) {
            *content_type = '\0';    /* no content-type */
        } else {
            /* safety */
            copy_len = ctype.length < LINE_SIZE - 1 ?
                ctype.length : LINE_SIZE - 1;

            memcpy(content_type, ctype.buf, copy_len);
            content_type[copy_len] = '\0';
        }
    }
    /* extract doc from msg */
    if ((*doc_length = response.msg.entity.length) == 0) {
        /* 0-length msg */
        *document = NULL;
    } else if (response.msg.status_code == HTTP_OK) {
        /*LEAK_FIX_MK */
        /* copy entity */
        entity_start = response.msg.entity.buf;    /* what we want */
        msg_length = response.msg.msg.length;    /* save for posterity    */
        msg_start = membuffer_detach(&response.msg.msg);    /* whole msg */
        /* move entity to the start; copy null-terminator too */
        memmove(msg_start, entity_start, *doc_length + 1);
        /* save mem for body only */
        *document = UPNP_REALLOC(msg_start, *doc_length + 1);    /*LEAK_FIX_MK */
        /* *document = Realloc( msg_start,msg_length, *doc_length + 1 ); LEAK_FIX_MK */
        /* shrink can't fail */
        assert(msg_length > *doc_length);
        assert(*document != NULL);
    }
    if (response.msg.status_code == HTTP_OK) {
        ret_code = 0;    /* success */
    } else {
        /* server sent error msg (not requested doc) */
        ret_code = response.msg.status_code;
    }
    httpmsg_destroy(&response.msg);
    membuffer_destroy(&request);

    return ret_code;
}

typedef struct HTTPPOSTHANDLE {
    SOCKINFO sock_info;
    int contentLength;
} http_post_handle_t;

/************************************************************************
 * Function: MakePostMessage
 *
 * Parameters:
 *    const char *url_str;        String as a URL
 *    membuffer *request;        Buffer containing the request
 *    uri_type *url;             URI object containing the scheme,
 *                    path query token, etc.
 *    int contentLength;        length of content
 *    const char *contentType;    Type of content
 *
 * Description:
 *    Makes the message for the HTTP POST message
 *
 * Returns:
 *    UPNP_E_INVALID_URL
 *     UPNP_E_INVALID_PARAM
 *    UPNP_E_SUCCESS
 ************************************************************************/
int MakePostMessage(const char *url_str, membuffer *request,
    uri_type *url, int contentLength, const char *contentType)
{
    int ret_code = 0;
    char *urlPath;
    size_t hostlen = 0;
    char *hoststr;
    char *temp;

    DLNA_LOGI("[VPPDLNA]:DOWNLOAD URL : %s\n", url_str);
    ret_code = http_FixStrUrl((char *)url_str, strlen(url_str), url);
    if (ret_code != UPNP_E_SUCCESS)
        return ret_code;
    /* make msg */
    membuffer_init(request);
        urlPath = alloca(strlen(url_str) + 1);
        if (NULL == urlPath)
        {
                return UPNP_E_OUTOF_MEMORY;
        }

    HISTRCPY(urlPath, url_str);
    hoststr = strstr(urlPath, "//");
    if (hoststr == NULL)
        return UPNP_E_INVALID_URL;
    hoststr += 2;
    temp = strchr(hoststr, '/');
    if (temp == NULL)
        return UPNP_E_INVALID_URL;
    *temp = '\0';
    hostlen = strlen(hoststr);
    *temp = '/';
    DLNA_LOGI("[VPPDLNA]:HOSTNAME : %s Length : %" PRIzu "\n", hoststr, hostlen);
    if (contentLength >= 0)
        ret_code = http_MakeMessage(request, 1, 1,
                        "Q" "s" "bcDCU" "T" "Nc",
                        HTTPMETHOD_POST,
                        url->pathquery.buff,
                        url->pathquery.size, "HOST: ",
                        hoststr, hostlen, contentType,
                        /*  MODIFIED by c00190074
                        (off_t) contentLength); */
                        (off64_t) contentLength);
    else if (contentLength == UPNP_USING_CHUNKED)
        ret_code = http_MakeMessage(request, 1, 1,
                        "Q" "s" "bcDCU" "TKc",
                        HTTPMETHOD_POST,
                        url->pathquery.buff,
                        url->pathquery.size, "HOST: ",
                        hoststr, hostlen, contentType);
    else if (contentLength == UPNP_UNTIL_CLOSE)
        ret_code = http_MakeMessage(request, 1, 1,
                        "Q" "s" "bcDCU" "Tc",
                        HTTPMETHOD_POST,
                        url->pathquery.buff,
                        url->pathquery.size, "HOST: ",
                        hoststr, hostlen, contentType);
    else
        ret_code = UPNP_E_INVALID_PARAM;
    if (ret_code != 0) {
        DLNA_LOGE("[VPPDLNA]:HTTP Makemessage failed\n");
        membuffer_destroy(request);
        return ret_code;
    }
    DLNA_LOGV("[VPPDLNA]:HTTP Buffer:\n%s\n" "----------END--------\n",
           request->buf);

    return UPNP_E_SUCCESS;
}

/************************************************************************
 * Function: http_WriteHttpPost
 *
 * Parameters:
 *    IN void *Handle:    Handle to the http post object
 *    IN char *buf:        Buffer to send to peer, if format used
 *                is not UPNP_USING_CHUNKED,
 *    IN unsigned int *size:    Size of the data to be sent.
 *    IN int timeout:        time out value
 *
 * Description:
 *    Formats data if format used is UPNP_USING_CHUNKED.
 *    Writes data on the socket connected to the peer.
 *
 * Return: int
 *    UPNP_E_SUCCESS - On Success
 *    UPNP_E_INVALID_PARAM - Invalid Parameter
 *    -1 - On Socket Error.
 ************************************************************************/
int http_WriteHttpPost( IN void *Handle,
                    IN char *buf,
                    IN size_t *size,
                    IN int timeout )
{
    http_post_handle_t *handle = (http_post_handle_t *)Handle;
    char *tempbuf = NULL;
    size_t tempbufSize = 0;
    int freeTempbuf = 0;
    int numWritten = 0;

    if (!handle || !size || !buf) {
        if (size)
            *size = 0;
        return UPNP_E_INVALID_PARAM;
    }
    if (handle->contentLength == UPNP_USING_CHUNKED) {
        if (*size) {
            size_t tempSize = *size +CHUNK_HEADER_SIZE + CHUNK_TAIL_SIZE;
            tempbuf = UPNP_MALLOC(tempSize);
            if (!tempbuf)
                return UPNP_E_OUTOF_MEMORY;
            /* begin chunk */
            memset(tempbuf,0,tempSize);
            snprintf(tempbuf, tempSize,"%zx\r\n", *size);
            tempSize = strlen(tempbuf);
            memcpy(tempbuf + tempSize, buf, *size);
            memcpy(tempbuf + tempSize + *size, "\r\n", 2);
            /* end of chunk */
            tempbufSize = tempSize + *size + 2;
            freeTempbuf = 1;
        }
    } else {
        tempbuf = buf;
        tempbufSize = *size;
    }
    numWritten =
        sock_write(&handle->sock_info, tempbuf, tempbufSize, &timeout);
    if (freeTempbuf)
        UPNP_FREE(tempbuf);
    if (numWritten < 0) {
        *size = 0;
        return numWritten;
    } else {
        *size = (size_t)numWritten;
        return UPNP_E_SUCCESS;
    }
}

/************************************************************************
 * Function: http_CloseHttpPost
 *
 * Parameters:
 *    IN void *Handle;    Handle to the http post object
 *    IN OUT int *httpStatus;    HTTP status returned on receiving a
 *                response message
 *    IN int timeout;        time out value
 *
 * Description:
 *    Sends remaining data if using  UPNP_USING_CHUNKED
 *    format. Receives any more messages. Destroys socket and any socket
 *    associated memory. Frees handle associated with the HTTP POST msg.
 *
 * Return: int
 *    UPNP_E_SUCCESS        - On success
 *    UPNP_E_INVALID_PARAM    - Invalid Parameter
 ************************************************************************/
int http_CloseHttpPost(IN void *Handle, IN OUT int *httpStatus, IN int timeout)
{
    int retc = 0;
    http_parser_t response;
    int http_error_code;
    const char *zcrlf = "0\r\n\r\n";

    http_post_handle_t *handle = Handle;
    if ((!handle) || (!httpStatus))
        return UPNP_E_INVALID_PARAM;
    if (handle->contentLength == UPNP_USING_CHUNKED)
        /*send last chunk */
        retc = sock_write(&handle->sock_info, zcrlf, strlen(zcrlf), &timeout);
    /*read response */
    parser_response_init(&response, HTTPMETHOD_POST);
    retc = http_RecvMessage(&handle->sock_info, &response,
        HTTPMETHOD_POST, &timeout, &http_error_code, NULL);
    *httpStatus = http_error_code;
    /*should shutdown completely */
    (void)sock_destroy(&handle->sock_info, SD_BOTH);
    httpmsg_destroy(&response.msg);
    UPNP_FREE(handle);

    return retc;
}

/************************************************************************
 * Function: http_OpenHttpPost
 *
 * Parameters:
 *    IN const char *url_str;        String as a URL
 *    IN OUT void **Handle;        Pointer to buffer to store HTTP
 *                    post handle
 *    IN const char *contentType;    Type of content
 *    IN int contentLength;        length of content
 *    IN int timeout;            time out value
 *
 * Description:
 *    Makes the HTTP POST message, connects to the peer,
 *    sends the HTTP POST request. Adds the post handle to buffer of
 *    such handles
 *
 * Return : int;
 *    UPNP_E_SUCCESS        - On success
 *    UPNP_E_INVALID_PARAM    - Invalid Parameter
 *    UPNP_E_OUTOF_MEMORY
 *    UPNP_E_SOCKET_ERROR
 *    UPNP_E_SOCKET_CONNECT
 ************************************************************************/
int http_OpenHttpPost(
    IN const char *url_str,
    IN OUT void **Handle,
    IN const char *contentType,
    IN int contentLength,
    IN int timeout)
{
    int ret_code;
    size_t sockaddr_len;
    SOCKET tcp_connection;
    membuffer request;
    http_post_handle_t *handle = NULL;
    uri_type url;

    if (!url_str || !Handle || !contentType)
        return UPNP_E_INVALID_PARAM;
    *Handle = handle;
    ret_code = MakePostMessage(url_str, &request, &url,
                   contentLength, contentType);
    if (ret_code != UPNP_E_SUCCESS)
        return ret_code;
    handle = UPNP_MALLOC(sizeof(http_post_handle_t));
    if (!handle)
        return UPNP_E_OUTOF_MEMORY;
    handle->contentLength = contentLength;
    tcp_connection = socket(url.hostport.IPaddress.ss_family,
        SOCK_STREAM, 0);
    if (tcp_connection == -1) {
        ret_code = UPNP_E_SOCKET_ERROR;
        goto errorHandler;
    }
    if (sock_init(&handle->sock_info, tcp_connection) != UPNP_E_SUCCESS) {
        (void)sock_destroy(&handle->sock_info, SD_BOTH);
        ret_code = UPNP_E_SOCKET_ERROR;
        goto errorHandler;
    }
    sockaddr_len = url.hostport.IPaddress.ss_family == AF_INET6 ?
        sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
    ret_code = private_connect(handle->sock_info.socket,
        (struct sockaddr *)&(url.hostport.IPaddress),
        (socklen_t)sockaddr_len, -1, -1);
    if (ret_code == -1) {
        (void)sock_destroy(&handle->sock_info, SD_BOTH);
        ret_code = UPNP_E_SOCKET_CONNECT;
        goto errorHandler;
    }
    /* send request */
    ret_code = http_SendMessage(&handle->sock_info, &timeout, "b",
                    request.buf, request.length);
    if (ret_code != 0)
        (void)sock_destroy(&handle->sock_info, SD_BOTH);

 errorHandler:
    membuffer_destroy(&request);
    *Handle = handle;

    return ret_code;
}

typedef struct HTTPGETHANDLE {
    http_parser_t response;
    SOCKINFO sock_info;
    int entity_offset;
    int cancel;
} http_get_handle_t;

/************************************************************************
* Function: MakeGetMessage
*
* Parameters:
*    const char *url_str ;    String as a URL
*    const char *proxy_str ;    String as a URL of proxy to use
*    membuffer *request ;    Buffer containing the request
*    uri_type *url ;     URI object containing the scheme, path
*                query token, etc.
*
* Description:
*    Makes the message for the HTTP GET method
*
* Returns:
*    UPNP_E_INVALID_URL
*     Error Codes returned by http_MakeMessage
*    UPNP_E_SUCCESS
************************************************************************/
int MakeGetMessage(const char *url_str, const char *proxy_str,
    membuffer *request, uri_type *url)
{
    int ret_code;
    char *urlPath = NULL;
    size_t querylen = 0;
    const char *querystr;
    size_t hostlen = 0;
    char *hoststr, *temp;

    DLNA_LOGI("[VPPDLNA]:DOWNLOAD URL : %s\n", url_str);
    urlPath = alloca(strlen(url_str) + 1);
        if (NULL == urlPath)
        {
            return UPNP_E_OUTOF_MEMORY;
        }

    ret_code = http_FixStrUrl((char *)url_str, strlen(url_str), url);
    if (ret_code != UPNP_E_SUCCESS)
        return ret_code;
    /* make msg */
    membuffer_init(request);
    HISTRCPY(urlPath, url_str);
    hoststr = strstr(urlPath, "//");
    if (hoststr == NULL)
        return UPNP_E_INVALID_URL;
    hoststr += 2;
    temp = strchr(hoststr, '/');
    if (temp == NULL)
        return UPNP_E_INVALID_URL;
    *temp = '\0';
    hostlen = strlen(hoststr);
    *temp = '/';
    DLNA_LOGI("[VPPDLNA]:HOSTNAME : %s Length : %" PRIzu "\n", hoststr, hostlen);
    if (proxy_str) {
        querystr = url_str;
        querylen = strlen(querystr);
    } else {
        querystr = url->pathquery.buff;
        querylen = url->pathquery.size;
    }
    ret_code = http_MakeMessage(request, 1, 1,
                    "Q" "s" "bcDCUc",
                    HTTPMETHOD_GET, querystr, querylen,
                    "HOST: ", hoststr, hostlen);
    if (ret_code != 0) {
        DLNA_LOGE("[VPPDLNA]:HTTP Makemessage failed\n");
        membuffer_destroy(request);

        return ret_code;
    }
    DLNA_LOGV("[VPPDLNA]:HTTP Buffer:\n%s\n" "----------END--------\n",
           request->buf);

    return UPNP_E_SUCCESS;
}

/*!
 * \brief Parses already exiting data. If not complete reads more
 * data on the connected socket. The read data is then parsed. The
 * same methid is carried out for headers.
 *
 * \return integer:
 *    \li \c PARSE_OK - On Success
 *    \li \c PARSE_FAILURE - Failure to parse data correctly
 *    \li \c UPNP_E_BAD_HTTPMSG - Socker read() returns an error
 */
static int ReadResponseLineAndHeaders(
    /*! Socket information object. */
    IN SOCKINFO *info,
    /*! HTTP Parser object. */
    IN OUT http_parser_t *parser,
    /*! Time out value. */
    IN OUT int *timeout_secs,
    /*! HTTP errror code returned. */
    IN OUT int *http_error_code)
{
    parse_status_t status;
    int num_read;
    char buf[2 * 1024];
    int done = 0;
    int ret_code = 0;

    /*read response line */
    status = parser_parse_responseline(parser);
    if (status == PARSE_OK)
        done = 1;
    else if (status == PARSE_INCOMPLETE)
        done = 0;
    else
        /*error */
        return status;
    while (!done) {
        num_read = sock_read(info, buf, sizeof(buf), timeout_secs);
        if (num_read > 0) {
            /* append data to buffer */
            ret_code =
                membuffer_append(&parser->msg.msg, buf, (size_t)num_read);
            if (ret_code != 0) {
                /* set failure status */
                parser->http_error_code =
                    HTTP_INTERNAL_SERVER_ERROR;
                return PARSE_FAILURE;
            }
            status = parser_parse_responseline(parser);
            if (status == PARSE_OK) {
                done = 1;
            } else if (status == PARSE_INCOMPLETE) {
                done = 0;
            } else {
                /*error */
                return status;
            }
        } else if (num_read == 0) {
            /* partial msg */
            *http_error_code = HTTP_BAD_REQUEST;    /* or response */
            return UPNP_E_BAD_HTTPMSG;
        } else {
            *http_error_code = parser->http_error_code;
            return num_read;
        }
    }
    done = 0;
    status = parser_parse_headers(parser);
    if ((status == PARSE_OK) && (parser->position == POS_ENTITY))
        done = 1;
    else if (status == PARSE_INCOMPLETE)
        done = 0;
    else
        /*error */
        return status;
    /*read headers */
    while (!done) {
        num_read = sock_read(info, buf, sizeof(buf), timeout_secs);
        if (num_read > 0) {
            /* append data to buffer */
            ret_code =
                membuffer_append(&parser->msg.msg, buf, (size_t)num_read);
            if (ret_code != 0) {
                /* set failure status */
                parser->http_error_code = HTTP_INTERNAL_SERVER_ERROR;
                return PARSE_FAILURE;
            }
            status = parser_parse_headers(parser);
            if (status == PARSE_OK && parser->position == POS_ENTITY)
                done = 1;
            else if (status == PARSE_INCOMPLETE)
                done = 0;
            else
                /*error */
                return status;
        } else if (num_read == 0) {
            /* partial msg */
            *http_error_code = HTTP_BAD_REQUEST;    /* or response */
            return UPNP_E_BAD_HTTPMSG;
        } else {
            *http_error_code = parser->http_error_code;
            return num_read;
        }
    }

    return PARSE_OK;
}

/************************************************************************
 * Function: http_ReadHttpGet
 *
 * Parameters:
 *    IN void *Handle;    Handle to the HTTP get object
 *    IN OUT char *buf;    Buffer to get the read and parsed data
 *    IN OUT size_t *size;    Size of the buffer passed
 *    IN int timeout;        time out value
 *
 * Description:
 *    Parses already existing data, then gets new data.
 *    Parses and extracts information from the new data.
 *
 * Return: int
 *    UPNP_E_SUCCESS        - On success
 *    UPNP_E_INVALID_PARAM    - Invalid Parameter
 *    UPNP_E_BAD_RESPONSE
 *    UPNP_E_BAD_HTTPMSG
 *    UPNP_E_CANCELED
 ************************************************************************/
int http_ReadHttpGet(
    IN void *Handle,
    IN OUT char *buf,
    IN OUT size_t *size,
    IN int timeout)
{
    http_get_handle_t *handle = Handle;
    parse_status_t status;
    int num_read;
    int ok_on_close = FALSE;
    char tempbuf[2 * 1024];
    int ret_code = 0;

    if (!handle || !size || (*size > 0 && !buf)) {
        if (size)
            *size = 0;
        return UPNP_E_INVALID_PARAM;
    }
    /* first parse what has already been gotten */
    if (handle->response.position != POS_COMPLETE)
        status = parser_parse_entity(&handle->response);
    else
        status = PARSE_SUCCESS;
    if (status == PARSE_INCOMPLETE_ENTITY)
        /* read until close */
        ok_on_close = TRUE;
    else if ((status != PARSE_SUCCESS)
           && (status != PARSE_CONTINUE_1)
           && (status != PARSE_INCOMPLETE)) {
        /*error */
        *size = 0;
        return UPNP_E_BAD_RESPONSE;
    }
    /* read more if necessary entity */
    while (handle->response.msg.amount_discarded + *size >
           handle->response.msg.entity.length &&
           !handle->cancel &&
           handle->response.position != POS_COMPLETE) {
        num_read = sock_read(&handle->sock_info, tempbuf,
            sizeof(tempbuf), &timeout);
        if (num_read > 0) {
            /* append data to buffer */
            ret_code = membuffer_append(&handle->response.msg.msg,
                tempbuf, (size_t)num_read);
            if (ret_code != 0) {
                /* set failure status */
                handle->response.http_error_code =
                    HTTP_INTERNAL_SERVER_ERROR;
                *size = 0;
                return PARSE_FAILURE;
            }
            status = parser_parse_entity(&handle->response);
            if (status == PARSE_INCOMPLETE_ENTITY) {
                /* read until close */
                ok_on_close = TRUE;
            } else if ((status != PARSE_SUCCESS)
                   && (status != PARSE_CONTINUE_1)
                   && (status != PARSE_INCOMPLETE)) {
                /*error */
                *size = 0;
                return UPNP_E_BAD_RESPONSE;
            }
        } else if (num_read == 0) {
            if (ok_on_close) {
                DLNA_LOGV("[VPPDLNA]:<<< (RECVD) <<<\n%s\n-----------------\n",
                       handle->response.msg.msg.buf);
                handle->response.position = POS_COMPLETE;
            } else {
                /* partial msg */
                *size = 0;
                handle->response.http_error_code = HTTP_BAD_REQUEST;    /* or response */
                return UPNP_E_BAD_HTTPMSG;
            }
        } else {
            *size = 0;
            return num_read;
        }
    }
    if (handle->cancel) {
        return UPNP_E_CANCELED;
    }
    /* truncate size to fall within available data */
    if (handle->response.msg.amount_discarded + *size >
        handle->response.msg.entity.length)
        *size = handle->response.msg.entity.length -
            handle->response.msg.amount_discarded;
    /* copy data to user buffer. delete copied data */
    if (*size > 0) {
        memcpy(buf, &handle->response.msg.msg.buf[handle->response.entity_start_position],
            *size);
        membuffer_delete(&handle->response.msg.msg,
            handle->response.entity_start_position, *size);
        /* update scanner position. needed for chunked transfers */
        handle->response.scanner.cursor -= *size;
        /* update amount discarded */
        handle->response.msg.amount_discarded += *size;
    }

    return UPNP_E_SUCCESS;
}

/************************************************************************
 * Function: http_HttpGetProgress
 *
 * Parameters:
 *    IN void *Handle;    Handle to the HTTP get object
 *    OUT size_t *length;    Buffer to get the read and parsed data
 *    OUT size_t *total;    Size of tge buffer passed
 *
 * Description:
 *    Extracts information from the Handle to the HTTP get object.
 *
 * Return: int
 *    UPNP_E_SUCCESS        - On Sucess
 *    UPNP_E_INVALID_PARAM    - Invalid Parameter
 ************************************************************************/
int http_HttpGetProgress(
    IN void *Handle,
    OUT size_t *length,
    OUT size_t *total)
{
    http_get_handle_t *handle = Handle;

    if (!handle || !length || !total) {
        return UPNP_E_INVALID_PARAM;
    }
    *length = handle->response.msg.entity.length;
    *total = handle->response.content_length;

    return UPNP_E_SUCCESS;
}

/************************************************************************
 * Function: http_CancelHttpGet
 *
 * Parameters:
 *    IN void *Handle;    Handle to HTTP get object
 *
 * Description:
 *    Set the cancel flag of the HttpGet handle
 *
 * Return: int
 *    UPNP_E_SUCCESS        - On Success
 *    UPNP_E_INVALID_PARAM    - Invalid Parameter
 ************************************************************************/
int http_CancelHttpGet(IN void *Handle)
{
    http_get_handle_t *handle = Handle;

    if (!handle)
        return UPNP_E_INVALID_PARAM;
    handle->cancel = 1;

    return UPNP_E_SUCCESS;
}


/************************************************************************
 * Function: http_CloseHttpGet
 *
 * Parameters:
 *    IN void *Handle;    Handle to HTTP get object
 *
 * Description:
 *    Clears the handle allocated for the HTTP GET operation
 *    Clears socket states and memory allocated for socket operations.
 *
 * Return: int
 *    UPNP_E_SUCCESS        - On Success
 *    UPNP_E_INVALID_PARAM    - Invalid Parameter
 ************************************************************************/
int http_CloseHttpGet(IN void *Handle)
{
    http_get_handle_t *handle = Handle;

    if (!handle)
        return UPNP_E_INVALID_PARAM;
    /*should shutdown completely */
    (void)sock_destroy(&handle->sock_info, SD_BOTH);
    httpmsg_destroy(&handle->response.msg);
    UPNP_FREE(handle);

    return UPNP_E_SUCCESS;
}

int http_OpenHttpGet( IN const char *url_str,
                  IN OUT void **Handle,
                  IN OUT char **contentType,
                  OUT int *contentLength,
                  OUT int *httpStatus,
                  IN int timeout)
{
    return http_OpenHttpGetProxy(
        url_str, NULL, Handle, contentType, contentLength, httpStatus,
        timeout);
}

int http_OpenHttpGetProxy(const char *url_str, const char *proxy_str,
    void **Handle, char **contentType, int *contentLength,
    int *httpStatus, int timeout)
{
    int ret_code;
    size_t sockaddr_len;
    int http_error_code;
    memptr ctype;
    SOCKET tcp_connection;
    membuffer request;
    http_get_handle_t *handle = NULL;
    uri_type url;
    uri_type proxy;
    uri_type *peer;
    parse_status_t status;

    if (!url_str || !Handle || !contentType || !httpStatus)
        return UPNP_E_INVALID_PARAM;
    *httpStatus = 0;
    *Handle = handle;
    *contentType = NULL;
    *contentLength = 0;
    ret_code = MakeGetMessage(url_str, proxy_str, &request, &url);
    if (ret_code != UPNP_E_SUCCESS)
        return ret_code;
    if (proxy_str) {
        ret_code =
            http_FixStrUrl((char *)proxy_str, strlen(proxy_str),
                   &proxy);
        peer = &proxy;
    } else {
        peer = &url;
    }
    handle = UPNP_MALLOC(sizeof(http_get_handle_t));
    if (!handle)
        {
            DLNA_LOGE("[VPPDLNA][%s][:%d]-:Memory Allocation Failure\r\n",
            __FUNCTION__,__LINE__);
            return UPNP_E_OUTOF_MEMORY;
        }
    handle->cancel = 0;
    parser_response_init(&handle->response, HTTPMETHOD_GET);
    tcp_connection =
        socket(peer->hostport.IPaddress.ss_family, SOCK_STREAM, 0);
    if (tcp_connection == -1) {
        ret_code = UPNP_E_SOCKET_ERROR;
        goto errorHandler;
    }
    if (sock_init(&handle->sock_info, tcp_connection) != UPNP_E_SUCCESS) {
        (void)sock_destroy(&handle->sock_info, SD_BOTH);
        ret_code = UPNP_E_SOCKET_ERROR;
        goto errorHandler;
    }
    sockaddr_len = peer->hostport.IPaddress.ss_family == AF_INET6 ?
        sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
    ret_code = private_connect(handle->sock_info.socket,
        (struct sockaddr *)&(peer->hostport.IPaddress),
        (socklen_t) sockaddr_len, -1, -1);
    if (ret_code == -1) {
        (void)sock_destroy(&handle->sock_info, SD_BOTH);
        ret_code = UPNP_E_SOCKET_CONNECT;
        goto errorHandler;
    }
    /* send request */
    ret_code = http_SendMessage(&handle->sock_info, &timeout, "b",
                    request.buf, request.length);
    if (ret_code) {
        (void)sock_destroy(&handle->sock_info, SD_BOTH);
        goto errorHandler;
    }
    status = (parse_status_t)ReadResponseLineAndHeaders(&handle->sock_info,
                        &handle->response, &timeout,
                        &http_error_code);
    if (status != PARSE_OK) {
        ret_code = UPNP_E_BAD_RESPONSE;
        goto errorHandler;
    }
    status = parser_get_entity_read_method(&handle->response);
    if (status != PARSE_CONTINUE_1 && status != PARSE_SUCCESS) {
        ret_code = UPNP_E_BAD_RESPONSE;
        goto errorHandler;
    }
    *httpStatus = handle->response.msg.status_code;
    ret_code = UPNP_E_SUCCESS;
    if (!httpmsg_find_hdr(&handle->response.msg, HDR_CONTENT_TYPE, &ctype))
        /* no content-type */
        *contentType = NULL;
    else
        *contentType = ctype.buf;
    if (handle->response.position == POS_COMPLETE)
        *contentLength = 0;
    else if (handle->response.ent_position == ENTREAD_USING_CHUNKED)
        *contentLength = UPNP_USING_CHUNKED;
    else if (handle->response.ent_position == ENTREAD_USING_CLEN)
        *contentLength = (int)handle->response.content_length;
    else if (handle->response.ent_position == ENTREAD_UNTIL_CLOSE)
        *contentLength = UPNP_UNTIL_CLOSE;

 errorHandler:
    *Handle = handle;
    membuffer_destroy(&request);
    if (ret_code != UPNP_E_SUCCESS)
        httpmsg_destroy(&handle->response.msg);
    return ret_code;
}

/************************************************************************
 * Function: http_SendStatusResponse
 *
 * Parameters:
 *    IN SOCKINFO *info;        Socket information object
 *    IN int http_status_code;    error code returned while making
 *                    or sending the response message
 *    IN int request_major_version;    request major version
 *    IN int request_minor_version;    request minor version
 *
 * Description:
 *    Generate a response message for the status query and send the
 *    status response.
 *
 * Return: int
 *    0 -- success
 *    UPNP_E_OUTOF_MEMORY
 *    UPNP_E_SOCKET_WRITE
 *    UPNP_E_TIMEDOUT
 ************************************************************************/
int http_SendStatusResponse(IN SOCKINFO *info, IN int http_status_code,
    IN int request_major_version, IN int request_minor_version)
{
    int response_major, response_minor;
    membuffer membuf;
    int ret;
    int timeout;

    http_CalcResponseVersion(request_major_version, request_minor_version,
                 &response_major, &response_minor);
    membuffer_init(&membuf);
    membuf.size_inc = 70;
    /* response start line */
    ret = http_MakeMessage(&membuf, response_major, response_minor, "RSCB",
                   http_status_code, http_status_code);
    if (ret == 0) {
        timeout = HTTP_DEFAULT_TIMEOUT;
        ret = http_SendMessage(info, &timeout, "b",
               membuf.buf, membuf.length);
    }
    membuffer_destroy(&membuf);

    return ret;
}

int http_MakeMessage(membuffer *buf, int http_major_version,
    int http_minor_version, const char *fmt, ...)
{
    char c;
    char *s = NULL;
    size_t num;

    /*  MODIFIED by c00190074
    off_t bignum; */
    off64_t bignum;
    size_t length;
    time_t *loc_time;
    time_t curr_time;
    struct tm *date;
    const char *start_str;
    const char *end_str;
    int status_code;
    const char *status_msg;
    http_method_t method;
    const char *method_str;
    const char *url_str;
    const char *temp_str;
    uri_type url;
    uri_type *uri_ptr;
    int error_code = 0;
    va_list argp;
    char tempbuf[200];
    const char *weekday_str = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";
    const char *month_str = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0"
        "Jul\0Aug\0Sep\0Oct\0Nov\0Dec";

    va_start(argp, fmt);
    memset(tempbuf,0,sizeof(tempbuf));
    while ((c = *fmt++) != 0) {
        if (c == 's') {
            /* C string */
            s = (char *)va_arg(argp, char *);
            assert(s);
            /*DLNA_LOGI("[VPPDLNA]:Adding a string : %s\n", s);*/
            if (membuffer_append(buf, s, strlen(s)))
                goto error_handler;
        } else if (c == 'K') {
            /* Add Chunky header */
            if (membuffer_append(buf, "TRANSFER-ENCODING: chunked\r\n",
                strlen("Transfer-Encoding: chunked\r\n")))
                goto error_handler;
        } else if (c == 'G') {
            /* Add Range header */
            struct SendInstruction *RespInstr;
            RespInstr = (struct SendInstruction *)
                va_arg(argp, struct SendInstruction *);
            assert(RespInstr);
            /* connection header */
            if (membuffer_append(buf, RespInstr->RangeHeader,
                strlen(RespInstr->RangeHeader)))
                goto error_handler;
        } else if (c == 'b') {
            /* mem buffer */
            s = (char *)va_arg(argp, char *);
            /*DLNA_LOGI("[VPPDLNA]:Adding a char Buffer starting with: %c\n", s[0]);*/
            assert(s);
            length = (size_t) va_arg(argp, size_t);
            if (membuffer_append(buf, s, length))
                goto error_handler;
        } else if (c == 'c') {
            /* crlf */
            if (membuffer_append(buf, "\r\n", 2))
                goto error_handler;
        } else if (c == 'd') {
            /* integer */
            num = (size_t)va_arg(argp, int);
            snprintf(tempbuf, sizeof(tempbuf),"%" PRIzu, num);
            if (membuffer_append(buf, tempbuf, strlen(tempbuf)))
                goto error_handler;
        } else if (c == 'h') {
            /* off_t */
            /*  MODIFIED by c00190074
            bignum = (off_t) va_arg(argp, off_t); */
            bignum = (off64_t) va_arg(argp, off64_t);
            snprintf(tempbuf, sizeof(tempbuf),"%" PRId64, (int64_t) bignum);
            if (membuffer_append(buf, tempbuf, strlen(tempbuf)))
                goto error_handler;
        } else if (c == 't' || c == 'D') {
            /* date */
            if (c == 'D') {
                /* header */
                start_str = "DATE: ";
                end_str = "\r\n";
                curr_time = time(NULL);
                loc_time = &curr_time;
            } else {
                /* date value only */
                start_str = end_str = "";
                loc_time = (time_t *)va_arg(argp, time_t *);
            }
            assert(loc_time);
            date = gmtime(loc_time);
            snprintf(tempbuf, sizeof(tempbuf),
                "%s%s, %02d %s %d %02d:%02d:%02d GMT%s",
                start_str, &weekday_str[date->tm_wday * 4],
                date->tm_mday, &month_str[date->tm_mon * 4],
                date->tm_year + 1900, date->tm_hour,
                date->tm_min, date->tm_sec, end_str);
            if (membuffer_append(buf, tempbuf, strlen(tempbuf)))
                goto error_handler;
        } else if (c == 'L') {
            /* Add CONTENT-LANGUAGE header only if WEB_SERVER_CONTENT_LANGUAGE */
            /* is not empty and if Accept-Language header is not empty */
            struct SendInstruction *RespInstr;
            RespInstr = (struct SendInstruction *)
                va_arg(argp, struct SendInstruction *);
            assert(RespInstr);

            if (RespInstr->AcceptLanguageHeader[0] != '\0'){
                DLNA_LOGI("[VPPDLNA][%s][:%d]-: AcceptLanguage Header is present"
                    " in GET request. Language is %s, We support %s Language"
                    " \r\n", __FUNCTION__,__LINE__,
                    RespInstr->AcceptLanguageHeader,WEB_SERVER_CONTENT_LANGUAGE);

             /* As of Now supporting Language is ENGLISH. If Accept Language
                have language other than English,  we Send response with
                CONTENT LANGUAGE as ENGLISH */
            if (http_MakeMessage(buf, http_major_version,
                    http_minor_version, "ssc","CONTENT-LANGUAGE: ",
                    WEB_SERVER_CONTENT_LANGUAGE) != 0)
                    goto error_handler;
            }
        } else if (c == 'C') {
            if ((http_major_version > 1) ||
                (http_major_version == 1 && http_minor_version == 1)
                ) {
                /* connection header */
                if (membuffer_append_str(buf, "CONNECTION: close\r\n"))
                    goto error_handler;
            }
        }else if(c == 'A'){
            if ((http_major_version > 1) ||
                (http_major_version == 1 && http_minor_version == 1)
                ) {
                /* connection header */
                if (membuffer_append_str(buf, "CONNECTION: Keep-Alive\r\n"))
                    goto error_handler;
                }
        }else if (c == 'N') {
            /* content-length header */
            /*  MODIFIED by c00190074
            bignum = (off_t) va_arg(argp, off_t); */
            bignum = (off64_t) va_arg(argp, off64_t);
            assert(bignum >= 0);
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                         "shc", "Content-Length: ", bignum) != 0)
                goto error_handler;
        } else if (c == 'S' || c == 'U') {
            /* SERVER or USER-AGENT header */
            temp_str = (c == 'S') ? "SERVER: " : "USER-AGENT: ";
            get_sdk_info(tempbuf,(sizeof(tempbuf)-1));
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                         "ss", temp_str, tempbuf) != 0)
                goto error_handler;
        } else if (c == 'u') {
            get_soap_sdk_info(tempbuf,(sizeof(tempbuf)-1));
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                         "ss", "USER-AGENT: ", tempbuf) != 0)
                goto error_handler;
        } else if (c == 'Z') {
            /* C string */
            s = (char *)va_arg(argp, char *);

            if (s && strlen(s)) {
                if (membuffer_append_str(buf, "contentFeatures.dlna.org: ") != 0)
                    goto error_handler;
                if (membuffer_append(buf, s, strlen(s)) != 0)
                    goto error_handler;
                if (membuffer_append(buf, "\r\n", 2))
                    goto error_handler;
            }
        } else if (c == 'Y') {
            /* C string */
            s = (char *)va_arg(argp, char *);

            if (s && strlen(s)) {
                if (membuffer_append_str(buf, "transferMode.dlna.org: ") != 0)
                    goto error_handler;
                if (membuffer_append(buf, s, strlen(s)) != 0)
                    goto error_handler;
                if (membuffer_append(buf, "\r\n", 2))
                    goto error_handler;
            }
       } else if (c == 'X') {
            /* C string */
            s = (char *)va_arg(argp, char *);
            assert(s);
            if (membuffer_append_str(buf, "X-User-Agent: ") != 0)
                goto error_handler;
            if (membuffer_append(buf, s, strlen(s)) != 0)
                goto error_handler;
        } else if (c == 'R') {
            /* response start line */
            /*   e.g.: 'HTTP/1.1 200 OK' code */
            status_code = (int)va_arg(argp, int);
            assert(status_code > 0);
            snprintf(tempbuf, sizeof(tempbuf), "HTTP/%d.%d %d ",
                http_major_version, http_minor_version,
                status_code);
            /* str */
            status_msg = http_get_code_text(status_code);
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                         "ssc", tempbuf, status_msg) != 0)
                goto error_handler;
        } else if (c == 'B') {
            /* body of a simple reply */
            status_code = (int)va_arg(argp, int);
            snprintf(tempbuf, sizeof(tempbuf), "%s%d %s%s",
                "<html><body><h1>",
                status_code, http_get_code_text(status_code),
                "</h1></body></html>");

            /*  MODIFIED by c00190074
            bignum = (off_t)strlen(tempbuf); */
            bignum = strlen(tempbuf);

            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                         "NTcs", bignum,    /* content-length */
                         "text/html",    /* content-type */
                         tempbuf) != 0    /* body */
                )
                goto error_handler;
        } else if (c == 'Q') {
            /* request start line */
            /* GET /foo/bar.html HTTP/1.1\r\n */
            method = (http_method_t) va_arg(argp, http_method_t);
            method_str = method_to_str(method);
            url_str = (const char *)va_arg(argp, const char *);
            num = (size_t) va_arg(argp, size_t);        /* length of url_str */
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                         "ssbsdsdc", method_str,    /* method */
                         " ", url_str, num,        /* url */
                         " HTTP/", http_major_version, ".",
                         http_minor_version) != 0)
                goto error_handler;
        } else if (c == 'q') {
            /* request start line and HOST header */
            method = (http_method_t) va_arg(argp, http_method_t);
            uri_ptr = (uri_type *) va_arg(argp, uri_type *);
            assert(uri_ptr);
            if (http_FixUrl(uri_ptr, &url) != 0) {
                error_code = UPNP_E_INVALID_URL;
                goto error_handler;
            }
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                    "Q" "sbc", method, url.pathquery.buff,
                    url.pathquery.size, "HOST: ",
                    url.hostport.text.buff,
                    url.hostport.text.size) != 0)
                goto error_handler;
        } else if (c == 'T') {
            /* content type header */
            temp_str = (const char *)va_arg(argp, const char *);    /* type/subtype format */
            if (http_MakeMessage(buf, http_major_version, http_minor_version,
                         "ssc", "CONTENT-TYPE: ", temp_str) != 0)
                goto error_handler;
        } else {
                  /*lint -e506*/
            assert(0);
                  /*lint +e506*/
        }
    }
    goto ExitFunction;

error_handler:
    /* Default is out of memory error. */
    if (!error_code)
        error_code = UPNP_E_OUTOF_MEMORY;
    membuffer_destroy(buf);

ExitFunction:
    va_end(argp);
    return error_code;
}


/************************************************************************
 * Function: http_CalcResponseVersion
 *
 * Parameters:
 *    IN int request_major_vers;    Request major version
 *    IN int request_minor_vers;    Request minor version
 *    OUT int* response_major_vers;    Response mojor version
 *    OUT int* response_minor_vers;    Response minor version
 *
 * Description:
 *    Calculate HTTP response versions based on the request versions.
 *
 * Return: void
 ************************************************************************/
void http_CalcResponseVersion( IN int request_major_vers,
                          IN int request_minor_vers,
                          OUT int *response_major_vers,
                          OUT int *response_minor_vers)
{
    if ((request_major_vers > 1) ||
        (request_major_vers == 1 && request_minor_vers >= 1)) {
        *response_major_vers = 1;
        *response_minor_vers = 1;
    } else {
        *response_major_vers = request_major_vers;
        *response_minor_vers = request_minor_vers;
    }
}

/************************************************************************
* Function: MakeGetMessageEx
*
* Parameters:
*    const char *url_str;    String as a URL
*    membuffer *request;    Buffer containing the request
*    uri_type *url;         URI object containing the scheme, path
*                query token, etc.
*
* Description:
*    Makes the message for the HTTP GET method
*
* Returns:
*    UPNP_E_INVALID_URL
*     Error Codes returned by http_MakeMessage
*    UPNP_E_SUCCESS
************************************************************************/
int MakeGetMessageEx( const char *url_str,
                  membuffer * request,
                  uri_type * url,
                  struct SendInstruction *pRangeSpecifier)
{
    int errCode = UPNP_E_SUCCESS;
    char *urlPath = NULL;
    size_t hostlen = 0;
    char *hoststr, *temp;

    do {
        DLNA_LOGI("[VPPDLNA]:DOWNLOAD URL : %s\n", url_str);
        if ((errCode = http_FixStrUrl((char *)url_str,
                          strlen(url_str),
                          url)) != UPNP_E_SUCCESS) {
            break;
        }
        /* make msg */
        membuffer_init(request);
        urlPath = alloca(strlen(url_str) + 1);
        if (!urlPath) {
            errCode = UPNP_E_OUTOF_MEMORY;
            break;
        }
        memset(urlPath, 0, strlen(url_str) + 1);
        HISTRCPY(urlPath, url_str);
        hoststr = strstr(urlPath, "//");
        if (hoststr == NULL) {
            errCode = UPNP_E_INVALID_URL;
            break;
        }
        hoststr += 2;
        temp = strchr(hoststr, '/');
        if (temp == NULL) {
            errCode = UPNP_E_INVALID_URL;
            break;
        }
        *temp = '\0';
        hostlen = strlen(hoststr);
        *temp = '/';
        DLNA_LOGI("[VPPDLNA]:HOSTNAME : %s Length : %" PRIzu "\n",
               hoststr, hostlen);
        errCode = http_MakeMessage(request, 1, 1,
                       "Q" "s" "bc" "GDCUc",
                       HTTPMETHOD_GET, url->pathquery.buff,
                       url->pathquery.size, "HOST: ",
                       hoststr, hostlen, pRangeSpecifier);
        if (errCode != 0) {
            DLNA_LOGE("[VPPDLNA]:HTTP Makemessage failed\n");
            membuffer_destroy(request);
            return errCode;
        }
    } while (0);
    DLNA_LOGV("[VPPDLNA]:HTTP Buffer:\n%s\n" "----------END--------\n",
           request->buf);

    return errCode;
}

#define SIZE_RANGE_BUFFER 50

/************************************************************************
 * Function: http_OpenHttpGetEx
 *
 * Parameters:
 *    IN const char *url_str;        String as a URL
 *    IN OUT void **Handle;        Pointer to buffer to store HTTP
 *                    post handle
 *    IN OUT char **contentType;    Type of content
 *    OUT int *contentLength;        length of content
 *    OUT int *httpStatus;        HTTP status returned on receiving a
 *                    response message
 *    IN int timeout;            time out value
 *
 * Description:
 *    Makes the HTTP GET message, connects to the peer,
 *    sends the HTTP GET request, gets the response and parses the
 *    response.
 *
 * Return: int
 *    UPNP_E_SUCCESS        - On Success
 *    UPNP_E_INVALID_PARAM    - Invalid Paramters
 *    UPNP_E_OUTOF_MEMORY
 *    UPNP_E_SOCKET_ERROR
 *    UPNP_E_BAD_RESPONSE
 ************************************************************************/
int http_OpenHttpGetEx(
    IN const char *url_str,
    IN OUT void **Handle,
    IN OUT char **contentType,
    OUT int *contentLength,
    OUT int *httpStatus,
    IN int lowRange,
    IN int highRange,
    IN int timeout)
{
    int http_error_code;
    memptr ctype;
    SOCKET tcp_connection;
    size_t sockaddr_len;
    membuffer request;
    http_get_handle_t *handle = NULL;
    uri_type url;
    parse_status_t status;
    int errCode = UPNP_E_SUCCESS;
    /* char rangeBuf[SIZE_RANGE_BUFFER]; */
    struct SendInstruction rangeBuf;

    do {
        /* Checking Input parameters */
        if (!url_str || !Handle || !contentType || !httpStatus ) {
            errCode = UPNP_E_INVALID_PARAM;
            break;
        }
        /* Initialize output parameters */
        *httpStatus = 0;
        *Handle = handle;
        *contentType = NULL;
        *contentLength = 0;
        if (lowRange > highRange) {
            errCode = UPNP_E_INTERNAL_ERROR;
            break;
        }
        memset(&rangeBuf, 0, sizeof(rangeBuf));
        snprintf(rangeBuf.RangeHeader,(sizeof(rangeBuf.RangeHeader)/sizeof(char)),
            "Range: bytes=%d-%d\r\n", lowRange, highRange);
        membuffer_init(&request);
        errCode = MakeGetMessageEx(url_str, &request, &url, &rangeBuf);
        if (errCode != UPNP_E_SUCCESS)
            break;
        handle = (http_get_handle_t *)UPNP_MALLOC(sizeof(http_get_handle_t));
        if (!handle) {
                DLNA_LOGE("[VPPDLNA][%s][:%d]-:Memory Allocation Failure\r\n",
                        __FUNCTION__,__LINE__);
            errCode = UPNP_E_OUTOF_MEMORY;
            break;
        }
        memset(handle, 0, sizeof(*handle));
        parser_response_init(&handle->response, HTTPMETHOD_GET);
        tcp_connection = socket(url.hostport.IPaddress.ss_family, SOCK_STREAM, 0);
        if (tcp_connection == -1) {
            errCode = UPNP_E_SOCKET_ERROR;
            UPNP_FREE(handle);
            break;
        }
        if (sock_init(&handle->sock_info, tcp_connection) != UPNP_E_SUCCESS) {
            (void)sock_destroy(&handle->sock_info, SD_BOTH);
            errCode = UPNP_E_SOCKET_ERROR;
            UPNP_FREE(handle);
            break;
        }
        sockaddr_len = url.hostport.IPaddress.ss_family == AF_INET6 ?
            sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
        errCode  = private_connect(handle->sock_info.socket,
            (struct sockaddr *)&(url.hostport.IPaddress),
            (socklen_t)sockaddr_len, -1, -1);
        if (errCode == -1) {
            (void)sock_destroy(&handle->sock_info, SD_BOTH);
            errCode = UPNP_E_SOCKET_CONNECT;
            UPNP_FREE(handle);
            break;
        }
        /* send request */
        errCode = http_SendMessage(&handle->sock_info, &timeout,
            "b", request.buf, request.length);
        if (errCode != UPNP_E_SUCCESS) {
            (void)sock_destroy(&handle->sock_info, SD_BOTH);
            UPNP_FREE(handle);
            break;
        }
        status = (parse_status_t)ReadResponseLineAndHeaders(&handle->sock_info,
            &handle->response, &timeout, &http_error_code);
        if (status != PARSE_OK) {
            errCode = UPNP_E_BAD_RESPONSE;
            UPNP_FREE(handle);
            break;
        }
        status = parser_get_entity_read_method(&handle->response);
        if (status != PARSE_CONTINUE_1 && status != PARSE_SUCCESS) {
            errCode = UPNP_E_BAD_RESPONSE;
            UPNP_FREE(handle);
            break;
        }
        *httpStatus = handle->response.msg.status_code;
        errCode = UPNP_E_SUCCESS;

        if (!httpmsg_find_hdr(&handle->response.msg, HDR_CONTENT_TYPE, &ctype))
            /* no content-type */
            *contentType = NULL;
        else
            *contentType = ctype.buf;
        if (handle->response.position == POS_COMPLETE)
            *contentLength = 0;
        else if(handle->response.ent_position == ENTREAD_USING_CHUNKED)
            *contentLength = UPNP_USING_CHUNKED;
        else if(handle->response.ent_position == ENTREAD_USING_CLEN)
            *contentLength = (int)handle->response.content_length;
        else if(handle->response.ent_position == ENTREAD_UNTIL_CLOSE)
            *contentLength = UPNP_UNTIL_CLOSE;
        *Handle = handle;
    } while (0);

    membuffer_destroy(&request);

    return errCode;
}


/************************************************************************
 * Function: get_sdk_info
 *
 * Parameters:
 *    OUT char *info;    buffer to store the operating system information
 *
 * Description:
 *    Returns the server information for the operating system
 *
 * Return:
 *    UPNP_INLINE void
 ************************************************************************/
/* 'info' should have a size of at least 100 bytes */
void get_sdk_info(OUT char *info, IN int nInfoLen)
{
#ifdef WIN32
    OSVERSIONINFO versioninfo;
    versioninfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&versioninfo) != 0)
        snprintf(info,nInfoLen,
            "%d.%d.%d %d/%s, UPnP/1.0, Portable SDK for UPnP devices/"
            PACKAGE_VERSION "\r\n", versioninfo.dwMajorVersion,
            versioninfo.dwMinorVersion, versioninfo.dwBuildNumber,
            versioninfo.dwPlatformId, versioninfo.szCSDVersion);
    else
        *info = '\0';
#else
    int ret_code;
    struct utsname sys_info;

    ret_code = uname(&sys_info);
    if (ret_code == -1)
        *info = '\0';
    snprintf(info,nInfoLen,
        "%s/%s UPnP/1.0 Portable SDK for UPnP devices/"
        PACKAGE_VERSION "\r\n", sys_info.sysname, sys_info.release);
#endif
}

/*
    To support the DLNA Guideline requirement that the UPnP CP
    Must send the user-agent header in the soap action request
    and it must haeve DLNADOC/1.50 in the header value
*/
void get_soap_sdk_info(OUT char *info,IN int nInfoLen)
{
    int ret_code;
    struct utsname sys_info;

    ret_code = uname(&sys_info);
    if (ret_code == -1)
        *info = '\0';

    snprintf(info,nInfoLen,"%s/%s UPnP/1.0 Portable_SDK_for_UPnP_devices/"
    PACKAGE_VERSION" DLNADOC/1.50\r\n", sys_info.sysname, sys_info.release);
}
