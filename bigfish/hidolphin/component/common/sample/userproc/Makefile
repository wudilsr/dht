#===============================================================================
# export variable
#===============================================================================
ifeq ($(CFG_HI_EXPORT_FLAG),)
SDK_DIR := $(CURDIR)/../..
include $(SDK_DIR)/base.mak
endif

include $(SAMPLE_DIR)/base.mak

#===============================================================================
# local variable
#===============================================================================

CFLAGS +=   -I$(SDK_INC_DIR)        \
            -I$(SAMPLE_DIR)/common

LIBPATH = -L$(PUB_DIR)/lib/static

DEP_LIBS := -lhi_common

LIB = -Wl,--start-group $(SYS_LIBS) $(DEP_LIBS) -Wl,--end-group

APP     := sample_userproc

OBJS    := sample_userproc.o

#===============================================================================
# rules
#===============================================================================
.PHONY: all clean install uninstall

all: $(APP)

$(APP): $(OBJS)
	$(AT)$(CC) -o $@ $^ $(LIBPATH) $(LIB)

clean:
	-$(AT)rm -f $(OBJS)
	-$(AT)rm -f $(APP)

%.o: %.c
	$(AT)$(CC) $(CFLAGS) -c -o $@ $^

install:

uninstall:
