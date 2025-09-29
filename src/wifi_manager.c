/**
 * @file wifi_manager.c
 * @brief Implementação do gerenciador de Wi-Fi
 */

#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static const char *TAG = "WIFI_MANAGER";

// Event group para sincronização
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_SCAN_DONE_BIT BIT2

// Configurações atuais
static wifi_ap_config_t s_ap_config = {0};
static wifi_sta_config_t s_sta_config = {0};
static bool s_ap_started = false;
static bool s_sta_connected = false;

// Callbacks
static wifi_connected_cb_t s_connected_cb = NULL;
static wifi_disconnected_cb_t s_disconnected_cb = NULL;
static wifi_scan_done_cb_t s_scan_done_cb = NULL;

// Resultados do scan
static wifi_scan_result_t s_scan_results[20];
static uint16_t s_scan_count = 0;

// NVS namespace
#define NVS_NAMESPACE "wifi_config"

/**
 * @brief Handler de eventos Wi-Fi
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "SoftAP iniciado");
                s_ap_started = true;
                break;
                
            case WIFI_EVENT_AP_STOP:
                ESP_LOGI(TAG, "SoftAP parado");
                s_ap_started = false;
                break;
                
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "STA iniciado");
                esp_wifi_connect();
                break;
                
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "STA conectado ao AP");
                break;
                
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "STA desconectado");
                s_sta_connected = false;
                xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                
                if (s_disconnected_cb) {
                    s_disconnected_cb();
                }
                
                // Tentar reconectar automaticamente
                esp_wifi_connect();
                break;
                
            case WIFI_EVENT_SCAN_DONE:
                ESP_LOGI(TAG, "Scan de Wi-Fi concluído");
                xEventGroupSetBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT);
                break;
                
            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP:
                {
                    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                    ESP_LOGI(TAG, "IP obtida:" IPSTR, IP2STR(&event->ip_info.ip));
                    s_sta_connected = true;
                    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                    
                    if (s_connected_cb) {
                        s_connected_cb();
                    }
                }
                break;
                
            default:
                break;
        }
    }
}

/**
 * @brief Task para processar resultados do scan
 */
static void scan_task(void *pvParameters)
{
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                             WIFI_SCAN_DONE_BIT,
                                             pdTRUE,
                                             pdFALSE,
                                             portMAX_DELAY);
        
        if (bits & WIFI_SCAN_DONE_BIT) {
            uint16_t ap_count = 0;
            esp_wifi_scan_get_ap_num(&ap_count);
            
            if (ap_count > 0) {
                wifi_ap_record_t *ap_records = malloc(ap_count * sizeof(wifi_ap_record_t));
                if (ap_records) {
                    esp_wifi_scan_get_ap_records(&ap_count, ap_records);
                    
                    s_scan_count = (ap_count > 20) ? 20 : ap_count;
                    
                    for (int i = 0; i < s_scan_count; i++) {
                        strncpy(s_scan_results[i].ssid, (char*)ap_records[i].ssid, 32);
                        s_scan_results[i].rssi = ap_records[i].rssi;
                        s_scan_results[i].auth_mode = ap_records[i].authmode;
                        s_scan_results[i].channel = ap_records[i].primary;
                    }
                    
                    free(ap_records);
                    
                    if (s_scan_done_cb) {
                        s_scan_done_cb(s_scan_results, s_scan_count);
                    }
                }
            }
        }
    }
}

esp_err_t wifi_manager_init(void)
{
    ESP_LOGI(TAG, "Inicializando gerenciador de Wi-Fi");
    
    // Criar event group
    s_wifi_event_group = xEventGroupCreate();
    if (!s_wifi_event_group) {
        ESP_LOGE(TAG, "Erro ao criar event group");
        return ESP_ERR_NO_MEM;
    }
    
    // Registrar handlers de eventos
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    // Inicializar Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    
    // Carregar configurações salvas
    wifi_manager_load_config();
    
    // Criar task de scan
    xTaskCreate(scan_task, "scan_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Gerenciador de Wi-Fi inicializado");
    return ESP_OK;
}

esp_err_t wifi_manager_set_ap_config(const wifi_ap_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_ap_config, config, sizeof(wifi_ap_config_t));
    ESP_LOGI(TAG, "Configuração AP atualizada: %s", s_ap_config.ssid);
    
    return ESP_OK;
}

esp_err_t wifi_manager_set_sta_config(const wifi_sta_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_sta_config, config, sizeof(wifi_sta_config_t));
    ESP_LOGI(TAG, "Configuração STA atualizada: %s", s_sta_config.ssid);
    
    return ESP_OK;
}

esp_err_t wifi_manager_start_ap(void)
{
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen((char*)s_ap_config.ssid),
            .channel = s_ap_config.channel,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    
    strcpy((char*)wifi_config.ap.ssid, (char*)s_ap_config.ssid);
    if (strlen((char*)s_ap_config.password) > 0) {
        strcpy((char*)wifi_config.ap.password, (char*)s_ap_config.password);
    }
    
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "SoftAP iniciado: %s", s_ap_config.ssid);
    return ESP_OK;
}

