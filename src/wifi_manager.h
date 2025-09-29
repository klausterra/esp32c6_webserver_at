/**
 * @file wifi_manager.h
 * @brief Gerenciador de Wi-Fi para ESP32-C6 Web Server AT
 * 
 * Este módulo implementa as funcionalidades de gerenciamento de Wi-Fi
 * incluindo configuração de SoftAP, conexão STA e provisioning.
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

// Usar as estruturas padrão do ESP-IDF
// wifi_ap_config_t e wifi_sta_config_t já estão definidas em esp_wifi_types.h

// Estrutura para informações de rede
typedef struct {
    char ssid[32];
    int8_t rssi;
    wifi_auth_mode_t auth_mode;
    uint8_t channel;
} wifi_scan_result_t;

// Callbacks
typedef void (*wifi_connected_cb_t)(void);
typedef void (*wifi_disconnected_cb_t)(void);
typedef void (*wifi_scan_done_cb_t)(wifi_scan_result_t *results, uint16_t count);

/**
 * @brief Inicializar gerenciador de Wi-Fi
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Configurar SoftAP
 * 
 * @param config Configuração do SoftAP
 * @return esp_err_t 
 */
esp_err_t wifi_manager_set_ap_config(const wifi_ap_config_t *config);

/**
 * @brief Configurar STA
 * 
 * @param config Configuração do STA
 * @return esp_err_t 
 */
esp_err_t wifi_manager_set_sta_config(const wifi_sta_config_t *config);

/**
 * @brief Iniciar SoftAP
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_start_ap(void);

/**
 * @brief Conectar STA
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_connect_sta(void);

/**
 * @brief Desconectar STA
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_disconnect_sta(void);

/**
 * @brief Escanear redes disponíveis
 * 
 * @param results Array para armazenar resultados
 * @param max_results Tamanho máximo do array
 * @param count Ponteiro para número de resultados encontrados
 * @return esp_err_t 
 */
esp_err_t wifi_manager_scan(wifi_scan_result_t *results, uint16_t max_results, uint16_t *count);

/**
 * @brief Obter status da conexão
 * 
 * @return true se conectado
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Obter IP do STA
 * 
 * @param ip Ponteiro para armazenar IP
 * @return esp_err_t 
 */
esp_err_t wifi_manager_get_sta_ip(esp_ip4_addr_t *ip);

/**
 * @brief Obter IP do SoftAP
 * 
 * @param ip Ponteiro para armazenar IP
 * @return esp_err_t 
 */
esp_err_t wifi_manager_get_ap_ip(esp_ip4_addr_t *ip);

/**
 * @brief Registrar callback de conexão
 * 
 * @param cb Callback
 */
void wifi_manager_register_connected_cb(wifi_connected_cb_t cb);

/**
 * @brief Registrar callback de desconexão
 * 
 * @param cb Callback
 */
void wifi_manager_register_disconnected_cb(wifi_disconnected_cb_t cb);

/**
 * @brief Registrar callback de scan
 * 
 * @param cb Callback
 */
void wifi_manager_register_scan_done_cb(wifi_scan_done_cb_t cb);

/**
 * @brief Limpar configurações salvas
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_clear_saved_config(void);

/**
 * @brief Salvar configuração atual
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_save_config(void);

/**
 * @brief Carregar configuração salva
 * 
 * @return esp_err_t 
 */
esp_err_t wifi_manager_load_config(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H
