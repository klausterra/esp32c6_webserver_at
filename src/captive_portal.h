/**
 * @file captive_portal.h
 * @brief Captive Portal para ESP32-C6 Web Server AT
 * 
 * Este módulo implementa o Captive Portal para redirecionamento
 * automático de dispositivos conectados ao SoftAP.
 */

#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

// Configurações do Captive Portal
#define CAPTIVE_PORTAL_DOMAIN "pos_softap.local"
#define CAPTIVE_PORTAL_REDIRECT_URL "http://192.168.4.1"

/**
 * @brief Inicializar Captive Portal
 * 
 * @return esp_err_t 
 */
esp_err_t init_captive_portal_service(void);

/**
 * @brief Parar Captive Portal
 * 
 * @return esp_err_t 
 */
esp_err_t stop_captive_portal_service(void);

/**
 * @brief Verificar se Captive Portal está ativo
 * 
 * @return true se ativo
 */
bool is_captive_portal_active(void);

/**
 * @brief Configurar domínio do Captive Portal
 * 
 * @param domain Domínio personalizado
 * @return esp_err_t 
 */
esp_err_t set_captive_portal_domain(const char *domain);

/**
 * @brief Configurar URL de redirecionamento
 * 
 * @param url URL de redirecionamento
 * @return esp_err_t 
 */
esp_err_t set_captive_portal_redirect_url(const char *url);

/**
 * @brief Handler para requisições DNS
 * 
 * @param req Requisição HTTP
 * @return esp_err_t 
 */
esp_err_t captive_portal_dns_handler(httpd_req_t *req);

/**
 * @brief Handler para requisições de redirecionamento
 * 
 * @param req Requisição HTTP
 * @return esp_err_t 
 */
esp_err_t captive_portal_redirect_handler(httpd_req_t *req);

/**
 * @brief Verificar se requisição é do Captive Portal
 * 
 * @param req Requisição HTTP
 * @return true se for do Captive Portal
 */
bool is_captive_portal_request(httpd_req_t *req);

/**
 * @brief Obter página de redirecionamento
 * 
 * @return const char* HTML da página de redirecionamento
 */
const char* get_captive_portal_redirect_page(void);

/**
 * @brief Obter página de sucesso do Captive Portal
 * 
 * @return const char* HTML da página de sucesso
 */
const char* get_captive_portal_success_page(void);

/**
 * @brief Registrar handlers do Captive Portal
 * 
 * @param server Handle do servidor HTTP
 * @return esp_err_t 
 */
esp_err_t register_captive_portal_handlers(httpd_handle_t server);

/**
 * @brief Processar requisição de teste de conectividade
 * 
 * @param req Requisição HTTP
 * @return esp_err_t 
 */
esp_err_t captive_portal_connectivity_test_handler(httpd_req_t *req);

/**
 * @brief Obter status do Captive Portal
 * 
 * @return const char* JSON com status
 */
const char* get_captive_portal_status(void);

/**
 * @brief Habilitar/desabilitar Captive Portal
 * 
 * @param enable true para habilitar
 * @return esp_err_t 
 */
esp_err_t set_captive_portal_enabled(bool enable);

/**
 * @brief Verificar se Captive Portal está habilitado
 * 
 * @return true se habilitado
 */
bool is_captive_portal_enabled(void);

#ifdef __cplusplus
}
#endif

#endif // CAPTIVE_PORTAL_H
