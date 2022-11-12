#include "jimlib.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_wifi_internal.h>


JStuff j;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

CLI_VARIABLE_INT(pwm, 30);
int pktCount = 0;
struct {
	int x = 0xdddddddd;
//	int x2 = 0xeeeeeeee;
} myData;

void OnDataRecv(const uint8_t * mac, const uint8_t *in, int len) {
	//OUT("onDataRecv(%d) : [%02x]", len, in[0]);	
	//j.led.setPattern(200, 3, 1.0, 1);
	pktCount++; 

	esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}



esp_now_peer_info_t peerInfo;

#define TRY_ESP_ACTION(action, name) if(action == ESP_OK) {Serial.println("\t+ "+String(name));} else {Serial.println("----------Error while " + String(name) + " !---------------");}
#define CHANNEL 6
#define DATARATE WIFI_PHY_RATE_24M

void setup() {
	j.jw.enabled = false;
	j.cliEcho = false;
	j.mqtt.active = false;
	j.onConn = []{};
	j.cli.on("BLINK", [](){ j.led.setPattern(200, 3, 1.0, 1); });
	j.begin();

	WiFi.mode(WIFI_STA);
	TRY_ESP_ACTION( esp_wifi_stop(), "stop WIFI");  
	TRY_ESP_ACTION( esp_wifi_deinit(), "De init");
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_tx_enable = 0;
	TRY_ESP_ACTION( esp_wifi_init(&my_config), "Disable AMPDU");
	TRY_ESP_ACTION( esp_wifi_start(), "Restart WiFi");
	TRY_ESP_ACTION( esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE), "Set channel");
	TRY_ESP_ACTION( esp_wifi_internal_set_fix_rate(ESP_IF_WIFI_STA, true, DATARATE), "Fixed rate set up");
	TRY_ESP_ACTION( esp_now_init(), "ESPNow Init");
	TRY_ESP_ACTION(  esp_now_register_send_cb(OnDataSent), "Attach send callback");
	TRY_ESP_ACTION( esp_now_register_recv_cb(OnDataRecv), "Attach recv callback");
  
	memcpy(peerInfo.peer_addr, broadcastAddress, 6);
	peerInfo.channel = CHANNEL;  
	peerInfo.encrypt = false;

	if (esp_now_add_peer(&peerInfo) != ESP_OK){
		Serial.println("Failed to add peer");
	}

	for (int n = 0; n < 5; n++) {
		esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
	}
}
void loop() {
	j.run();
	if (j.hz(5)) { 
		OUT("%06.1f %d", millis() / 1000.0, pktCount);
	}
	
	struct {
		int x = 0xdddddddd;
	//	int x2 = 0xeeeeeeee;
	} myData;

	if (false && j.hz(1)) { 
		esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
		
		if (result == ESP_OK) {
			Serial.println("Sent with success");
		}
		else {
			Serial.println("Error sending the data");
		}
	}
	delayMicroseconds(10);
}

