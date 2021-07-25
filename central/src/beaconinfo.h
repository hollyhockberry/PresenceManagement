// Copyright (c) 2021 Inaba

#ifndef BEACONINFO_H_
#define BEACONINFO_H_

#include "NimBLEUUID.h"

struct BEACON_INFO {
  time_t Time;
  uint16_t Node;
  uint16_t ID;
  uint16_t Major, Minor;
  int8_t TxPower;
  int8_t RSSI;
  NimBLEUUID UUID;

  double Distance() const;
  size_t Push(void* buffer) const;
  bool Pop(const void* buffer, size_t length);
};

#endif  // BEACONINFO_H_
