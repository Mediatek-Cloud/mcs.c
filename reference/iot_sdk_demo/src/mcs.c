#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "network_init.h"
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

#include "MQTTClient.h"

/* tcp config */
#define SOCK_TCP_SRV_PORT 443

#define MAX_STRING_SIZE 200
TimerHandle_t heartbeat_timer;

/* RESTful config */
#define BUF_SIZE   (1024 * 1)
/* Now only .com , must do for china */
#define HTTPS_MTK_CLOUD_URL_COM "http://api.mediatek.com/mcs/v2/devices/"
#define HTTPS_MTK_CLOUD_URL_CN "http://api.mediatek.cn/mcs/v2/devices/"

/* MQTT HOST */
#define MQTT_HOST_COM "mqtt.mcs.mediatek.com"
#define MQTT_HOST_CN "mqtt.mcs.mediatek.cn"

char *TCP_ip [20] = {0};

/* utils */
void mcs_split(char **arr, char *str, const char *del) {
  char *s = strtok(str, del);
  while(s != NULL) {
    *arr++ = s;
    s = strtok(NULL, del);
  }
}

/**
 * @brief Split MCS response into limited splits
 * @details There two difference between mcs_split:
 *          1. This function can avoid burst of MCS data
 *          (for now, two MCS response data concatnates sometimes when sending requests in high frequency)
 *          2. This function is reentrant version of mcs_split
 *          (use strtok_r instead of strtok)
 *
 * @param dst output buffer
 * @param src input buffer
 * @param delimiter
 * @param max_split max number of splits
 */
void mcs_splitn(char ** dst, char * src, const char * delimiter, uint32_t max_split)
{
    uint32_t split_cnt = 0;
    char *saveptr = NULL;
    char *s = strtok_r(src, delimiter, &saveptr);
    while (s != NULL && split_cnt < max_split) {
        *dst++ = s;
        s = strtok_r(NULL, delimiter, &saveptr);
        split_cnt++;
    }
}

char *mcs_replace(char *st, char *orig, char *repl) {
  static char buffer[1024];
  char *ch;
  if (!(ch = strstr(st, orig)))
   return st;
  strncpy(buffer, st, ch-st);
  buffer[ch-st] = 0;
  sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
  return buffer;
}

/* to get TCP IP */
HTTPCLIENT_RESULT getInitialTCPIP () {
    int ret = HTTPCLIENT_ERROR_CONN;
    httpclient_t client = {0};
    char *buf = NULL;

    httpclient_data_t client_data = {0};

    /* deviceKey */
    char deviceKey[20];
    int nvdm_deviceKey_len = sizeof(deviceKey);
    nvdm_read_data_item("common", "deviceKey", (uint8_t *)deviceKey, (uint32_t *)&nvdm_deviceKey_len);

    /* deviceId */
    char deviceId[20];
    int nvdm_deviceId_len = sizeof(deviceId);
    nvdm_read_data_item("common", "deviceId", (uint8_t *)deviceId, (uint32_t *)&nvdm_deviceId_len);

    /* set Url */
    char get_url[70] ={0};

    char host[5];
    int nvdm_host_len = sizeof(host);
    nvdm_read_data_item("common", "host", (uint8_t *)host, (uint32_t *)&nvdm_host_len);

    if (strcmp(host, "com") == 0) {
        strcat(get_url, HTTPS_MTK_CLOUD_URL_COM);
    } else {
        strcat(get_url, HTTPS_MTK_CLOUD_URL_CN);
    }

    strcat(get_url, deviceId);
    strcat(get_url, "/connections.csv");

    /* Set header */
    char header[40] = {0};
    strcat(header, "deviceKey:");
    strcat(header, deviceKey);
    strcat(header, "\r\n");

    buf = pvPortMalloc(BUF_SIZE);
    if (buf == NULL) {
        return ret;
    }
    buf[0] = '\0';
    ret = httpclient_connect(&client, get_url, HTTPS_PORT);

    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    httpclient_set_custom_header(&client, header);

    ret = httpclient_get(&client, get_url, HTTP_PORT, &client_data);
    if (ret < 0) {
        return ret;
    }

    printf("content:%s\n", client_data.response_buf);

    if (200 == httpclient_get_response_code(&client)) {
        char split_buf[MAX_STRING_SIZE] = {0};
        strcpy(split_buf, client_data.response_buf);

        char *arr[1];
        char *del = ",";
        mcs_split(arr, split_buf, del);
        strcpy(TCP_ip, arr[0]);
    }
    vPortFree(buf);
    httpclient_close(&client, HTTPS_PORT);
    return ret;
}

