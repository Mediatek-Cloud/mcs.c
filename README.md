# mcs.c

## API

* `void mcs_upload_datapoint(char *);`
  - Upload data points to MCS.

* `void mcs_tcp_init(void (*mcs_tcp_callback)(char *));`
  - Listen the TCP command from MCS.

* `void mcs_mqtt_init(void (*mcs_mqtt_callback)(char *));`
  - Listen the MQTT from MCS.

## Usage

### For TCP
#### Please see `/reference/iot_sdk_demo` :
* Copy /reference/iot_sdk_demo folder to your SDK: /project/mt7687_hdk/apps/iot_sdk_demo

### For MQTT
#### Please  see `/reference/iot_sdk_demo_mqtt` :
* Copy /reference/iot_sdk_demo_mqtt folder to your SDK: /project/mt7687_hdk/apps/iot_sdk_demo_mqtt

## Binding MTK RTOS SDK version
* 4.1.0 SDK
