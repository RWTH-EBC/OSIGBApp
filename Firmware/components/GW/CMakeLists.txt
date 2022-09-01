set(COMPONENT_SRCDIRS "src")
set(COMPONENT_ADD_INCLUDEDIRS "include")

set(COMPONENT_REQUIRES log freertos driver nvs_flash esp_http_server mqtt LTE json bt esp_http_client esp_https_ota app_update)

register_component()
