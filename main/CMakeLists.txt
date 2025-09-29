idf_component_register(SRCS "main.c"
                                     "../src/wifi_manager.c"
                                     "../src/web_server.c"
                                     "../src/ota_handler.c"
                                     "../src/captive_portal.c"
                            INCLUDE_DIRS "."
                                         "../src"
                            REQUIRES esp_wifi
                                     esp_http_server
                                     esp_https_ota
                                     app_update
                                     esp_netif
                                     nvs_flash
                                     json
                                     esp_timer
                                     esp_system
                                     freertos
                                     esp_partition
                                     esp_app_format)
