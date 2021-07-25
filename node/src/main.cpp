// Sketch for node of PresenceManagement
// Work on M5Atom
// Copyright (c) 2021 Inaba

#include <map>
#include <M5Atom.h>
#include <NimBLEDevice.h>
#include <NimBLEBeacon.h>
#include <esp_now.h>
#include <WiFi.h>

#include "beaconinfo.h"
#include "command.h"

namespace {

const int NODE = 0;
const uint8_t peer_addr[] = { 0x0, 0x00, 0x00, 0x00, 0x00, 0x00 };
Command command(peer_addr);

const uint8_t CHANNEL = 0;

const double distanceThresh = 2.0;

const int scanDuration = 5;
const int scanInterval = 20;
NimBLEScan* bleScan;

const std::map<int, NimBLEUUID> UUIDs {
  // mamorio
  { 0, NimBLEUUID("b9407f30-f5f8-466e-aff9-25556b57fe6e") },
};

bool find(BLEAdvertisedDevice* device, BEACON_INFO* ptr) {
  if (!device->haveManufacturerData()) {
    return false;
  }
  uint8_t packet[100];
  auto manufacturerData = device->getManufacturerData();
  manufacturerData.copy(
    reinterpret_cast<char *>(packet), manufacturerData.length(), 0);

  if (manufacturerData.length() != 25 ||
        packet[0] != 0x4C || packet[1] != 0x00) {
    return false;
  }

  auto beacon = NimBLEBeacon();
  beacon.setData(manufacturerData);

  for (auto itr = UUIDs.begin(); itr != UUIDs.end(); ++itr) {
    if (beacon.getProximityUUID() == itr->second) {
#     define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))
      auto t = ::time(nullptr);
      ptr->Time = t;
      ptr->Node = NODE;
      ptr->ID = beacon.getManufacturerId();
      ptr->Major = ENDIAN_CHANGE_U16(beacon.getMajor());
      ptr->Minor = ENDIAN_CHANGE_U16(beacon.getMinor());
      ptr->UUID = beacon.getProximityUUID();
      ptr->TxPower = beacon.getSignalPower();
      if (device->haveRSSI()) {
        ptr->RSSI = device->getRSSI();
      }
      return true;
    }
  }
  return false;
}

void ReceiveBeaconInfo(const BEACON_INFO& info) {
  command.Send(info);
}

void ReceiveTime(time_t time) {
  timeval tv = {
    .tv_sec = mktime(::localtime(&time))
  };
  settimeofday(&tv, nullptr);
}

void onReceived(const uint8_t*, const uint8_t* recvData, int len) {
  command.Receive(recvData, len);
}

}  // namespace

void setup() {
  M5.begin(true, false, true);

  command.SetCallback(ReceiveBeaconInfo);
  command.SetCallback(ReceiveTime);

  WiFi.mode(WIFI_STA);
  Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
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

  NimBLEDevice::init("");
  bleScan = NimBLEDevice::getScan();
  bleScan->setActiveScan(true);
  bleScan->setInterval(100);
  bleScan->setWindow(99);
}

void loop() {
  const auto begin = ::millis();
  BEACON_INFO info;
  auto foundDevices = bleScan->start(scanDuration, false);
  for (int i = 0; i < foundDevices.getCount(); ++i) {
    auto device = foundDevices.getDevice(i);
    if (find(&device, &info) && (info.Distance() <= distanceThresh)) {
      command.Send(info);
      Serial.printf("%d: d:%f\n", i, info.Distance());
    }
  }
  bleScan->clearResults();

  const auto end = ::millis();
  ::delay(scanInterval * 1000 - (end - begin));
}
