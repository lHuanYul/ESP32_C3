#include "main/main.h"
#include "main/mod_cfg.h"
#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/wifi/main.h"
#include "HY_MOD/http/main.h"
#include "HY_MOD/spi/main.h"

void StartHttpTask(void *argument)
{
    my_wifi_connect();
    vTaskDelay(pdMS_TO_TICKS(1000));
    HttpParametar *http = &http_h;
    RESULT_CHECK_HANDLE(http_start_server(http));
    while (1)
    {
        // http_send();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}

void StartSpiTask(void *argument)
{
    static const char *TAG = "MY_SPI_TASK";
    json_pkt_pool_init(&json_pkt_pool);
    SpiParametar *spi = &spi2_h;
    spi_init(spi);

    const char json_response[] = "{\"RC\":\"SC\"}";
    JsonPkt *tx_normal = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
    json_pkt_set_len(tx_normal, (uint16_t)strlen(json_response));
    memcpy(tx_normal->data, json_response, tx_normal->len);
    RESULT_CHECK_HANDLE(json_pkt_buf_push(&spi_trsm_buf, tx_normal, &json_pkt_pool, 0));

    ESP_LOGI(TAG, "SPI Slave Initialized");
    JsonPkt *tx_pkt = NULL;
    while (1)
    {
        switch (spi2_h.state)
        {
            case SPI_STATE_ERROR:
            {
                // ESP_LOGE(TAG, "SPI_STATE_ERROR: %02x %02x %02x %02x", 
                //     spi->rx_buf[0], spi->rx_buf[1], spi->rx_buf[2], spi->rx_buf[3]);
                ESP_LOGE(TAG, "Error");
                spi2_h.state = SPI_STATE_FINISH;
                break;
            }
            case SPI_STATE_FINISH:
            {
                spi_recv_start(spi, sizeof(SPI_MASTER_ASK), spi->rx_buf, 10000);
                spi2_h.state = SPI_STATE_RECV_HEADER;
                break;
            }
            case SPI_STATE_RECV_HEADER:
            {
                if (memcmp(spi->rx_buf, SPI_MASTER_ASK, sizeof(SPI_MASTER_ASK)) == 0) 
                {
                    spi2_h.state = SPI_STATE_TRSM_HEADER;
                    ESP_LOGI(TAG, "Recv R");
                }
                else if (
                    spi->rx_buf[0] == '$' &&
                    spi->rx_buf[1] == 'L' &&
                    spi->rx_buf[2] == ':' &&
                    spi->rx_buf[spi->rx_buf_len - 1] == '\0'
                ) {
                    ESP_LOGI(TAG, "Recv H");
                    spi2_h.state = SPI_STATE_RECV_BODY;
                    uint16_t payload_len = var_u8_to_u16_be(spi->rx_buf + 3);
                    if (payload_len > JSON_PKT_LEN) goto error;
                    spi->rx_buf_len = payload_len;
                    spi_recv_start(spi, spi->rx_buf_len, spi->rx_buf, 500);
                }
                else goto error;
                break;
            }
            case SPI_STATE_TRSM_HEADER:
            {
                Result res = json_pkt_buf_get(&spi_trsm_buf);
                if (RESULT_CHECK_RAW(res))
                {
                    spi->tx_buf_len = sizeof(SPI_SLAVE_EMP);
                    memcpy(spi->tx_buf, SPI_SLAVE_EMP, spi->tx_buf_len);
                    spi_trsm_start(spi, spi->tx_buf_len, spi->tx_buf, 500);
                    ESP_LOGI(TAG, "Send E");
                    spi2_h.state = SPI_STATE_FINISH;
                }
                else
                {
                    tx_pkt = RESULT_UNWRAP_HANDLE(res);
                    spi->tx_buf_len = sizeof(SPI_LENGTH_H);
                    memcpy(spi->tx_buf, SPI_LENGTH_H, spi->tx_buf_len);
                    var_u16_to_u8_be(tx_pkt->len, (spi->tx_buf + 3));
                    spi_trsm_start(spi, spi->tx_buf_len, spi->tx_buf, 500);
                    ESP_LOGI(TAG, "Send H: pl= %d, sl= %d", tx_pkt->len, spi->tx_buf_len);
                    // ESP_LOGI(TAG, "Send H");
                    spi2_h.state = SPI_STATE_TRSM_BODY;
                }
                break;
            }
            case SPI_STATE_TRSM_BODY:
            {
                if (tx_pkt == NULL) goto error;
                spi->tx_buf_len = tx_pkt->len;
                memcpy(spi->tx_buf, tx_pkt->data, spi->tx_buf_len);
                spi_trsm_start(spi, spi->tx_buf_len, spi->tx_buf, 500);
                // ESP_LOGI(TAG, "Send B: %.*s", spi->tx_buf_len, spi->tx_buf);
                ESP_LOGI(TAG, "Send B");
                spi2_h.state = SPI_STATE_FINISH;
                tx_pkt = NULL;
                JsonPkt *pkt = RESULT_UNWRAP_HANDLE(json_pkt_buf_pop(&spi_trsm_buf));
                json_pkt_pool_free(&json_pkt_pool, pkt);
                break;
            }
            case SPI_STATE_RECV_BODY:
            {
                if (
                    spi->rx_buf_len > 0      &&
                    spi->rx_buf[0] == '{'    &&
                    spi->rx_buf[spi->rx_buf_len - 1] == '}'
                ) {
                    spi2_h.state = SPI_STATE_FINISH;
                    // ESP_LOGI(TAG, "Recv Body: %.*s", spi->rx_buf_len, spi->rx_buf);
                    ESP_LOGI(TAG, "Recv B");
                    JsonPkt *pkt = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
                    RESULT_CHECK_GOTO(json_pkt_set_len(pkt, spi->rx_buf_len), release);
                    memcpy(pkt->data, spi->rx_buf, spi->rx_buf_len);
                    RESULT_CHECK_GOTO(json_pkt_buf_push(&spi_recv_buf, pkt, &json_pkt_pool, 1), release);
                }
                else goto error;
release:
                break;
            }
        }
        continue;
error:
        spi2_h.state = SPI_STATE_ERROR;
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    xTaskCreate(StartHttpTask, "HTTP_TASK", 4096, NULL, 5, NULL);
    vTaskDelay(pdMS_TO_TICKS(1000));
    xTaskCreate(StartSpiTask, "SPI_Task", 4096, NULL, 10, NULL);
}

void http_recv_register(HttpParametar *http)
{
    httpd_register_uri_handler(http->server, &http->rx_recv);
}

void http_recv_proc(HttpParametar *http)
{
    static const char *TAG = "MY_HTTP_RX";
    ESP_LOGI(TAG, "%.*s", (int)http->rx_buf.len, http->rx_buf.buffer);
    JsonPkt *tx_normal = RESULT_UNWRAP_HANDLE(json_pkt_pool_alloc(&json_pkt_pool));
    json_pkt_set_len(tx_normal, http->rx_buf.len);
    memcpy(tx_normal->data, http->rx_buf.buffer, tx_normal->len);
    RESULT_CHECK_HANDLE(json_pkt_buf_push(&spi_trsm_buf, tx_normal, &json_pkt_pool, 1));
}
