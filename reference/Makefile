##############################################################################
#
# Project settings
#
##############################################################################


SOURCE_DIR  = ../../../../..
BINPATH     = ~/gcc-arm-none-eabi/bin
PWD         = $(shell pwd)
DATIME      = $(shell date --iso=seconds)
V          ?= 0


##############################################################################
#
# build log output
#
##############################################################################


ifeq ($(V),0)
Q           = @
else
Q           =
endif


##############################################################################
#
# config files
#
##############################################################################


FEATURE    ?= feature.mk

include $(FEATURE)

# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk
# Board Config
-include $(SOURCE_DIR)/config/board/$(BOARD_CONFIG)/board.mk


##############################################################################
#
# config
#
##############################################################################


RAM_BOOTING     = 0

DEBUG           = 0
FLOAT_TYPE      = hard
BUILD_DIR       = $(PWD)/Build

# Project name
PROJ_NAME       = mt7687_mcs_gpio
PROJ_PATH       = $(PWD)
OUTPATH         = $(PWD)/Build

APP_PATH        = project/mt7687_hdk/apps/mcs_gpio
APP_PATH_SRC    = $(APP_PATH)/src


##############################################################################
#
# SDK source files
#
##############################################################################


APP_FILES       = $(APP_PATH_SRC)/main.c     \
                  $(APP_PATH_SRC)/io_def.c   \
                  $(APP_PATH_SRC)/task_def.c \
                  $(APP_PATH_SRC)/net_init.c \
                  $(APP_PATH)/GCC/syscalls.c \

SYS_FILES       =  $(APP_PATH_SRC)/system_mt7687.c

ifeq ($(MTK_MINICLI_ENABLE),y)
APP_FILES       += $(APP_PATH_SRC)/cli_def.c
endif

APP_FILES       += $(APP_PATH_SRC)/mcs_upload.c

ifeq ($(MTK_SMARTCONNECT_HDK),y)
APP_FILES       += $(APP_PATH_SRC)/smart_connection.c
endif

ifeq ($(MTK_HOMEKIT_ENABLE),y)
APP_FILES       += $(APP_PATH_SRC)/homekit_init.c
endif

C_FILES         += $(APP_FILES) $(SYS_FILES)

S_FILES         += $(APP_PATH)/GCC/startup_mt7687.s


##############################################################################
#
# SDK object files
#
##############################################################################


C_OBJS          = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
S_OBJS          = $(S_FILES:%.s=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)


##############################################################################
#
# CFLAGS
#
##############################################################################


CFLAGS += -DMTK_FW_VERSION=\"$(MTK_FW_VERSION)-$(DATIME)\"
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/mt7687/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/src/include
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/ports/include

ifeq ($(MTK_SMARTCONNECT_HDK),y)
CFLAGS += -DMTK_SMARTCONNECT_HDK
endif

##############################################################################
#
# LDFLAGS
#
##############################################################################


LDFLAGS += $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles -u _printf_float -u _scanf_float

ifeq ($(RAM_BOOTING), 1)
LDFLAGS += -Wl,-Tmt7687_sram.ld -Wl,--gc-sections
else
LDFLAGS += -Wl,-Tmt7687_flash.ld -Wl,--gc-sections
endif

ifneq ($(MTK_DEBUG_LEVEL), none)
LDFLAGS += -Wl,-wrap=printf
endif


##############################################################################
#
# Libraries
#
##############################################################################


ifeq ($(MTK_HOMEKIT_ENABLE),y)
LIBS += $(OUTPATH)/libhomekit.a
LIBS += $(OUTPATH)/libjsonc.a
LIBS += $(OUTPATH)/libed25519.a
LIBS += $(OUTPATH)/libchacha20poly1305.a
LIBS += $(OUTPATH)/libcurve25519.a
LIBS += $(OUTPATH)/libsrp.a
endif
LIBS += $(OUTPATH)/libcjson.a
LIBS += $(OUTPATH)/libfota.a
LIBS += $(OUTPATH)/libtftp.a
LIBS += $(OUTPATH)/libhttpclient.a
LIBS += $(OUTPATH)/libmbedtls.a
LIBS += $(OUTPATH)/libminicli.a
ifeq ($(MTK_WIFI_WPS_ENABLE),y)
LIBS += $(OUTPATH)/libminisupp_wps.a
else
LIBS += $(OUTPATH)/libminisupp.a
endif
LIBS += $(OUTPATH)/libmqtt.a
LIBS += $(OUTPATH)/libnghttp2.a
ifeq ($(MTK_NVRAM_ENABLE),y)
LIBS += $(OUTPATH)/libnvram.a
endif
ifeq ($(MTK_NVDM_ENABLE),y)
LIBS += $(OUTPATH)/libnvdm.a
endif
LIBS += $(OUTPATH)/libsntp.a
LIBS += $(OUTPATH)/libxml.a
LIBS += $(OUTPATH)/libiperf.a
LIBS += $(OUTPATH)/librtos.a
LIBS += $(OUTPATH)/libbspex.a
LIBS += $(OUTPATH)/libbsp.a
LIBS += $(OUTPATH)/libbsp_sec.a
LIBS += $(OUTPATH)/libsmtcn.a
LIBS += $(OUTPATH)/libsmtcn_core.a
LIBS += $(OUTPATH)/libservice.a
LIBS += $(OUTPATH)/libping.a
LIBS += $(OUTPATH)/liblwip.a
LIBS += $(OUTPATH)/libhal.a
LIBS += $(OUTPATH)/libdhcpd.a
LIBS += $(SOURCE_DIR)/middleware/third_party/lzma_decoder/lib/liblzma.a


