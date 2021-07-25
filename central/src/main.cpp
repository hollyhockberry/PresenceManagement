// Sketch for central-hub of PresenceManagement
// Work on M5Stack
// Copyright (c) 2021 Inaba

#include <M5Stack.h>
#include <esp_now.h>
#include <WiFi.h>

#include "beaconinfo.h"
#include "command.h"

namespace {

const uint8_t CHANNEL = 0;

const char* SSID = "*******";
const char* PSK = "*******";

const char* ntpServer =  "ntp.jst.mfeed.ad.jp";

// broadcast??
const uint8_t peer_addr[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
Command command(peer_addr);

int lastNode = -1;
time_t lasttime = 0;
bool isChanged = false;

void Draw() {
  M5.Lcd.clear();
  for (auto i = 0; i < 3; ++i) {
    M5.Lcd.setCursor(14 + i * 100, 34);
    M5.Lcd.printf("Room %d", i + 1);
    M5.Lcd.drawRect(10 + i * 100, 60, 100, 160, 0xffffff);
  }

  M5.Lcd.setCursor(0, 10);
  if (lastNode >= 0) {
    auto t = ::localtime(&lasttime);
    M5.Lcd.printf("Room: %d %2d/%2d %02d:%02d:%02d",
      lastNode + 1,
      t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    M5.Lcd.fillCircle(60 + lastNode * 100, 100, 20, RED);
  } else {
    M5.Lcd.printf("Not exist..");
  }
}

void onReceived(const uint8_t*, const uint8_t* recvData, int len) {
  command.Receive(recvData, len);
}

void ReceiveBeaconInfo(const BEACON_INFO& info) {
  lastNode = info.Node;
  lasttime = info.Time;
  isChanged = true;
}

void DisconnectWiFi() {
  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(10);
}

void SyncDateTime() {
  DisconnectWiFi();

  WiFi.begin(SSID, PSK);
  while (WiFi.status() != WL_CONNECTED) {
    ::delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");

  ::configTime(9 * 3600, 0, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%Y %m %d %a %H:%M:%S");
}

void StartEspNow() {
  DisconnectWiFi();

  WiFi.mode(WIFI_STA);
  if (::esp_now_init() != ESP_OK) {
    return;
  }
  ::esp_now_register_recv_cb(onReceived);

  esp_now_peer_info_t peerInfo;
  ::memcpy(peerInfo.peer_addr, peer_addr, 6);
  peerInfo.channel = CHANNEL;
  peerInfo.encrypt = false;
  if (::esp_now_add_peer(&peerInfo) != ESP_OK) {
    return;
  }
}

}  // namespace

void setup() {
  M5.begin(true, false);

  command.SetCallback(ReceiveBeaconInfo);

  SyncDateTime();
  StartEspNow();

  M5.Lcd.init();
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);
  Draw();
  Serial.println(WiFi.macAddress().c_str());
}

void loop() {
  M5.update();

  // Press Button A to broadcast datetime to all nodes.
  if (M5.BtnA.wasPressed()) {
    auto time = ::time(nullptr);
    command.Send(time);
  }

  if (lastNode >= 0) {
    // Undetected for a some time.
    auto time = ::time(nullptr);
    if ((time - lasttime) > 30) {
      lastNode = -1;
      isChanged = true;
    }
  }

  if (isChanged) {
    Draw();
    isChanged = false;
  }
}