void mcs_upload_datapoint(char *value)
{
    /* upload mcs datapoint */
    httpclient_t client = {0};
    char *buf = NULL;

    int ret = HTTPCLIENT_ERROR_CONN;
    httpclient_data_t client_data = {0};
    char *content_type = "text/csv";
    // char post_data[32];

    /* deviceKey */
    char deviceKey[20];
    int nvdm_deviceKey_len = sizeof(deviceKey);
    nvdm_read_data_item("common", "deviceKey", (uint8_t *)deviceKey, (uint32_t *)&nvdm_deviceKey_len);

    /* deviceId */
    char deviceId[20];
    int nvdm_deviceId_len = sizeof(deviceId);
    nvdm_read_data_item("common", "deviceId", (uint8_t *)deviceId, (uint32_t *)&nvdm_deviceId_len);

    /* Set post_url */
    char post_url[70] ={0};

    char host[5];
    int nvdm_host_len = sizeof(host);
    nvdm_read_data_item("common", "host", (uint8_t *)host, (uint32_t *)&nvdm_deviceId_len);

    if (strcmp(host, "com") == 0) {
        strcat(post_url, HTTPS_MTK_CLOUD_URL_COM);
    } else {
        strcat(post_url, HTTPS_MTK_CLOUD_URL_CN);
    }

    strcat(post_url, deviceId);
    strcat(post_url, "/datapoints.csv");

    /* Set header */
    char header[40] = {0};
    strcat(header, "deviceKey:");
    strcat(header, deviceKey);
    strcat(header, "\r\n");

    printf("header: %s\n", header);
    printf("url: %s\n", post_url);
    printf("data: %s\n", value);

    buf = pvPortMalloc(BUF_SIZE);
    if (buf == NULL) {
        printf("buf malloc failed.\r\n");
        return ret;
    }
    buf[0] = '\0';
    ret = httpclient_connect(&client, post_url, HTTPS_PORT);

    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    client_data.post_content_type = content_type;
    // sprintf(post_data, data);
    client_data.post_buf = value;
    client_data.post_buf_len = strlen(value);
    httpclient_set_custom_header(&client, header);
    ret = httpclient_send_request(&client, post_url, HTTPCLIENT_POST, &client_data);
    if (ret < 0) {
        return ret;
    }
    ret = httpclient_recv_response(&client, &client_data);
    if (ret < 0) {
        return ret;
    }
    printf("\n************************\n");
    printf("httpclient_test_keepalive post data every 5 sec, http status:%d, response data: %s\r\n", httpclient_get_response_code(&client), client_data.response_buf);
    printf("\n************************\n");
    vPortFree(buf);
    httpclient_close(&client, HTTPS_PORT);
    return ret;
}

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

    char topic[50];
    int nvdm_topic_len = sizeof(topic);
    nvdm_read_data_item("common", "topic", (uint8_t *)topic, (uint32_t *)&nvdm_topic_len);

    char clientId[50];
    int nvdm_clientId_len = sizeof(clientId);
    nvdm_read_data_item("common", "clientId", (uint8_t *)clientId, (uint32_t *)&nvdm_clientId_len);

    char port[5];
    int nvdm_port_len = sizeof(port);
    nvdm_read_data_item("common", "port", (uint8_t *)port, (uint32_t *)&nvdm_port_len);

    // char qos_method[1] = {0};
    // int nvdm_qos_method_len = sizeof(qos_method);
    // nvdm_read_data_item("common", "qos", (uint8_t *)qos_method, (uint32_t *)&nvdm_qos_method_len);

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

    char host[5];
    int nvdm_host_len = sizeof(host);
    nvdm_read_data_item("common", "host", (uint8_t *)host, (uint32_t *)&nvdm_host_len);

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

