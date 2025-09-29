/**
 * @file web_server.h
 * @brief Servidor web para ESP32-C6 Web Server AT
 * 
 * Este módulo implementa o servidor web HTTP para configuração
 * de Wi-Fi, OTA upgrade e outras funcionalidades.
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

// Estrutura para dados de configuração Wi-Fi
typedef struct {
    char ssid[32];
    char password[64];
    bool auto_connect;
} wifi_config_data_t;

// Estrutura para dados OTA
typedef struct {
    char partition[16];
    size_t file_size;
    char file_name[64];
} ota_config_data_t;

// Estrutura para resposta do servidor
typedef struct {
    int code;
    char message[128];
    bool success;
} server_response_t;

/**
 * @brief Inicializar servidor web
 * 
 * @return esp_err_t 
 */
esp_err_t web_server_init(void);

/**
 * @brief Registrar handlers HTTP
 * 
 * @param server Handle do servidor HTTP
 * @return esp_err_t 
 */
esp_err_t register_web_handlers(httpd_handle_t server);

/**
 * @brief Obter página principal
 * 
 * @return const char* HTML da página principal
 */
const char* get_main_page(void);

/**
 * @brief Obter página de dashboard
 * 
 * @return const char* HTML da página de dashboard
 */
const char* get_dashboard_page(void);

/**
 * @brief Obter página de configuração Wi-Fi
 * 
 * @return const char* HTML da página de configuração
 */
const char* get_wifi_config_page(void);

/**
 * @brief Obter página OTA
 * 
 * @return const char* HTML da página OTA
 */
const char* get_ota_page(void);

/**
 * @brief Processar configuração Wi-Fi
 * 
 * @param data Dados de configuração
 * @return server_response_t Resposta do servidor
 */
server_response_t process_wifi_config(const wifi_config_data_t *data);

/**
 * @brief Processar upgrade OTA
 * 
 * @param data Dados OTA
 * @param file_data Dados do arquivo
 * @param file_size Tamanho do arquivo
 * @return server_response_t Resposta do servidor
 */
server_response_t process_ota_upgrade(const ota_config_data_t *data, 
                                     const uint8_t *file_data, 
                                     size_t file_size);

/**
 * @brief Obter lista de redes Wi-Fi
 * 
 * @return const char* JSON com lista de redes
 */
const char* get_wifi_scan_results(void);

/**
 * @brief Obter status do sistema
 * 
 * @return const char* JSON com status
 */
const char* get_system_status(void);

/**
 * @brief Obter informações de firmware
 * 
 * @return const char* JSON com informações de firmware
 */
const char* get_firmware_info(void);

/**
 * @brief Obter lista de partições OTA
 * 
 * @return const char* JSON com lista de partições
 */
const char* get_ota_partitions(void);

/**
 * @brief Handler para página principal
 */
esp_err_t root_get_handler(httpd_req_t *req);

/**
 * @brief Handler para dashboard
 */
esp_err_t dashboard_get_handler(httpd_req_t *req);

/**
 * @brief Handler para configuração Wi-Fi
 */
esp_err_t wifi_config_get_handler(httpd_req_t *req);
esp_err_t wifi_config_post_handler(httpd_req_t *req);

/**
 * @brief Handler para OTA
 */
esp_err_t ota_get_handler(httpd_req_t *req);
esp_err_t ota_post_handler(httpd_req_t *req);

/**
 * @brief Handler para API de scan Wi-Fi
 */
esp_err_t wifi_scan_api_handler(httpd_req_t *req);

/**
 * @brief Handler para API de status
 */
esp_err_t status_api_handler(httpd_req_t *req);

/**
 * @brief Handler para API de firmware
 */
esp_err_t firmware_api_handler(httpd_req_t *req);

/**
 * @brief Handler para API de partições OTA
 */
esp_err_t ota_partitions_api_handler(httpd_req_t *req);

/**
 * @brief Handler para arquivos estáticos
 */
esp_err_t static_file_handler(httpd_req_t *req);

/**
 * @brief Handler para WeChat applet
 */
esp_err_t wechat_handler(httpd_req_t *req);

#ifdef __cplusplus
}
#endif

#endif // WEB_SERVER_H
