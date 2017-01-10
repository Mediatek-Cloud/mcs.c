#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "ethernetif.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"
#include "timers.h"
#include "os.h"
#include "httpclient.h"
#include "mcs.h"

/* Now only .com , must do for china */
#define HTTPS_MTK_CLOUD_URL_COM "http://api.mediatek.com/mcs/v2/devices/"
#define HTTPS_MTK_CLOUD_URL_CN "http://api.mediatek.cn/mcs/v2/devices/"

#include "hal_sys.h"
#include "fota.h"
#include "fota_config.h"

void mqttMessageArrived(MessageData *md) {
    char rcv_buf_old[100] = {0};

    MQTTMessage *message = md->message;
    char rcv_buf[100] = {0};
    strcpy(rcv_buf, message->payload);

    char split_buf[MAX_STRING_SIZE] = {0};
    strcpy(split_buf, rcv_buf);

    char *arr[7];
    char *del = ",";
    mcs_split(arr, split_buf, del);

    if (0 == strncmp (arr[3], "FOTA", 4)) {
        char *s = mcs_replace(arr[6], "https", "http");
        printf("fota url: %s\n", s);
        fota_download_by_http(s);
        fota_trigger_update();
        fota_ret_t err;
        err = fota_trigger_update();
        if (0 == err ){
            hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
            // LOG_I(fota_dl_api, "Reboot device!");
            return 0;
        } else {
            // LOG_E(fota_dl_api, "Trigger FOTA error!");
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

void mcs_mqtt_init(void (*mcs_mqtt_callback)(char *)) {
    static int arrivedcount = 0;
    Client c;   //MQTT client
    MQTTMessage message;
    int rc = 0;

    printf("topic: %s\n", topic);
    printf("clientId: %s\n", clientId);
    printf("port: %s\n", port);
    // printf("qos: %s\n", qos_method);

    arrivedcount = 0;

    unsigned char msg_buf[100];     //generate messages such as unsubscrube
    unsigned char msg_readbuf[100]; //receive messages such as unsubscrube ack

    Network n;  //TCP network
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

    //init mqtt network structure
    NewNetwork(&n);

    if (strcmp(host, "com") == 0) {
        rc = ConnectNetwork(&n, MQTT_HOST_COM, port);
    } else {
        rc = ConnectNetwork(&n, MQTT_HOST_CN, port);
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
    data.clientID.cstring = clientId;
    data.username.cstring = NULL;
    data.password.cstring = NULL;
    data.keepAliveInterval = 10;
    data.cleansession = 1;

    //send mqtt connect req to remote mqtt server
    rc = MQTTConnect(&c, &data);

    if (rc != 0) {
        printf("MQTT connect fail,status%d\n", rc);
    }

    printf("Subscribing to %s\n", topic);

    // if (strcmp(qos_method, "0") == 0) {
        rc = MQTTSubscribe(&c, topic, QOS0, mqttMessageArrived);
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