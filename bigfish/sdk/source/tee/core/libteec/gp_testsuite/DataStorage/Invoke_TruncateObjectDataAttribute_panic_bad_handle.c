#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "tee_client_api.h"
#include "tee_client_id.h"
#include "client_ds.h"

static TEEC_Session session;
static uint32_t origin;

void retrieveBufferFromUint32(uint32_t n, char *buffer)
{
   uint32_t i;
   for (i=0;i<4;i++)
    {
        buffer[i] = (n<<i*8)>>24;
    }
   /* NOTE : Tools must write integers in BIG ENDIAN format */

   return;
}

//testcase No.38 TruncateObjectData_panic
TEEC_Result Invoke_TruncateObjectDataAttribute_panic_bad_handle(void)
{
    TEEC_Operation operation;
    TEEC_Result result;
    uint32_t OUT_ExpectedReturn = TEE_ERROR_TARGET_DEAD  ;

    operation.started = 1;
    operation.paramTypes = TEEC_PARAM_TYPES(
            TEEC_VALUE_INPUT,
            TEEC_NONE,
            TEEC_NONE,
            TEEC_NONE);

    operation.params[0].value.b = TEEC_VALUE_UNDEF;
    printf("test for CMD_TruncateObjectData_panic\n");

    //test for badhadle 0xFFFFDEAD result data abort
    operation.params[0].value.a = CASE_DATA_BAD_HANDLE;
    result = TEEC_InvokeCommand(&session, CMD_TruncateObjectData_panic, &operation, &origin);
    if (result != OUT_ExpectedReturn) {
        TEEC_Error("invoke failed, codes=0x%x, origin=0x%x\n", result, origin);
	 result = -1;
        goto error;
    }

    return 0;
    error:
        return result;
}

int main(void)
{
    TEEC_Context context;
    TEEC_Result result;

    result = TEEC_InitializeContext(NULL, &context);
    if(result != TEEC_SUCCESS) {
        TEEC_Error("teec initial failed\n");
        goto cleanup_1;
    }

    result = TEEC_OpenSession(&context, &session, &EXAMPLE_uuid,
            TEEC_LOGIN_PUBLIC, NULL, NULL, NULL);
    if(result != TEEC_SUCCESS) {
        TEEC_Error("teec open session failed\n");
        goto cleanup_2;
    }
    //start test
    if((result= Invoke_TruncateObjectDataAttribute_panic_bad_handle()) != 0)
    {
        TEEC_Error("Invoke_TruncateObjectDataAttribute_panic_bad_handle is Failed!\n");
        goto error;
    }
    printf("Invoke_TruncateObjectDataAttribute_panic_bad_handle is Successful!\n");
    TEEC_CloseSession(&session);
cleanup_2:
    TEEC_FinalizeContext(&context);
cleanup_1:
    return 0;
    error:
        TEEC_CloseSession(&session);
        TEEC_FinalizeContext(&context);
        return result;
}
