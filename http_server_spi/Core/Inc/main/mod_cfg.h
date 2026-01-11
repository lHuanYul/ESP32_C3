#pragma once

#include "main/config.h"
#include "HY_MOD/spi/basic.h"
#include "HY_MOD/http/basic.h"
#include "HY_MOD/packet/json.h"

extern SpiParametar spi2_h;
extern HttpParametar http_h;

extern JsonPktPool json_pkt_pool;
extern JsonPktBuf spi_recv_buf;
extern JsonPktBuf spi_trsm_buf;
