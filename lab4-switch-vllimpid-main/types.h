#ifndef COMPNET_LAB4_TYPES_H
#define COMPNET_LAB4_TYPES_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>

typedef uint8_t mac_addr_t[6];
typedef std::chrono::time_point<std::chrono::steady_clock> timestamp_t;

#define ETH_ALEN 6  // length of mac address

const uint16_t ETHER_DATA_TYPE = 0;//数据帧，其后面的payload包含host之间的通讯数据
const uint16_t ETHER_CONTROL_TYPE = 1;//此帧仅会由Controller发出，这表明此帧是一个控制帧

// MAC Table aging.
const uint32_t ETHER_COMMAND_TYPE_AGING = 0;
const int32_t ETHER_MAC_AGING_THRESHOLD = 10;

#pragma pack(2)
typedef struct {
  mac_addr_t ether_dest;//帧的目的MAC地址
  mac_addr_t ether_src;//帧的源MAC地址
  uint16_t ether_type;//帧的类型
  uint16_t length;//length of payload
} ether_header_t;
#pragma pack()

#endif  // ! COMPNET_LAB4_TYPES_H
