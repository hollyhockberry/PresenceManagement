// Copyright (c) 2021 Inaba

#ifndef COMMAND_H_
#define COMMAND_H_

#include <stddef.h>
#include "beaconinfo.h"

typedef void (* cb_beacon_info_func)(const BEACON_INFO& info);
typedef void (* cb_time_func)(time_t info);

class Command {
  const uint8_t* peer_addr;
  cb_beacon_info_func cb_beacon_info;
  cb_time_func cb_time;
 public:
  explicit Command(const uint8_t* peer_addr);

  void Send(const BEACON_INFO& info) const;
  void Send(time_t time) const;

  void Receive(const uint8_t* data, int len);
  void SetCallback(cb_beacon_info_func func) {
    cb_beacon_info = func;
  }
  void SetCallback(cb_time_func func) {
    cb_time = func;
  }
};

#endif  // COMMAND_H_
