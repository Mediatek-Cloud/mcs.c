#ifndef __MQTT_H_
#define __MQTT_H_

#define MQTT_USR_TLS
#define MQTT_USE_CLIENT_CERT

extern int mqtt_client_example(void);
#ifdef MQTT_USR_TLS
extern int mqtt_client_example_ssl(void);
#endif

#endif /* __MQTT_H_ */
