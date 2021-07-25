// Copyright (c) 2021 Inaba

#include "command.h"
#include <string.h>
#include <esp_now.h>

namespace {
enum {
  BEACON,
  DATETIME,
};

uint8_t send_buffer[250];
}  // namespace

Command::Command(const uint8_t* peer)
: peer_addr(peer), cb_beacon_info(nullptr), cb_time(nullptr) {
}

void Command::Send(const BEACON_INFO& info) const {
  if (peer_addr == nullptr) return;
  *(reinterpret_cast<uint32_t*>(send_buffer)) = BEACON;
  auto len = info.Push(&send_buffer[4]);
  ::esp_now_send(peer_addr, send_buffer, len + sizeof(uint32_t));
}

void Command::Send(time_t time) const {
  if (peer_addr == nullptr) return;
  *(reinterpret_cast<uint32_t*>(send_buffer)) = DATETIME;
  ::memcpy(&send_buffer[4], &time, sizeof(time_t));
  ::esp_now_send(peer_addr, send_buffer, sizeof(time_t) + sizeof(uint32_t));
}

void Command::Receive(const uint8_t* data, int len) {
  if (len < 4) return;
  auto command = *(reinterpret_cast<const uint32_t*>(data));
  data += sizeof(uint32_t);
  len -= sizeof(uint32_t);
  switch (command) {
    case BEACON:
      if (cb_beacon_info != nullptr) {
        BEACON_INFO info;
        if (info.Pop(data, len)) {
          cb_beacon_info(info);
        }
      }
      break;
    case DATETIME:
      if (cb_time != nullptr) {
        auto time = *(reinterpret_cast<const time_t*>(data));
        cb_time(time);
      }
      break;
  }
}
