#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include "tee_client_api.h"
#include "tee_client_id.h"
#include "tee_client_storage_api.h"

typedef unsigned int u32;
typedef int s32;
typedef unsigned char BYTE;

#define FOR_FPGA_TEST	1

#define TEEC_DEBUG 1

#define HDCPKEY_PATH "hdcpkey"

#define MAX_KEY_DATA_LEN 544
typedef enum {
	EN_NONE_TYPE = 0,
	EN_TESTKEY_TYPE = 1,
	EN_FACTORY_TYPE = 2,
} HDCP_KEY_TYPE;

typedef struct _HDCP_KEY_INFO {
	int nKeyType;	// HDCP Key type, for example: EN_TESTKEY_TYPE or EN_FACTORY_TYPE
	BYTE strEnKeyBuff[MAX_KEY_DATA_LEN];	// storage encrpted hdcp key, length is 544
} HDCP_KEY_INFO;

#ifdef FOR_FPGA_TEST
// For FPGA Test, storage tested hdcp key, length is 544
static BYTE strTestKeyBuff[MAX_KEY_DATA_LEN] = {
0x20, 0x05, 0x20, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
0xAE, 0xFE, 0x97, 0x7B, 0x06, 0x41, 0x39, 0x76, 0x8A, 0x3A, 0xD5, 0xF9, 0x80, 0x2C, 0xDC, 0xE9,
0x67, 0x09, 0x25, 0xFA, 0xA3, 0x35, 0x8A, 0x3F, 0x93, 0x3C, 0xCA, 0xCA, 0xDE, 0xE1, 0xA4, 0xCA,
0x5A, 0xEC, 0x60, 0x8A, 0x54, 0xDC, 0x8F, 0xF9, 0xCE, 0x64, 0x78, 0x0D, 0xFC, 0x7B, 0xF6, 0xF9,
0xB4, 0xA7, 0x26, 0xE4, 0x88, 0xF1, 0xCA, 0xE2, 0xEF, 0x75, 0x06, 0xCC, 0xDE, 0x31, 0xDD, 0x07,
0x72, 0x1E, 0x50, 0x25, 0xC5, 0x07, 0x10, 0x01, 0xA2, 0xA3, 0xF9, 0x13, 0x51, 0xDA, 0x00, 0x0C,
0x5C, 0xB7, 0xDA, 0xD3, 0x6B, 0x26, 0xDA, 0x13, 0x0B, 0xD1, 0x17, 0x9C, 0x60, 0x84, 0x51, 0x63,
0x93, 0xAF, 0x8E, 0x51, 0xA2, 0x07, 0xA4, 0x72, 0xFA, 0x89, 0xDD, 0x2D, 0x62, 0xAC, 0xF0, 0xE7,
0x0A, 0xAE, 0xFA, 0x3E, 0x22, 0xB3, 0x38, 0xCD, 0x90, 0xFF, 0x23, 0x67, 0xCD, 0x72, 0x07, 0xB4,
0x44, 0xD7, 0xF7, 0x1B, 0x88, 0x6F, 0x23, 0x7F, 0xBA, 0x6D, 0x89, 0xC6, 0x10, 0x15, 0x36, 0x79,
0xCB, 0xD7, 0x8A, 0x8F, 0xFC, 0x7E, 0xD9, 0x45, 0xC6, 0xC0, 0x60, 0x85, 0xEF, 0xE7, 0x2A, 0x9D,
0xF7, 0x05, 0xA6, 0x9D, 0x79, 0x22, 0x5D, 0x79, 0xB2, 0xA9, 0x6F, 0x43, 0xE1, 0x56, 0xC7, 0xB1,
0x3A, 0x34, 0xEE, 0xC0, 0xD5, 0xD3, 0x3B, 0x41, 0x1D, 0x1B, 0x2A, 0x8F, 0x8C, 0xD2, 0xB3, 0xE6,
0xBD, 0xB0, 0x43, 0x8F, 0x0F, 0x35, 0xB0, 0xAA, 0x86, 0x19, 0x98, 0x32, 0xB4, 0xFA, 0xBD, 0xD4,
0xA7, 0x6D, 0x28, 0xA9, 0xD5, 0x88, 0x60, 0x24, 0x2F, 0x51, 0xC0, 0x12, 0x1E, 0x97, 0x0C, 0x68,
0xD8, 0x8D, 0x4C, 0xED, 0x8F, 0x50, 0xFB, 0xDF, 0x4C, 0xD1, 0xCD, 0xE4, 0x8D, 0x7F, 0x0C, 0x8A,
0xEB, 0x55, 0x7E, 0x78, 0x64, 0x98, 0x9C, 0xDC, 0x52, 0xA4, 0xEE, 0xA6, 0x2D, 0x9E, 0xE8, 0xEF,
0xE0, 0x41, 0xF0, 0x13, 0xB9, 0x32, 0x43, 0x29, 0x18, 0x37, 0x4C, 0x49, 0x9F, 0x15, 0x34, 0x42,
0xBB, 0xE7, 0x91, 0x1C, 0xE3, 0x55, 0x1E, 0x86, 0x85, 0xD4, 0xAE, 0xAB, 0xF5, 0x4F, 0x9D, 0xFB,
0x4E, 0xF5, 0x3D, 0x91, 0x74, 0x28, 0x7C, 0x82, 0x36, 0xE8, 0x5B, 0x22, 0x20, 0xE4, 0x78, 0xF6,
0x25, 0xE4, 0xC8, 0x07, 0x50, 0x28, 0x4B, 0xAD, 0xF0, 0xA0, 0x96, 0xE2, 0x63, 0x3D, 0x32, 0x8E,
0x7A, 0x67, 0xB4, 0x71, 0x0C, 0xFC, 0xD8, 0x77, 0x1A, 0xF0, 0x30, 0xF6, 0x6F, 0x60, 0xB3, 0xFE,
0x45, 0x8B, 0x2F, 0x0C, 0x36, 0x28, 0x2D, 0x68, 0x1C, 0x75, 0x30, 0x06, 0x97, 0xEE, 0xA8, 0xE7,
0xC7, 0x49, 0x75, 0x2F, 0xA1, 0x76, 0x8B, 0x3F, 0x38, 0x82, 0x7C, 0x97, 0x14, 0x34, 0x7B, 0xFA,
0x12, 0xAA, 0xF0, 0xFA, 0x7D, 0x79, 0x05, 0xA3, 0x81, 0x95, 0x2A, 0x71, 0x2B, 0x47, 0x50, 0x4E,
0xE2, 0x0E, 0xDF, 0x5C, 0x9F, 0x95, 0x60, 0x69, 0xBA, 0x01, 0x63, 0x21, 0xD2, 0x82, 0x6A, 0xF3,
0x96, 0x9C, 0xD1, 0x6D, 0x53, 0xB7, 0xAC, 0xC7, 0xB8, 0x01, 0xB6, 0xD9, 0x55, 0xCD, 0xB4, 0x25,
0x39, 0x24, 0xD7, 0x5B, 0x98, 0x34, 0xA9, 0x34, 0x37, 0xF1, 0x0F, 0xEC, 0x16, 0x59, 0x3C, 0xA3,
0x0F, 0xB6, 0xC5, 0x24, 0xDC, 0xDD, 0x9C, 0xFF, 0x51, 0x26, 0xCA, 0x47, 0x63, 0xE5, 0x39, 0x27,
0xD6, 0x4E, 0xB1, 0x30, 0x72, 0x20, 0x21, 0x67, 0x6D, 0x78, 0xB8, 0x5D, 0x05, 0x3D, 0x7E, 0xA6,
0x31, 0x5D, 0xC2, 0x12, 0xB9, 0x5B, 0xD6, 0x56, 0xCC, 0x58, 0x3F, 0x61, 0x6D, 0x35, 0x5D, 0xF6,
0x1D, 0x85, 0xB7, 0x3B, 0x5F, 0x6E, 0xF5, 0x6C, 0xDF, 0xCE, 0x77, 0xEB, 0xC4, 0x40, 0x12, 0x21,
0x82, 0x28, 0x44, 0x20, 0x7F, 0x2C, 0x22, 0x93, 0xB9, 0x5E, 0x29, 0x34, 0x96, 0x53, 0x3D, 0xFC,
0x02, 0xFB, 0x4E, 0xC8, 0x18, 0x74, 0x77, 0x2E, 0xA3, 0xF0, 0xF4, 0x7B, 0xD6, 0x60, 0x90, 0x62};
#endif


