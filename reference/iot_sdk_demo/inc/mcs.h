#define _MCS_H_

#define MCS_TCP_INIT_ERROR -1
#define MCS_TCP_SOCKET_INIT_ERROR 0x1
#define MCS_TCP_DISCONNECT 0x2
#define MCS_MAX_STRING_SIZE 200

#define DEVICEID "DU8xrUWV"
#define DEVICEKEY "nE1EFLIlm3TrZg79"
#define HOST "com"

/* if you use MQTT
#define TOPIC "mcs/{Input your deviceId}/{Input your deviceKey}/+"
#define PORT "1883"
#define CLIENTID "mt7687"
*/

typedef void (*mcs_tcp_callback_t)(char *);
typedef void (*mcs_mqtt_callback_t)(char *);

void mcs_upload_datapoint(char *);
int32_t mcs_tcp_init(void (*mcs_tcp_callback)(char *));
void mcs_mqtt_init(void (*mcs_mqtt_callback)(char *));

/* utils */
void mcs_split(char **arr, char *str, const char *del);
void mcs_splitn(char ** dst, char * src, const char * delimiter, uint32_t max_split);
char *mcs_replace(char *st, char *orig, char *repl);
