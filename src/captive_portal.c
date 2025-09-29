/**
 * @file captive_portal.c
 * @brief Implementação do Captive Portal
 */

#include "captive_portal.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "CAPTIVE_PORTAL";

// Configurações do Captive Portal
static char s_domain[64] = CAPTIVE_PORTAL_DOMAIN;
static char s_redirect_url[128] = CAPTIVE_PORTAL_REDIRECT_URL;
static bool s_enabled = false;
static bool s_active = false;

// Handlers HTTP para Captive Portal
static const httpd_uri_t captive_dns_uri = {
    .uri       = "/generate_204",
    .method    = HTTP_GET,
    .handler   = captive_portal_dns_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t captive_redirect_uri = {
    .uri       = "/hotspot-detect.html",
    .method    = HTTP_GET,
    .handler   = captive_portal_redirect_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t captive_connectivity_uri = {
    .uri       = "/connectivity-check.html",
    .method    = HTTP_GET,
    .handler   = captive_portal_connectivity_test_handler,
    .user_ctx  = NULL
};

esp_err_t init_captive_portal_service(void)
{
    ESP_LOGI(TAG, "Inicializando Captive Portal");
    
    s_enabled = true;
    s_active = true;
    
    ESP_LOGI(TAG, "Captive Portal inicializado");
    ESP_LOGI(TAG, "Domínio: %s", s_domain);
    ESP_LOGI(TAG, "URL de redirecionamento: %s", s_redirect_url);
    
    return ESP_OK;
}

esp_err_t stop_captive_portal_service(void)
{
    ESP_LOGI(TAG, "Parando Captive Portal");
    
    s_active = false;
    
    ESP_LOGI(TAG, "Captive Portal parado");
    return ESP_OK;
}

bool is_captive_portal_active(void)
{
    return s_active && s_enabled;
}

esp_err_t set_captive_portal_domain(const char *domain)
{
    if (!domain) {
        return ESP_ERR_INVALID_ARG;
    }
    
    strncpy(s_domain, domain, sizeof(s_domain) - 1);
    s_domain[sizeof(s_domain) - 1] = '\0';
    
    ESP_LOGI(TAG, "Domínio do Captive Portal alterado para: %s", s_domain);
    return ESP_OK;
}

esp_err_t set_captive_portal_redirect_url(const char *url)
{
    if (!url) {
        return ESP_ERR_INVALID_ARG;
    }
    
    strncpy(s_redirect_url, url, sizeof(s_redirect_url) - 1);
    s_redirect_url[sizeof(s_redirect_url) - 1] = '\0';
    
    ESP_LOGI(TAG, "URL de redirecionamento alterada para: %s", s_redirect_url);
    return ESP_OK;
}

esp_err_t captive_portal_dns_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Requisição DNS do Captive Portal");
    
    // Redirecionar para página principal
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", s_redirect_url);
    httpd_resp_send(req, NULL, 0);
    
    return ESP_OK;
}

esp_err_t captive_portal_redirect_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Requisição de redirecionamento do Captive Portal");
    
    const char *redirect_page = get_captive_portal_redirect_page();
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, redirect_page, HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

bool is_captive_portal_request(httpd_req_t *req)
{
    if (!req || !s_enabled) {
        return false;
    }
    
    const char *uri = req->uri;
    char host[64];
    size_t host_len = sizeof(host);
    esp_err_t ret = httpd_req_get_hdr_value_str(req, "Host", host, host_len);
    const char *host_ptr = (ret == ESP_OK) ? host : NULL;
    
    // Verificar se é uma requisição típica de Captive Portal
    if (strstr(uri, "generate_204") || 
        strstr(uri, "hotspot-detect") ||
        strstr(uri, "connectivity-check") ||
        strstr(uri, "ncsi.txt")) {
        return true;
    }
    
    // Verificar se o Host contém o domínio do Captive Portal
    if (host_ptr && strstr(host_ptr, s_domain)) {
        return true;
    }
    
    return false;
}

const char* get_captive_portal_redirect_page(void)
{
    return "<!DOCTYPE html>"
           "<html><head>"
           "<title>Redirecionamento - ESP32-C6</title>"
           "<meta http-equiv='refresh' content='0;url=" CAPTIVE_PORTAL_REDIRECT_URL "'>"
           "<script>"
           "window.location.href = '" CAPTIVE_PORTAL_REDIRECT_URL "';"
           "</script>"
           "</head><body>"
           "<p>Redirecionando...</p>"
           "</body></html>";
}

const char* get_captive_portal_success_page(void)
{
    return "<!DOCTYPE html>"
           "<html><head>"
           "<title>Conectado - ESP32-C6</title>"
           "<meta name='viewport' content='width=device-width, initial-scale=1'>"
           "<style>"
           "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }"
           ".container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
           "h1 { color: #27ae60; text-align: center; }"
           ".success { background: #d4edda; color: #155724; padding: 15px; border-radius: 5px; margin: 20px 0; }"
           ".button { display: inline-block; padding: 10px 20px; margin: 10px 5px; background: #3498db; color: white; text-decoration: none; border-radius: 5px; text-align: center; }"
           "</style></head>"
           "<body>"
           "<div class='container'>"
           "<h1>✓ Conectado com Sucesso</h1>"
           "<div class='success'>"
           "<p>Seu dispositivo foi conectado ao ESP32-C6 com sucesso!</p>"
           "<p>Você pode agora acessar a interface de configuração.</p>"
           "</div>"
           "<a href='" CAPTIVE_PORTAL_REDIRECT_URL "' class='button'>Continuar</a>"
           "</div>"
           "</body></html>";
}

esp_err_t register_captive_portal_handlers(httpd_handle_t server)
{
    if (!server) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Registrando handlers do Captive Portal");
    
    // Registrar handlers
    httpd_register_uri_handler(server, &captive_dns_uri);
    httpd_register_uri_handler(server, &captive_redirect_uri);
    httpd_register_uri_handler(server, &captive_connectivity_uri);
    
    ESP_LOGI(TAG, "Handlers do Captive Portal registrados");
    return ESP_OK;
}

esp_err_t captive_portal_connectivity_test_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Teste de conectividade do Captive Portal");
    
    // Resposta simples para teste de conectividade
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OK", 2);
    
    return ESP_OK;
}

const char* get_captive_portal_status(void)
{
    static char json_buffer[256];
    cJSON *json = cJSON_CreateObject();
    
    cJSON_AddBoolToObject(json, "enabled", s_enabled);
    cJSON_AddBoolToObject(json, "active", s_active);
    cJSON_AddStringToObject(json, "domain", s_domain);
    cJSON_AddStringToObject(json, "redirect_url", s_redirect_url);
    
    char *json_string = cJSON_Print(json);
    strncpy(json_buffer, json_string, sizeof(json_buffer) - 1);
    json_buffer[sizeof(json_buffer) - 1] = '\0';
    
    free(json_string);
    cJSON_Delete(json);
    
    return json_buffer;
}

esp_err_t set_captive_portal_enabled(bool enable)
{
    s_enabled = enable;
    
    if (enable) {
        ESP_LOGI(TAG, "Captive Portal habilitado");
    } else {
        ESP_LOGI(TAG, "Captive Portal desabilitado");
    }
    
    return ESP_OK;
}

bool is_captive_portal_enabled(void)
{
    return s_enabled;
}
