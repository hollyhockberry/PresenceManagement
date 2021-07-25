// Copyright (c) 2021 Inaba

#include "beaconinfo.h"
#include <math.h>
#include <stddef.h>
#include <string.h>
#include <string>

double BEACON_INFO::Distance() const {
  auto tx = static_cast<double>(TxPower);
  auto rssi = static_cast<double>(RSSI);
  return ::pow(10.0, (tx - rssi) / 20.0);
}

namespace {

struct HEADER {
  time_t Time;
  uint16_t Node;
  uint16_t ID;
  uint16_t Major, Minor;
  int8_t TxPower;
  int8_t RSSI;
  char uuid[];
};

}  // namespace

size_t BEACON_INFO::Push(void* buffer) const {
  HEADER* ptr = reinterpret_cast<HEADER*>(buffer);
  ptr->Time = Time;
  ptr->Node = Node;
  ptr->ID = ID;
  ptr->Major = Major;
  ptr->Minor = Minor;
  ptr->TxPower = TxPower;
  ptr->RSSI = RSSI;
  auto uuid = UUID.toString();
  ::strcpy(ptr->uuid, uuid.c_str());
  return offsetof(HEADER, uuid) + uuid.length() + 1;
}

bool BEACON_INFO::Pop(const void* buffer, size_t length) {
  if (length < sizeof(HEADER))
    return false;
  const HEADER* ptr = reinterpret_cast<const HEADER*>(buffer);
  Time = ptr->Time;
  Node = ptr->Node;
  ID = ptr->ID;
  Major = ptr->Major;
  Minor = ptr->Minor;
  TxPower = ptr->TxPower;
  RSSI = ptr->RSSI;
  UUID = NimBLEUUID(std::string(ptr->uuid));
  return true;
}