/* tcp connection */
int32_t mcs_tcp_init(void (*mcs_tcp_callback)(char *))
{
    int s;
    int c;
    int ret;
    struct sockaddr_in addr;
    int count = 0;
    int rcv_len, rlen;

    int32_t mcs_ret = MCS_TCP_DISCONNECT;

    /* Setting the TCP ip */
    if (HTTPCLIENT_OK != getInitialTCPIP()) {
        return MCS_TCP_INIT_ERROR;
    }

    /* deviceId */
    char deviceId[20];
    int nvdm_deviceId_len = sizeof(deviceId);
    nvdm_read_data_item("common", "deviceId", (uint8_t *)deviceId, (uint32_t *)&nvdm_deviceId_len);

    /* deviceKey */
    char deviceKey[20];
    int nvdm_deviceKey_len = sizeof(deviceKey);
    nvdm_read_data_item("common", "deviceKey", (uint8_t *)deviceKey, (uint32_t *)&nvdm_deviceKey_len);

    /* command buffer */
    char cmd_buf [50]= {0};
    strcat(cmd_buf, deviceId);
    strcat(cmd_buf, ",");
    strcat(cmd_buf, deviceKey);
    strcat(cmd_buf, ",0");

mcs_tcp_connect:
    os_memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SOCK_TCP_SRV_PORT);
    addr.sin_addr.s_addr =inet_addr(TCP_ip);

    /* create the socket */
    s = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        mcs_ret = MCS_TCP_SOCKET_INIT_ERROR;
        printf("tcp client create fail 0\n");
        goto idle;
    }

    ret = lwip_connect(s, (struct sockaddr *)&addr, sizeof(addr));

    if (ret < 0) {
        lwip_close(s);
        printf("tcp client connect fail 1\n");
        goto mcs_tcp_connect;
    }

    /* timer */
    void tcpTimerCallback( TimerHandle_t pxTimer ) {
        ret = lwip_write(s, cmd_buf, sizeof(cmd_buf));
    }

    heartbeat_timer = xTimerCreate("TimerMain", (30*1000 / portTICK_RATE_MS), pdTRUE, (void *)0, tcpTimerCallback);
    xTimerStart( heartbeat_timer, 0 );

    for (;;) {
        char rcv_buf[MAX_STRING_SIZE] = {0};

        if (0 == count) {
            ret = lwip_write(s, cmd_buf, sizeof(cmd_buf));
        }

        LOG_I(common, "MCS tcp-client waiting for data...");
        rcv_len = 0;
        rlen = lwip_recv(s, &rcv_buf[rcv_len], sizeof(rcv_buf) - 1 - rcv_len, 0);
        rcv_len += rlen;

        if ( 0 == rcv_len ) {
            return MCS_TCP_DISCONNECT;
        }

        LOG_I(common, "MCS tcp-client received data:%s", rcv_buf);

        /* split the string of rcv_buffer */
        char split_buf[MAX_STRING_SIZE] = {0};
        strcpy(split_buf, rcv_buf);

        char *arr[7];
        char *del = ",";
        mcs_splitn(arr, split_buf, del, 7);
        if (0 == strncmp (arr[3], "FOTA", 4)) {
            char *s = mcs_replace(arr[6], "https", "http");
            printf("fota url: %s\n", s);
            char data_buf [100] = {0};
            strcat(data_buf, "status");
            strcat(data_buf, ",,fotaing");
            mcs_upload_datapoint(data_buf);
            fota_download_by_http(s);
        } else {
          mcs_tcp_callback(rcv_buf);
        }

        count ++;
    }

idle:
    LOG_I(common, "MCS tcp-client end");
    return MCS_TCP_DISCONNECT;
}