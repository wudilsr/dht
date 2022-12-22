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

TEEC_Result Invoke_FreePersistentObjectEnumerator_handle_not_valid(void)
{
    TEEC_Operation operation;
    TEEC_Result result;
    uint32_t OUT_ExpectedReturn = TEE_ERROR_TARGET_DEAD  ;
    operation.started = 1;
    operation.paramTypes = TEEC_PARAM_TYPES(
            TEEC_NONE,
            TEEC_NONE,
            TEEC_NONE,
            TEEC_NONE);

    result = TEEC_InvokeCommand(&session, CMD_DS_FreePersistentObjectEnumeratorPanic , &operation, &origin);
    if (result != OUT_ExpectedReturn) {
        TEEC_Error("invoke failed, codes=0x%x, origin=0x%x\n", result, origin);
        printf("OUT_ExpectedReturn is 0x%x\n",OUT_ExpectedReturn);
	 result = -1;
        goto error;
    }
    return TEEC_SUCCESS;
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

    if(TEEC_SUCCESS != Invoke_FreePersistentObjectEnumerator_handle_not_valid())
    {
        TEEC_Error("Invoke_FreePersistentObjectEnumerator_handle_not_valid is Failed!\n");
        goto error;
    }
    printf("Invoke_FreePersistentObjectEnumerator_handle_not_valid is Successful!\n");

    TEEC_CloseSession(&session);
cleanup_2:
    TEEC_FinalizeContext(&context);
cleanup_1:
    return TEEC_SUCCESS;
    error:
        TEEC_CloseSession(&session);
        TEEC_FinalizeContext(&context);
        return result;
}