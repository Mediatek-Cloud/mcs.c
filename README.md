# mcs.c

## API

* `void mcs_upload_datapoint(char *);`
  - Upload data points to MCS.

* `void mcs_tcp_init(void (*mcs_tcp_callback)(char *));`
  - Listen the TCP command from MCS.

* `void mcs_mqtt_init(void (*mcs_mqtt_callback)(char *));`
  - Listen the MQTT from MCS.

* `smart_config_if_enabled()`
  - Enable smart connection with MCS app.

## Usage

### 請參考 /reference 下的 iot_sdk_demo project 目錄結構
* Copy /reference/iot_sdk_demo folder to your SDK: /project/mt7687_hdk/apps/iot_sdk_demo

## Binding MTK RTOS SDK version

* [V3.3.1](https://cdn.mediatek.com/download_page/index.html?platform=RTOS&version=v3.3.1&filename=LinkIt_SDK_V3.3.1_public.tar.gz)