int read_and_compare(const HDCP_KEY_INFO* hdcp_test_key)
{
    int32_t read_fd = 0;
    size_t read_size = 0;
    uint8_t* read_buff = NULL;
	HDCP_KEY_INFO read_key_info;
    size_t read_len = 0;
    uint8_t file_name[32] = HDCPKEY_PATH;
    TEEC_Result ret = TEEC_SUCCESS;

    ret = TEEC_InitStorageService();
    if (ret != TEEC_SUCCESS) {
        TEEC_Error("InitStorageService failed\n");
		ret = TEEC_ERROR_GENERIC;
        goto cleanup_1;
    } else {
        TEEC_Debug("InitStorageService success\n");
    }

    read_fd = TEEC_FOpen(file_name,
        TEE_DATA_FLAG_ACCESS_READ);
    if (read_fd == -1) {
        TEEC_Error("Open file %s failed\n", file_name);
		ret = TEEC_ERROR_GENERIC;
        goto cleanup_2;
    } else {
        TEEC_Debug("Open file %s success\n", file_name);
    }

    memset((uint8_t* )&read_key_info, 0x00, sizeof(read_key_info));
	read_buff = (uint8_t* )&read_key_info;
	read_len = 200;//sizeof(read_key_info);
    read_size = TEEC_FRead(read_fd, read_buff, read_len);
    if ((read_size != read_len)
        && (TEEC_GetFErr() != TEEC_SUCCESS)) {
        TEEC_Error("Read file failed, read_size=0x%x\n", read_size);
		ret = TEEC_ERROR_GENERIC;
        goto cleanup_3;
    } else {
        TEEC_Debug("Read file success, read_size=0x%x\n", read_size);
    }

    if (memcmp(hdcp_test_key, &read_key_info, read_len) != 0) {
        TEEC_Error("Read is NOT same to Write\n");
		ret = TEEC_ERROR_GENERIC;
        goto cleanup_3;
    } else {
        TEEC_Debug("Read is same to org, success\n");
    }

cleanup_3:
    TEEC_FClose(read_fd);
cleanup_2:
    TEEC_UninitStorageService();
cleanup_1:
    return ret;
}


