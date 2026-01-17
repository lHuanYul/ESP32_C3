#include "main/mod_cfg.h"
#include "HY_MOD/http/receive.h"

SpiParametar spi2_h = {
    .const_h = {
        .SPIx_HOST = SPI2_HOST,
        .SCK = 6,
        .MISO = 2,
        .MOSI = 7,
        .NSS = 10,
    },
};

HttpParametar http_h = {
    .config_server = HTTPD_DEFAULT_CONFIG(),
    .rx_buf = {
        .max_len = JSON_PKT_LEN
    },
    .rx_recv = {
        .uri = "/recv",
        .method = HTTP_POST,
        .handler = http_recv_handler,
        .user_ctx = &http_h,
    },
    .tx_buf = {
        .max_len = JSON_PKT_LEN
    },
    .config_client = {
        .url = "http://192.168.0.21/set-temperature",
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    },
};

JsonPktPool json_pkt_pool;
static JsonPkt* recv_pkts[JSON_RECV_BUF_CAP];
JsonPktBuf spi_recv_buf = {
    .buf = recv_pkts,
    .cap = JSON_RECV_BUF_CAP,
};

static JsonPkt* trsm_pkts[JSON_TRSM_BUF_CAP];
JsonPktBuf spi_trsm_buf = {
    .buf = trsm_pkts,
    .cap = JSON_TRSM_BUF_CAP,
};
