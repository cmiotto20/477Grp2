idf_component_register(SRCS "http_request_example_main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi nvs_flash esp_http_client protocol_examples_common lwip@2.1.2
                    PRIV_REQUIRES esp_event)