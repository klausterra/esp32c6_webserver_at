/*
 * Basic Usage Example - ESP32-C6 Web Server AT
 * 
 * Este exemplo demonstra como usar as principais funcionalidades
 * do servidor web ESP32-C6 de forma básica.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "nvs_flash.h"

static const char *TAG = "BASIC_USAGE";

// Configurações básicas
#define WIFI_SSID      "MinhaRede"
#define WIFI_PASSWORD  "minhasenha123"
#define AP_SSID        "ESP32-AP"
#define AP_PASSWORD    "12345678"
#define WEB_PORT       80

/**
 * @brief Handler para página principal
 */
static esp_err_t root_handler(httpd_req_t *req)
{
    const char* html = 
        "<!DOCTYPE html>"
        "<html><head><title>ESP32-C6 Web Server</title></head>"
        "<body>"
        "<h1>ESP32-C6 Web Server</h1>"
        "<p>Servidor web funcionando corretamente!</p>"
        "<p><a href='/status'>Status</a> | <a href='/wifi'>Wi-Fi</a></p>"
        "</body></html>";
    
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/**
 * @brief Handler para status do sistema
 */
static esp_err_t status_handler(httpd_req_t *req)
{
    char response[512];
    snprintf(response, sizeof(response),
        "{\n"
        "  \"status\": \"online\",\n"
        "  \"uptime\": %lu,\n"
        "  \"free_heap\": %lu,\n"
        "  \"wifi_connected\": %s,\n"
        "  \"ap_active\": %s\n"
        "}",
        esp_timer_get_time() / 1000000,
        esp_get_free_heap_size(),
        "true",  // Simplificado para exemplo
        "true"   // Simplificado para exemplo
    );
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/**
 * @brief Configurar SoftAP
 */
static esp_err_t setup_softap(void)
{
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(AP_SSID),
            .channel = 11,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    
    strcpy((char*)wifi_config.ap.ssid, AP_SSID);
    strcpy((char*)wifi_config.ap.password, AP_PASSWORD);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "SoftAP configurado: %s", AP_SSID);
    return ESP_OK;
}

/**
 * @brief Inicializar servidor web
 */
static esp_err_t init_web_server(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = WEB_PORT;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registrar handlers
        httpd_uri_t root_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_handler
        };
        httpd_register_uri_handler(server, &root_uri);
        
        httpd_uri_t status_uri = {
            .uri = "/status",
            .method = HTTP_GET,
            .handler = status_handler
        };
        httpd_register_uri_handler(server, &status_uri);
        
        ESP_LOGI(TAG, "Servidor web iniciado na porta %d", WEB_PORT);
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "Erro ao iniciar servidor web");
    return ESP_FAIL;
}

/**
 * @brief Aplicação principal
 */
void app_main(void)
{
    ESP_LOGI(TAG, "=== Exemplo de Uso Básico ===");
    
    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Inicializar esp_netif
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Inicializar Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    
    // Criar interface de rede
    esp_netif_create_default_wifi_ap();
    
    // Configurar SoftAP
    ESP_ERROR_CHECK(setup_softap());
    
    // Aguardar inicialização
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Inicializar servidor web
    ESP_ERROR_CHECK(init_web_server());
    
    ESP_LOGI(TAG, "Sistema inicializado!");
    ESP_LOGI(TAG, "Conecte-se à rede: %s", AP_SSID);
    ESP_LOGI(TAG, "Acesse: http://192.168.4.1");
    
    // Loop principal
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "Sistema rodando - Free heap: %lu bytes", esp_get_free_heap_size());
    }
}