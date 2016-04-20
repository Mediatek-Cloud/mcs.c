# mcs.c

## API

* `void mcs_upload_datapoint(char *);`
  - Upload data points to MCS.

* `void mcs_tcp_init(void (*mcs_tcp_callback)(char *));`
  - Listen the TCP command from MCS.

* `smart_config_if_enabled()`
  - smart connection with MCS app.

## Usage

* copy mcs.c, smart_connection.c into `{SDK_Root}/project/mt7687_hdk/{your project name}/src/`

* copy mcs.h, smart_connection.h into `{SDK_Root}/project/mt7687_hdk/{your project name}/inc/`

* 至您的 `{SDK_Root}/middleware/MTK/fota/src/76x7/fota_cli.c` 把他替換成這個：[fota_cli.c](https://gist.github.com/iamblue/ca2b8391c368f485937d4414d8333b8a)

* 至您的 `{SDK_Root}/project/common/bsp_ex/src/wifi_ex_config.c` 找到 `uint8_t _smart_config_test(uint8_t len, char *param[])` 這隻 function 把他替換成這個內容: [Click me](https://gist.github.com/iamblue/35481f606e94d917c050ec198859307e)

* Following, we assume `{SDK_Root}/project/mt7687_hdk/{your project name}/` is the `{project_root}` path.

* If you want to use the smart connection feature, edit the `{project_root}/GCC/feature.mk`:

``` Makefile
  # add this line:
  MTK_SMARTCONNECT_HDK = y
  # Please reference mcs.c/reference/feature.mk, line24

```
* Edit `{project_root}/main.c`:

``` c
  // add this line:
  #include "smart_connection.h"
  #include "mcs.h"

  // Please reference mcs.c/reference/main.c, line18
```

* Edit `{project_root/GCC/Makefile}`:

``` Makefile
  # add this line:

  APP_FILES       += $(APP_PATH_SRC)/mcs.c

  ifeq ($(MTK_SMARTCONNECT_HDK),y)
  APP_FILES       += $(APP_PATH_SRC)/smart_connection.c
  endif

  # Please reference mcs.c/reference/Makefile, line85

  ifeq ($(MTK_SMARTCONNECT_HDK),y)
    CFLAGS += -DMTK_SMARTCONNECT_HDK
  endif

  # Please reference mcs.c/reference/Makefile, line130
```

## SDK version

* V3.1.0
