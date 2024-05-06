#include "WirelessConnectionManager.h"
#include <WiFiClientSecure.h>

const char *ssid;
const char *pw;
bool wifiConnected = false;

void onWiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_CONNECTED:
    wifiConnected = true;
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    wifiConnected = false;
    break;
  default:
    break;
  }
}

void WirelessConnectionManager::initialize()
{
  // wifi credentials
  ssid = "INSERT_YOUR_SSID";
  pw = "INSERT_YOUR_PASSWORD";

  long startTimeWIFIConnect = millis();

  // connect to wifi and wait until connected
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.onEvent(onWiFiEvent);
  WiFi.begin(ssid, pw);
  while (wifiConnected == false)
  {
    delay(500);
  }

  Serial.println("WIFI ready.");
  configTime(0, 0, "pool.ntp.org");
  Serial.println("NTP time configured.");
}

void reconnectToWiFi()
{
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(ssid, pw);
  while (wifiConnected == false)
  {
    delay(500);
  }
}

void WirelessConnectionManager::run()
{
  if (!wifiConnected)
  {
    reconnectToWiFi();
  }
}