##############################################################################
#
# Target Rules
#
##############################################################################


.PHONY: libs proj clean

all: libs proj
  $(Q)mkdir -p $(BUILD_DIR)
  $(Q)$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
  $(Q)echo "Generate Assembly from elf:"
  #$(Q)echo cp $(SOURCE_DIR)/$(APP_PATH)/download_config/* $(OUTPATH)/
  $(Q)$(OBJDUMP) -S $(OUTPATH)/$(PROJ_NAME).elf > $(OUTPATH)/$(PROJ_NAME).elf.s
  #$(Q)cp $(SOURCE_DIR)/driver/chip/mt7687/wifi_n9/WIFI_RAM_CODE_MT7687_in_flash.bin $(OUTPATH)/
  #$(Q)cp $(SOURCE_DIR)/driver/board/mt76x7_hdk/bootloader/loader_inflash.bin $(OUTPATH)/
  $(Q)$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin
  $(Q)cp *.cmm $(OUTPATH)/


MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH)

libs:
  $(Q)make -C $(KRL_OS_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_TFTP_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_LWIP_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_CJSON_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_DHCPD_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_HTTPCLIENT_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_MBEDTLS_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_MINICLI_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_MINISUPP_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_MQTT_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_NGHTTP2_PATH) $(MOD_EXTRA)
ifeq ($(MTK_NVRAM_ENABLE),y)
  $(Q)make -C $(MID_NVRAM_PATH) $(MOD_EXTRA)
endif
ifeq ($(MTK_NVDM_ENABLE),y)
  $(Q)make -C $(MID_NVDM_PATH) $(MOD_EXTRA)
endif
  $(Q)make -C $(MID_SMTCN_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_SMTCN_CORE_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_SNTP_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_XML_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_BSPEXT_PATH) $(MOD_EXTRA)
  $(Q)make -C $(DRV_CHIP_PATH) $(MOD_EXTRA)
  $(Q)make -C $(KRL_SRV_PATH) $(MOD_EXTRA)
  $(Q)make -C $(DRV_BSP_PATH) $(MOD_EXTRA)
  $(Q)make -C $(DRV_BSP_SEC_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_PING_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_IPERF_PATH) $(MOD_EXTRA)
  $(Q)make -C $(MID_FOTA_PATH) $(MOD_EXTRA)
ifeq ($(MTK_HOMEKIT_ENABLE),y)
  $(Q)make -C $(MID_HOMEKIT_PATH)  $(MOD_EXTRA)
  $(Q)make -C $(MID_CURVE25519_PATH)  $(MOD_EXTRA)
  $(Q)make -C $(MID_JSONC_PATH)  $(MOD_EXTRA)
  $(Q)make -C $(MID_ED25519_PATH)  $(MOD_EXTRA)
  $(Q)make -C $(MID_CHACHA20POLY1305_PATH)  $(MOD_EXTRA)
  $(Q)make -C $(MID_SRP_PATH)  $(MOD_EXTRA)
endif


proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS)
  @echo Linking...
  @if [ -e "$@" ]; then rm -f "$@"; fi
  @if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
  @if [ -e "$(OUTPATH)/$(PROJ_NAME).hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).hex"; fi
  @if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
  $(Q)$(CXX) $(LDFLAGS) $^ -Wl,-Map=$(OUTPATH)/$(PROJ_NAME).map -lm -o $@ 2>>$(ERR_LOG)
  $(Q)$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
  $(Q)$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
  @echo Done

$(LIBS): libs

include $(SOURCE_DIR)/.rule.mk

clean:
  $(Q)rm -rf $(OUTPATH)
  $(Q)if [ -e $(MID_MINISUPP_PATH) ]; then \
    make -C $(MID_MINISUPP_PATH) $(MOD_EXTRA) clean \
  ;fi

.PHONY: options

options:
  @for opt in `sed 's/[()]/\n/g' $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk|sed 's/-D/ /'|sort|uniq|grep ^MTK_|grep _ENABLE$$`; do echo $$opt; done
print-%  : ; @echo $* = $($*)
