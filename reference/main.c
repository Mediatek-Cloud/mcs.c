#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "os.h"
#include "net_init.h"
#include "network_init.h"
#include "wifi_api.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "ethernetif.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"

/* Basic lib */
#include "mcs.h"
#include "smart_connection.h"

/* pwm module */
#include "hal_pwm.h"

/* HAL_PWM_CLOCK_40MHZ = 4 */
#define mode (4)
#define frequency (400000)

/* gpio(pin 31) == pwm(pin 32) */
#define pwm_pin 32
#define pin 31

/* mcs channel */
#define PWM_CHANNEL "PWM"

void wifi_connect_init(void *args)
{
    LOG_I(common, "enter connect init.");
    uint8_t opmode  = WIFI_MODE_STA_ONLY;
    uint8_t port = WIFI_PORT_STA;

    /* ssid */
    char ssid[15];
    int nvdm_ssid_len = sizeof(ssid);
    nvdm_read_data_item("STA", "Ssid", (uint8_t *)ssid, (uint32_t *)&nvdm_ssid_len);

    /* password */
    char pwd[15];
    int nvdm_pwd_len = sizeof(pwd);
    nvdm_read_data_item("STA", "Password", (uint8_t *)pwd, (uint32_t *)&nvdm_pwd_len);

    // nvram set STA Ssid GSDTest
    // nvram set STA Password itgordon26591
    // nvram show STA password

    wifi_auth_mode_t auth = WIFI_AUTH_MODE_WPA_PSK_WPA2_PSK;
    wifi_encrypt_type_t encrypt = WIFI_ENCRYPT_TYPE_TKIP_AES_MIX;

    uint8_t nv_opmode;

    if (wifi_config_init() == 0) {
        wifi_config_get_opmode(&nv_opmode);
        if (nv_opmode != opmode) {
            wifi_config_set_opmode(opmode);
        }
        wifi_config_set_ssid(port, ssid ,strlen(ssid));
        wifi_config_set_security_mode(port, auth, encrypt);
        wifi_config_set_wpa_psk_key(port, pwd, strlen(pwd));
        wifi_config_reload_setting();

        network_dhcp_start(opmode);
    }
    vTaskDelete(NULL);
}

void tcp_callback(char *rcv_buf) {

    char *arr[5];
    char *del = ",";
    mcs_split(arr, rcv_buf, del);

    if (0 == strncmp(arr[3], PWM_CHANNEL, strlen(PWM_CHANNEL))) {
        printf("value: %d\n", atoi(arr[4]));
        hal_pwm_set_duty_cycle(pwm_pin, atoi(arr[4]));
    }

    printf("rcv_buf: %s\n", rcv_buf);
}

void wifi_connected_task(void *parameter) {
    char data_buf [100] = {0};
    strcat(data_buf, "status");
    strcat(data_buf, ",,connect wifi");
    mcs_upload_datapoint(data_buf);

    mcs_tcp_init(tcp_callback);
    for (;;) {
        ;
    }
}

void wifi_connected_init(const struct netif *netif) {
    xTaskCreate(wifi_connected_task, "wifiConnectedTask", 8048, NULL, 10, NULL);
}

void start_pwm() {
    /* pwm */
    hal_pinmux_set_function(pin, 9);

    uint32_t total_count = 0;

    if (HAL_PWM_STATUS_OK != hal_pwm_init(HAL_PWM_CLOCK_40MHZ)) {
      printf("hal_pwm_init fail");
    }
    if (HAL_PWM_STATUS_OK != hal_pwm_set_frequency(pwm_pin, frequency, &total_count)) {
      printf("hal_pwm_set_frequency fail");
    }
    if (HAL_PWM_STATUS_OK != hal_pwm_set_duty_cycle(pwm_pin, 0)) {
      printf("hal_pwm_set_duty_cycle fail");
    }
    if (HAL_PWM_STATUS_OK != hal_pwm_start(pwm_pin)) {
      printf("hal_pwm_start fail");
    }

}

extern void smart_config_if_enabled();

int main(void)
{
    system_init();
    wifi_register_ip_ready_callback(wifi_connected_init);
    network_init();
    xTaskCreate(wifi_connect_init, "wifiConnect", 1024, NULL, 1, NULL);
    start_pwm();
    smart_config_if_enabled();
    vTaskStartScheduler();
    while (1) {
    }
    return 0;
}


