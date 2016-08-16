#define _MCS_H_

#define MCS_TCP_INIT_ERROR -1
#define MCS_TCP_SOCKET_INIT_ERROR 0x1
#define MCS_TCP_DISCONNECT 0x2

typedef void (*mcs_tcp_callback_t)(char *);
typedef void (*mcs_mqtt_callback_t)(char *);

void mcs_upload_datapoint(char *);
int32_t mcs_tcp_init(void (*mcs_tcp_callback)(char *));
void mcs_mqtt_init(void (*mcs_mqtt_callback)(char *));
