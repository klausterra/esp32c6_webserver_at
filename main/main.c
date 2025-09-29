/**
 * @file main.c
 * @brief ESP32-C6 Web Server AT Examples - Arquivo Principal
 * 
 * Este arquivo implementa as funcionalidades principais do Web Server AT
 * baseado na documentação oficial da Espressif para ESP32-C6.
 * 
 * Funcionalidades:
 * - Wi-Fi Provisioning via Browser
 * - OTA Firmware Upgrade
 * - Captive Portal
 * - WeChat Applet Support
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
// #include "esp_captive_portal.h" // Removido - não existe no ESP-IDF

#include "wifi_manager.h"
#include "web_server.h"
#include "ota_handler.h"
#include "captive_portal.h"

static const char *TAG = "WEBSERVER_AT";

// Event group para sincronização
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Configurações padrão
#define DEFAULT_AP_SSID     "pos_softap"
#define DEFAULT_AP_PASSWORD "espressif"
#define DEFAULT_AP_CHANNEL  11
#define DEFAULT_AP_MAX_CONN 3
#define DEFAULT_WEB_PORT    80
#define DEFAULT_WEB_TIMEOUT 25

// Estrutura de configuração global
typedef struct {
    char ap_ssid[32];
    char ap_password[64];
    uint8_t ap_channel;
    uint8_t ap_max_conn;
    uint16_t web_port;
    uint16_t web_timeout;
    bool captive_portal_enabled;
    bool wechat_support_enabled;
} webserver_config_t;

static webserver_config_t g_config = {
    .ap_ssid = DEFAULT_AP_SSID,
    .ap_password = DEFAULT_AP_PASSWORD,
    .ap_channel = DEFAULT_AP_CHANNEL,
    .ap_max_conn = DEFAULT_AP_MAX_CONN,
    .web_port = DEFAULT_WEB_PORT,
    .web_timeout = DEFAULT_WEB_TIMEOUT,
    .captive_portal_enabled = false,
    .wechat_support_enabled = false
};

/**
 * @brief Handler de eventos Wi-Fi
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START) {
        ESP_LOGI(TAG, "SoftAP iniciado com sucesso");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STOP) {
        ESP_LOGI(TAG, "SoftAP parado");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        ESP_LOGI(TAG, "Dispositivo conectado ao SoftAP");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        ESP_LOGI(TAG, "Dispositivo desconectado do SoftAP");
    }
}

/**
 * @brief Inicializar NVS
 */
static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

/**
 * @brief Configurar SoftAP
 */
static esp_err_t setup_softap(void)
{
    ESP_LOGI(TAG, "Configurando SoftAP: %s", g_config.ap_ssid);
    
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(g_config.ap_ssid),
            .channel = g_config.ap_channel,
            .max_connection = g_config.ap_max_conn,
            .authmode = strlen(g_config.ap_password) > 0 ? WIFI_AUTH_WPA_WPA2_PSK : WIFI_AUTH_OPEN
        },
    };
    
    strcpy((char*)wifi_config.ap.ssid, g_config.ap_ssid);
    if (strlen(g_config.ap_password) > 0) {
        strcpy((char*)wifi_config.ap.password, g_config.ap_password);
    }
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    return ESP_OK;
}

/**
 * @brief Inicializar servidor web
 */
static esp_err_t init_web_server(void)
{
    ESP_LOGI(TAG, "Inicializando servidor web na porta %d", g_config.web_port);
    
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = g_config.web_port;
    config.max_open_sockets = 7;
    config.max_resp_headers = 8;
    config.max_uri_handlers = 8;
    config.recv_wait_timeout = g_config.web_timeout;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registrar handlers
        register_web_handlers(server);
        ESP_LOGI(TAG, "Servidor web iniciado com sucesso");
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "Erro ao iniciar servidor web");
    return ESP_FAIL;
}

/**
 * @brief Inicializar Captive Portal
 */
static esp_err_t init_captive_portal(void)
{
    if (!g_config.captive_portal_enabled) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Inicializando Captive Portal");
    return init_captive_portal_service();
}

/**
 * @brief Processar comandos AT
 */
static void process_at_commands(void)
{
    // Esta função seria implementada para processar comandos AT
    // Por enquanto, apenas um placeholder
    ESP_LOGI(TAG, "Sistema pronto para comandos AT");
}

/**
 * @brief Task principal do sistema
 */
static void webserver_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Iniciando task do webserver");
    
    while (1) {
        // Processar comandos AT
        process_at_commands();
        
        // Verificar status do Wi-Fi
        EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "Wi-Fi conectado");
        } else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(TAG, "Falha na conexão Wi-Fi");
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Função principal
 */
void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP32-C6 Web Server AT Examples ===");
    ESP_LOGI(TAG, "Versão: 1.0.0");
    ESP_LOGI(TAG, "Build: %s %s", __DATE__, __TIME__);
    
    // Inicializar NVS
    ESP_ERROR_CHECK(init_nvs());
    
    // Inicializar esp_netif (necessário para LWIP)
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Inicializar sistema de eventos
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Criar event group
    s_wifi_event_group = xEventGroupCreate();
    
    // Registrar handlers de eventos
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    
    // Inicializar Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    
    // Criar interface de rede padrão para Wi-Fi (APÓS inicializar Wi-Fi)
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();
    
    // Configurar SoftAP
    ESP_ERROR_CHECK(setup_softap());
    
    // Aguardar Wi-Fi estar pronto antes de iniciar servidor web
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "SoftAP configurado, iniciando servidor web");
    
    // Aguardar mais um pouco para garantir que o LWIP está pronto
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Inicializar servidor web
    ESP_ERROR_CHECK(web_server_init());
    
    // Inicializar Captive Portal (se habilitado)
    ESP_ERROR_CHECK(init_captive_portal());
    
    // Inicializar OTA handler
    ESP_ERROR_CHECK(init_ota_handler());
    
    // Criar task principal
    xTaskCreate(webserver_task, "webserver_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Sistema inicializado com sucesso!");
    ESP_LOGI(TAG, "SoftAP: %s", g_config.ap_ssid);
    ESP_LOGI(TAG, "Web Server: http://192.168.4.1:%d", g_config.web_port);
    
    if (g_config.captive_portal_enabled) {
        ESP_LOGI(TAG, "Captive Portal habilitado");
    }
}
