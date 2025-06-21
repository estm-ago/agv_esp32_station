#pragma once

#define WIFI_DHCP "192.168.0.20"
#define WIFI_SSID "HY-TPL-BF94"
#define WIFI_PSWD "23603356"
#define CONNECT_MAXIMUM_RETRY 5

void wifi_setup_sta(void);
void wifi_connect_sta(void);

