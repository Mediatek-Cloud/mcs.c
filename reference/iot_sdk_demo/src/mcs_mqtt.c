#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "MQTTClient.h"
#include "mqtt.h"
#include "os.h"
#include "mcs.h"

#define MQTT_HOST_COM "mqtt.mcs.mediatek.com"
#define MQTT_HOST_CN "mqtt.mcs.mediatek.cn"

#include "hal_sys.h"
#include "fota.h"
#include "fota_config.h"

#define MIN(a,b) ((a) < (b) ? a : b)

char rcv_buf[150] = {0};

void mqttMessageArrived(MessageData *md)
{
    char rcv_buf_old[150] = {0};
    MQTTMessage *message = md->message;

    const size_t write_len = MIN((size_t)(message->payloadlen), 150 - 1);
    strncpy(rcv_buf, message->payload, write_len);
    rcv_buf[write_len] = 0;
    printf("rcv1: %s\n", rcv_buf);

    char split_buf[MCS_MAX_STRING_SIZE] = {0};
    strcpy(split_buf, rcv_buf);

    char *arr[5];
    char *del = ",";
    mcs_splitn(arr, split_buf, del, 5);

    if (0 == strncmp (arr[1], "FOTA", 4)) {
        char *s = mcs_replace(arr[4], "https", "http");
        fota_download_by_http(s);
        fota_ret_t err;
        err = fota_trigger_update();
        if (0 == err){
            hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
            return 0;
        } else {
            return -1;
        }
    } else {
        if (strcmp(rcv_buf_old, rcv_buf) != 0) {
            rcv_buf[(size_t)(message->payloadlen)] = '\0';
            * rcv_buf_old = "";
            strcpy(*rcv_buf_old, rcv_buf);
            mcs_mqtt_callback(rcv_buf);
        }
    }

}

void mcs_mqtt_init(void (*mcs_mqtt_callback)(char *))
{
    static int arrivedcount = 0;
    Client c;   //MQTT client
    MQTTMessage message;
    int rc = 0;

    printf("topic: %s\n", TOPIC);
    printf("clientId: %s\n", CLIENTID);
    printf("port: %s\n", PORT);
    // printf("qos: %s\n", qos_method);

    arrivedcount = 0;

    unsigned char msg_buf[100];     //generate messages such as unsubscrube
    unsigned char msg_readbuf[100]; //receive messages such as unsubscrube ack

    Network n;  //TCP network
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

    //init mqtt network structure
    NewNetwork(&n);

    if (strcmp(HOST, "com") == 0) {
        rc = ConnectNetwork(&n, MQTT_HOST_COM, PORT);
    } else {
        rc = ConnectNetwork(&n, MQTT_HOST_CN, PORT);
    }

    if (rc != 0) {
        printf("TCP connect fail,status -%4X\n", -rc);
        return true;
    }

    //init mqtt client structure
    MQTTClient(&c, &n, 12000, msg_buf, 100, msg_readbuf, 100);

    //mqtt connect req packet header
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.clientID.cstring = CLIENTID;
    data.username.cstring = NULL;
    data.password.cstring = NULL;
    data.keepAliveInterval = 10;
    data.cleansession = 1;

    //send mqtt connect req to remote mqtt server
    rc = MQTTConnect(&c, &data);

    if (rc != 0) {
        printf("MQTT connect fail,status%d\n", rc);
    }

    printf("Subscribing to %s\n", TOPIC);

    // if (strcmp(qos_method, "0") == 0) {
        rc = MQTTSubscribe(&c, TOPIC, QOS0, mqttMessageArrived);
    // } else if (strcmp(qos_method, "1") == 0) {
    //     rc = MQTTSubscribe(&c, topic, QOS1, mqttMessageArrived);
    // } else if (strcmp(qos_method, "2") == 0) {
    //     rc = MQTTSubscribe(&c, topic, QOS2, mqttMessageArrived);
    // }

    printf("Client Subscribed %d\n", rc);

    for(;;) {
        MQTTYield(&c, 1000);
    }

    return true;
}