int main(void)
{
    TEEC_Context context;
    TEEC_Session session;
    TEEC_Result result = TEEC_SUCCESS;
    TEEC_Operation operation;
    TEEC_UUID svc_id = TEE_SERVICE_HDCP;
    HDCP_KEY_INFO hdcp_test_key = {1, {0}};

#ifdef FOR_FPGA_TEST
	// For FPGA Test
	memcpy(hdcp_test_key.strEnKeyBuff, strTestKeyBuff, sizeof(strTestKeyBuff));
#endif

    result = TEEC_InitializeContext(
               NULL,
               &context);

    if(result != TEEC_SUCCESS) {
        goto cleanup_1;
    }
    result = TEEC_OpenSession(
                &context,
                &session,
                &svc_id,
                TEEC_LOGIN_PUBLIC,
                NULL,
                NULL,
                NULL);

    if(result != TEEC_SUCCESS) {
        goto cleanup_2;
    }
    TEEC_Debug("session id 0x%x\n", session.session_id);

    memset(&operation, 0x00, sizeof(operation));
    operation.started = 1;
    operation.paramTypes = TEEC_PARAM_TYPES(
									        TEEC_VALUE_INPUT,
									        TEEC_MEMREF_TEMP_INPUT,
									        TEEC_NONE,
									        TEEC_NONE);
    operation.params[0].value.a = 0xFFFF;
    operation.params[0].value.b = 0xFFFE;
    operation.params[1].tmpref.buffer = &hdcp_test_key;
    operation.params[1].tmpref.size = sizeof(HDCP_KEY_INFO);

    result = TEEC_InvokeCommand(
                 &session,
                 ECHO_CMD_ID_SEND_CMD,
                 &operation,
                 NULL);

    if (result != TEEC_SUCCESS) {
        TEEC_Error("invoke failed\n");
        goto cleanup_3;
    } else {
        TEEC_Debug("[0] : %x %x\n", operation.params[0].value.a, operation.params[0].value.b);
    }

//#ifdef FOR_FPGA_TEST
#if 0
	// read and compare
	result = read_and_compare(&hdcp_test_key);
	if (result != TEEC_SUCCESS) {
        TEEC_Error("read_and_compare failed return %d.\n", result);
        goto cleanup_3;
    } else {
	TEEC_Debug("hdcp key store test pass\n");
    }
#endif

cleanup_3:
    TEEC_CloseSession(&session);
cleanup_2:
    TEEC_FinalizeContext(&context);
cleanup_1:
    return result;
}
