# Embed the server root certificate into the final binary
#
# (If this was a component, we would set COMPONENT_EMBED_TXTFILES here.)
set(requires "")
idf_build_get_property(target IDF_TARGET)

list(APPEND requires
    nvs_flash
    esp_netif
    esp_http_client
    esp_http_server
    esp_event
    esp_wifi
    esp_system
    driver
    protocol_examples_common
    esp-tls
    # This
    ESP32_MOD
)

file(GLOB_RECURSE app_sources "Src/*.c")

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "Inc"
    REQUIRES ${requires}
    # EMBED_TXTFILES "Src/example/howsmyssl_com_root_cert.pem"
    # "Src/example/postman_root_cert.pem"
)