esp_err_t wifi_manager_connect_sta(void)
{
    if (strlen((char*)s_sta_config.ssid) == 0) {
        ESP_LOGE(TAG, "SSID não configurado");
        return ESP_ERR_INVALID_STATE;
    }
    
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    strcpy((char*)wifi_config.sta.ssid, (char*)s_sta_config.ssid);
    strcpy((char*)wifi_config.sta.password, (char*)s_sta_config.password);
    
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    ESP_LOGI(TAG, "Conectando STA a: %s", s_sta_config.ssid);
    return ESP_OK;
}

esp_err_t wifi_manager_disconnect_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    s_sta_connected = false;
    ESP_LOGI(TAG, "STA desconectado");
    return ESP_OK;
}

esp_err_t wifi_manager_scan(wifi_scan_result_t *results, uint16_t max_results, uint16_t *count)
{
    if (!results || !count) {
        return ESP_ERR_INVALID_ARG;
    }
    
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = {
            .active = {
                .min = 100,
                .max = 300
            }
        }
    };
    
    esp_err_t ret = esp_wifi_scan_start(&scan_config, false);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao iniciar scan: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Aguardar conclusão do scan
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_SCAN_DONE_BIT,
                                         pdTRUE,
                                         pdFALSE,
                                         pdMS_TO_TICKS(10000));
    
    if (!(bits & WIFI_SCAN_DONE_BIT)) {
        ESP_LOGE(TAG, "Timeout no scan de Wi-Fi");
        return ESP_ERR_TIMEOUT;
    }
    
    // Copiar resultados
    *count = (s_scan_count > max_results) ? max_results : s_scan_count;
    memcpy(results, s_scan_results, *count * sizeof(wifi_scan_result_t));
    
    ESP_LOGI(TAG, "Scan concluído: %d redes encontradas", *count);
    return ESP_OK;
}

bool wifi_manager_is_connected(void)
{
    return s_sta_connected;
}

esp_err_t wifi_manager_get_sta_ip(esp_ip4_addr_t *ip)
{
    if (!ip) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!s_sta_connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif) {
        return ESP_ERR_NOT_FOUND;
    }
    
    esp_netif_ip_info_t ip_info;
    esp_err_t ret = esp_netif_get_ip_info(netif, &ip_info);
    if (ret == ESP_OK) {
        *ip = ip_info.ip;
    }
    
    return ret;
}

esp_err_t wifi_manager_get_ap_ip(esp_ip4_addr_t *ip)
{
    if (!ip) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!s_ap_started) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (!netif) {
        return ESP_ERR_NOT_FOUND;
    }
    
    esp_netif_ip_info_t ip_info;
    esp_err_t ret = esp_netif_get_ip_info(netif, &ip_info);
    if (ret == ESP_OK) {
        *ip = ip_info.ip;
    }
    
    return ret;
}

void wifi_manager_register_connected_cb(wifi_connected_cb_t cb)
{
    s_connected_cb = cb;
}

void wifi_manager_register_disconnected_cb(wifi_disconnected_cb_t cb)
{
    s_disconnected_cb = cb;
}

void wifi_manager_register_scan_done_cb(wifi_scan_done_cb_t cb)
{
    s_scan_done_cb = cb;
}

esp_err_t wifi_manager_clear_saved_config(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = nvs_erase_all(nvs_handle);
    nvs_close(nvs_handle);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Configurações Wi-Fi limpas");
    }
    
    return ret;
}

esp_err_t wifi_manager_save_config(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Salvar configuração AP
    ret = nvs_set_blob(nvs_handle, "ap_config", &s_ap_config, sizeof(wifi_ap_config_t));
    if (ret == ESP_OK) {
        // Salvar configuração STA
        ret = nvs_set_blob(nvs_handle, "sta_config", &s_sta_config, sizeof(wifi_sta_config_t));
    }
    
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs_handle);
    }
    
    nvs_close(nvs_handle);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Configurações Wi-Fi salvas");
    }
    
    return ret;
}

esp_err_t wifi_manager_load_config(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    
    size_t required_size = sizeof(wifi_ap_config_t);
    ret = nvs_get_blob(nvs_handle, "ap_config", &s_ap_config, &required_size);
    if (ret == ESP_OK) {
        required_size = sizeof(wifi_sta_config_t);
        ret = nvs_get_blob(nvs_handle, "sta_config", &s_sta_config, &required_size);
    }
    
    nvs_close(nvs_handle);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Configurações Wi-Fi carregadas");
    } else {
        ESP_LOGI(TAG, "Nenhuma configuração Wi-Fi salva encontrada");
    }
    
    return ret;
}
