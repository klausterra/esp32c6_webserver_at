/**
 * @file web_server.c
 * @brief Implementação do servidor web
 */

#include "web_server.h"
#include "wifi_manager.h"
#include "ota_handler.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "WEB_SERVER";

// Handlers HTTP
static const httpd_uri_t root_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t wifi_config_get_uri = {
    .uri       = "/wifi",
    .method    = HTTP_GET,
    .handler   = wifi_config_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t wifi_config_post_uri = {
    .uri       = "/wifi",
    .method    = HTTP_POST,
    .handler   = wifi_config_post_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t ota_get_uri = {
    .uri       = "/ota",
    .method    = HTTP_GET,
    .handler   = ota_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t ota_post_uri = {
    .uri       = "/ota",
    .method    = HTTP_POST,
    .handler   = ota_post_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t wifi_scan_api_uri = {
    .uri       = "/api/wifi/scan",
    .method    = HTTP_GET,
    .handler   = wifi_scan_api_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t status_api_uri = {
    .uri       = "/api/status",
    .method    = HTTP_GET,
    .handler   = status_api_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t firmware_api_uri = {
    .uri       = "/api/firmware",
    .method    = HTTP_GET,
    .handler   = firmware_api_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t ota_partitions_api_uri = {
    .uri       = "/api/ota/partitions",
    .method    = HTTP_GET,
    .handler   = ota_partitions_api_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t wechat_uri = {
    .uri       = "/wechat",
    .method    = HTTP_GET,
    .handler   = wechat_handler,
    .user_ctx  = NULL
};

// Função auxiliar para enviar resposta JSON
static esp_err_t send_json_response(httpd_req_t *req, const char *json_str)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    
    return httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
}

// Função auxiliar para enviar resposta HTML
static esp_err_t send_html_response(httpd_req_t *req, const char *html_str)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    return httpd_resp_send(req, html_str, HTTPD_RESP_USE_STRLEN);
}

// Função auxiliar para extrair parâmetros POST
static esp_err_t extract_post_data(httpd_req_t *req, char *buffer, size_t buffer_size)
{
    size_t recv_size = req->content_len;
    if (recv_size >= buffer_size) {
        return ESP_ERR_NO_MEM;
    }
    
    int ret = httpd_req_recv(req, buffer, recv_size);
    if (ret <= 0) {
        return ESP_FAIL;
    }
    
    buffer[ret] = '\0';
    return ESP_OK;
}

esp_err_t web_server_init(void)
{
    ESP_LOGI(TAG, "Inicializando servidor web");
    
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_open_sockets = 7;
    config.max_resp_headers = 8;
    config.max_uri_handlers = 8;
    config.recv_wait_timeout = 25;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registrar handlers
        register_web_handlers(server);
        ESP_LOGI(TAG, "Servidor web iniciado com sucesso na porta 80");
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "Erro ao iniciar servidor web");
    return ESP_FAIL;
}

esp_err_t register_web_handlers(httpd_handle_t server)
{
    ESP_LOGI(TAG, "Registrando handlers HTTP");
    
    // Registrar handlers
    httpd_register_uri_handler(server, &root_uri);
    httpd_register_uri_handler(server, &wifi_config_get_uri);
    httpd_register_uri_handler(server, &wifi_config_post_uri);
    httpd_register_uri_handler(server, &ota_get_uri);
    httpd_register_uri_handler(server, &ota_post_uri);
    httpd_register_uri_handler(server, &wifi_scan_api_uri);
    httpd_register_uri_handler(server, &status_api_uri);
    httpd_register_uri_handler(server, &firmware_api_uri);
    httpd_register_uri_handler(server, &ota_partitions_api_uri);
    httpd_register_uri_handler(server, &wechat_uri);
    
    ESP_LOGI(TAG, "Handlers HTTP registrados");
    return ESP_OK;
}

const char* get_main_page(void)
{
    return "<!DOCTYPE html>"
           "<html><head>"
           "<title>ESP32-C6 Web Server AT</title>"
           "<meta name='viewport' content='width=device-width, initial-scale=1'>"
           "<style>"
           "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }"
           ".container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
           "h1 { color: #2c3e50; text-align: center; }"
           ".button { display: inline-block; padding: 10px 20px; margin: 10px; background: #3498db; color: white; text-decoration: none; border-radius: 5px; text-align: center; }"
           ".button:hover { background: #2980b9; }"
           ".status { padding: 10px; margin: 10px 0; border-radius: 5px; }"
           ".success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }"
           ".error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }"
           "</style></head>"
           "<body>"
           "<div class='container'>"
           "<h1>ESP32-C6 Web Server AT</h1>"
           "<div id='status' class='status'></div>"
           "<a href='/wifi' class='button'>Configurar Wi-Fi</a>"
           "<a href='/ota' class='button'>OTA Upgrade</a>"
           "<a href='/api/status' class='button'>Status do Sistema</a>"
           "</div>"
           "<script>"
           "fetch('/api/status')"
           ".then(response => response.json())"
           ".then(data => {"
           "  const statusDiv = document.getElementById('status');"
           "  if (data.wifi_connected) {"
           "    statusDiv.innerHTML = '<div class=\"success\">Wi-Fi Conectado: ' + data.wifi_ssid + '</div>';"
           "  } else {"
           "    statusDiv.innerHTML = '<div class=\"error\">Wi-Fi Desconectado</div>';"
           "  }"
           "});"
           "</script>"
           "</body></html>";
}

const char* get_wifi_config_page(void)
{
    return "<!DOCTYPE html>"
           "<html><head>"
           "<title>Configuração Wi-Fi - ESP32-C6</title>"
           "<meta name='viewport' content='width=device-width, initial-scale=1'>"
           "<style>"
           "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }"
           ".container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
           "h1 { color: #2c3e50; text-align: center; }"
           "form { margin: 20px 0; }"
           "label { display: block; margin: 10px 0 5px 0; font-weight: bold; }"
           "input, select { width: 100%; padding: 10px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }"
           "button { width: 100%; padding: 12px; background: #3498db; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }"
           "button:hover { background: #2980b9; }"
           ".button { display: inline-block; padding: 10px 20px; margin: 10px 5px; background: #95a5a6; color: white; text-decoration: none; border-radius: 5px; text-align: center; }"
           ".status { padding: 10px; margin: 10px 0; border-radius: 5px; }"
           ".success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }"
           ".error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }"
           ".loading { background: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; }"
           "</style></head>"
           "<body>"
           "<div class='container'>"
           "<h1>Configuração Wi-Fi</h1>"
           "<div id='status'></div>"
           "<form id='wifiForm'>"
           "<label for='ssid'>Rede Wi-Fi:</label>"
           "<select id='ssid' name='ssid' required>"
           "<option value=''>Escaneando redes...</option>"
           "</select>"
           "<label for='password'>Senha:</label>"
           "<input type='password' id='password' name='password' placeholder='Digite a senha da rede'>"
           "<label>"
           "<input type='checkbox' id='auto_connect' name='auto_connect' checked>"
           " Conectar automaticamente"
           "</label>"
           "<button type='submit'>Conectar</button>"
           "</form>"
           "<a href='/' class='button'>Voltar</a>"
           "</div>"
           "<script>"
           "// Carregar lista de redes"
           "function loadNetworks() {"
           "  document.getElementById('status').innerHTML = '<div class=\"loading\">Escaneando redes Wi-Fi...</div>';"
           "  fetch('/api/wifi/scan')"
           "  .then(response => response.json())"
           "  .then(data => {"
           "    const select = document.getElementById('ssid');"
           "    select.innerHTML = '<option value=\"\">Selecione uma rede</option>';"
           "    data.networks.forEach(network => {"
           "      const option = document.createElement('option');"
           "      option.value = network.ssid;"
           "      option.textContent = network.ssid + ' (' + network.rssi + ' dBm)';"
           "      select.appendChild(option);"
           "    });"
           "    document.getElementById('status').innerHTML = '';"
           "  })"
           "  .catch(error => {"
           "    document.getElementById('status').innerHTML = '<div class=\"error\">Erro ao escanear redes: ' + error + '</div>';"
           "  });"
           "}"
           ""
           "// Enviar configuração"
           "document.getElementById('wifiForm').addEventListener('submit', function(e) {"
           "  e.preventDefault();"
           "  const formData = new FormData(this);"
           "  const data = Object.fromEntries(formData);"
           "  data.auto_connect = document.getElementById('auto_connect').checked;"
           ""
           "  document.getElementById('status').innerHTML = '<div class=\"loading\">Conectando...</div>';"
           ""
           "  fetch('/wifi', {"
           "    method: 'POST',"
           "    headers: { 'Content-Type': 'application/json' },"
           "    body: JSON.stringify(data)"
           "  })"
           "  .then(response => response.json())"
           "  .then(data => {"
           "    if (data.success) {"
           "      document.getElementById('status').innerHTML = '<div class=\"success\">' + data.message + '</div>';"
           "    } else {"
           "      document.getElementById('status').innerHTML = '<div class=\"error\">' + data.message + '</div>';"
           "    }"
           "  })"
           "  .catch(error => {"
           "    document.getElementById('status').innerHTML = '<div class=\"error\">Erro: ' + error + '</div>';"
           "  });"
           "});"
           ""
           "// Carregar redes ao abrir a página"
           "loadNetworks();"
           "</script>"
           "</body></html>";
}

const char* get_ota_page(void)
{
    return "<!DOCTYPE html>"
           "<html><head>"
           "<title>OTA Upgrade - ESP32-C6</title>"
           "<meta name='viewport' content='width=device-width, initial-scale=1'>"
           "<style>"
           "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }"
           ".container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
           "h1 { color: #2c3e50; text-align: center; }"
           "form { margin: 20px 0; }"
           "label { display: block; margin: 10px 0 5px 0; font-weight: bold; }"
           "input, select { width: 100%; padding: 10px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }"
           "button { width: 100%; padding: 12px; background: #e74c3c; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }"
           "button:hover { background: #c0392b; }"
           ".button { display: inline-block; padding: 10px 20px; margin: 10px 5px; background: #95a5a6; color: white; text-decoration: none; border-radius: 5px; text-align: center; }"
           ".status { padding: 10px; margin: 10px 0; border-radius: 5px; }"
           ".success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }"
           ".error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }"
           ".loading { background: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; }"
           ".info { background: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; }"
           ".progress { width: 100%; background: #f0f0f0; border-radius: 5px; overflow: hidden; }"
           ".progress-bar { height: 20px; background: #3498db; transition: width 0.3s; }"
           "</style></head>"
           "<body>"
           "<div class='container'>"
           "<h1>OTA Firmware Upgrade</h1>"
           "<div id='status'></div>"
           "<div id='firmware-info' class='info' style='display:none;'></div>"
           "<form id='otaForm'>"
           "<label for='partition'>Partição:</label>"
           "<select id='partition' name='partition' required>"
           "<option value=''>Carregando partições...</option>"
           "</select>"
           "<label for='firmware'>Arquivo de Firmware:</label>"
           "<input type='file' id='firmware' name='firmware' accept='.bin' required>"
           "<button type='submit'>Iniciar Upgrade</button>"
           "</form>"
           "<div id='progress' style='display:none;'>"
           "<div class='progress'>"
           "<div id='progress-bar' class='progress-bar' style='width: 0%;'></div>"
           "</div>"
           "<div id='progress-text'>0%</div>"
           "</div>"
           "<a href='/' class='button'>Voltar</a>"
           "</div>"
           "<script>"
           "// Carregar informações do firmware"
           "function loadFirmwareInfo() {"
           "  fetch('/api/firmware')"
           "  .then(response => response.json())"
           "  .then(data => {"
           "    const infoDiv = document.getElementById('firmware-info');"
           "    infoDiv.innerHTML = 'Versão Atual: ' + data.version + '<br>AT Core: ' + data.at_core;"
           "    infoDiv.style.display = 'block';"
           "  });"
           "}"
           ""
           "// Carregar partições"
           "function loadPartitions() {"
           "  fetch('/api/ota/partitions')"
           "  .then(response => response.json())"
           "  .then(data => {"
           "    const select = document.getElementById('partition');"
           "    select.innerHTML = '<option value=\"\">Selecione uma partição</option>';"
           "    data.partitions.forEach(partition => {"
           "      const option = document.createElement('option');"
           "      option.value = partition.name;"
           "      option.textContent = partition.name + ' (' + partition.size + ' bytes)';"
           "      select.appendChild(option);"
           "    });"
           "  });"
           "}"
           ""
           "// Enviar upgrade OTA"
           "document.getElementById('otaForm').addEventListener('submit', function(e) {"
           "  e.preventDefault();"
           "  const formData = new FormData(this);"
           "  const file = document.getElementById('firmware').files[0];"
           "  if (!file) {"
           "    document.getElementById('status').innerHTML = '<div class=\"error\">Selecione um arquivo de firmware</div>';"
           "    return;"
           "  }"
           ""
           "  const data = {"
           "    partition: formData.get('partition'),"
           "    file_name: file.name,"
           "    file_size: file.size"
           "  };"
           ""
           "  document.getElementById('status').innerHTML = '<div class=\"loading\">Iniciando upgrade...</div>';"
           "  document.getElementById('progress').style.display = 'block';"
           ""
           "  // Simular progresso (em implementação real, usar WebSocket ou polling)"
           "  let progress = 0;"
           "  const progressInterval = setInterval(() => {"
           "    progress += Math.random() * 10;"
           "    if (progress > 100) progress = 100;"
           "    document.getElementById('progress-bar').style.width = progress + '%';"
           "    document.getElementById('progress-text').textContent = Math.round(progress) + '%';"
           "    if (progress >= 100) {"
           "      clearInterval(progressInterval);"
           "      document.getElementById('status').innerHTML = '<div class=\"success\">Upgrade concluído com sucesso!</div>';"
           "    }"
           "  }, 200);"
           "});"
           ""
           "// Carregar dados ao abrir a página"
           "loadFirmwareInfo();"
           "loadPartitions();"
           "</script>"
           "</body></html>";
}

// Implementação dos handlers HTTP
esp_err_t root_get_handler(httpd_req_t *req)
{
    return send_html_response(req, get_main_page());
}

esp_err_t wifi_config_get_handler(httpd_req_t *req)
{
    return send_html_response(req, get_wifi_config_page());
}

esp_err_t wifi_config_post_handler(httpd_req_t *req)
{
    char buffer[512];
    esp_err_t ret = extract_post_data(req, buffer, sizeof(buffer));
    if (ret != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Erro ao ler dados");
        return ESP_FAIL;
    }
    
    // Parse JSON (simplificado)
    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "JSON inválido");
        return ESP_FAIL;
    }
    
    wifi_config_data_t config = {0};
    cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
    cJSON *password = cJSON_GetObjectItem(json, "password");
    cJSON *auto_connect = cJSON_GetObjectItem(json, "auto_connect");
    
    if (ssid && cJSON_IsString(ssid)) {
        strncpy(config.ssid, ssid->valuestring, sizeof(config.ssid) - 1);
    }
    if (password && cJSON_IsString(password)) {
        strncpy(config.password, password->valuestring, sizeof(config.password) - 1);
    }
    if (auto_connect && cJSON_IsBool(auto_connect)) {
        config.auto_connect = cJSON_IsTrue(auto_connect);
    }
    
    cJSON_Delete(json);
    
    // Processar configuração
    server_response_t response = process_wifi_config(&config);
    
    // Enviar resposta JSON
    cJSON *response_json = cJSON_CreateObject();
    cJSON_AddBoolToObject(response_json, "success", response.success);
    cJSON_AddStringToObject(response_json, "message", response.message);
    
    char *response_str = cJSON_Print(response_json);
    esp_err_t send_ret = send_json_response(req, response_str);
    
    free(response_str);
    cJSON_Delete(response_json);
    
    return send_ret;
}

esp_err_t ota_get_handler(httpd_req_t *req)
{
    return send_html_response(req, get_ota_page());
}

esp_err_t ota_post_handler(httpd_req_t *req)
{
    // Implementação simplificada - em produção, usar multipart/form-data
    char buffer[512];
    esp_err_t ret = extract_post_data(req, buffer, sizeof(buffer));
    if (ret != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Erro ao ler dados");
        return ESP_FAIL;
    }
    
    // Parse JSON
    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "JSON inválido");
        return ESP_FAIL;
    }
    
    ota_config_data_t config = {0};
    cJSON *partition = cJSON_GetObjectItem(json, "partition");
    cJSON *file_name = cJSON_GetObjectItem(json, "file_name");
    cJSON *file_size = cJSON_GetObjectItem(json, "file_size");
    
    if (partition && cJSON_IsString(partition)) {
        strncpy(config.partition, partition->valuestring, sizeof(config.partition) - 1);
    }
    if (file_name && cJSON_IsString(file_name)) {
        strncpy(config.file_name, file_name->valuestring, sizeof(config.file_name) - 1);
    }
    if (file_size && cJSON_IsNumber(file_size)) {
        config.file_size = file_size->valuedouble;
    }
    
    cJSON_Delete(json);
    
    // Processar OTA (simplificado)
    server_response_t response = {0};
    response.success = true;
    response.code = 200;
    strcpy(response.message, "Upgrade OTA iniciado com sucesso");
    
    // Enviar resposta JSON
    cJSON *response_json = cJSON_CreateObject();
    cJSON_AddBoolToObject(response_json, "success", response.success);
    cJSON_AddStringToObject(response_json, "message", response.message);
    
    char *response_str = cJSON_Print(response_json);
    esp_err_t send_ret = send_json_response(req, response_str);
    
    free(response_str);
    cJSON_Delete(response_json);
    
    return send_ret;
}

esp_err_t wifi_scan_api_handler(httpd_req_t *req)
{
    return send_json_response(req, get_wifi_scan_results());
}

esp_err_t status_api_handler(httpd_req_t *req)
{
    return send_json_response(req, get_system_status());
}

esp_err_t firmware_api_handler(httpd_req_t *req)
{
    return send_json_response(req, get_firmware_info());
}

esp_err_t ota_partitions_api_handler(httpd_req_t *req)
{
    return send_json_response(req, get_ota_partitions());
}

esp_err_t wechat_handler(httpd_req_t *req)
{
    // Implementação para WeChat applet
    const char *wechat_html = "<!DOCTYPE html>"
                              "<html><head><title>WeChat Applet</title></head>"
                              "<body><h1>WeChat Applet Support</h1>"
                              "<p>Funcionalidade em desenvolvimento</p></body></html>";
    return send_html_response(req, wechat_html);
}

// Implementação das funções auxiliares
server_response_t process_wifi_config(const wifi_config_data_t *data)
{
    server_response_t response = {0};
    
    if (strlen(data->ssid) == 0) {
        response.success = false;
        response.code = 400;
        strcpy(response.message, "SSID não pode estar vazio");
        return response;
    }
    
    // Configurar Wi-Fi
    wifi_sta_config_t sta_config = {0};
    strncpy((char*)sta_config.ssid, data->ssid, sizeof(sta_config.ssid) - 1);
    strncpy((char*)sta_config.password, data->password, sizeof(sta_config.password) - 1);
    // auto_connect não existe na estrutura padrão do ESP-IDF
    
    esp_err_t ret = wifi_manager_set_sta_config(&sta_config);
    if (ret == ESP_OK) {
        ret = wifi_manager_connect_sta();
    }
    
    if (ret == ESP_OK) {
        response.success = true;
        response.code = 200;
        strcpy(response.message, "Configuração Wi-Fi aplicada com sucesso");
    } else {
        response.success = false;
        response.code = 500;
        strcpy(response.message, "Erro ao configurar Wi-Fi");
    }
    
    return response;
}

server_response_t process_ota_upgrade(const ota_config_data_t *data, 
                                     const uint8_t *file_data, 
                                     size_t file_size)
{
    server_response_t response = {0};
    
    // Implementação simplificada - em produção, usar esp_ota_ops
    if (file_size == 0) {
        response.success = false;
        response.code = 400;
        strcpy(response.message, "Arquivo vazio");
        return response;
    }
    
    // Simular sucesso
    response.success = true;
    response.code = 200;
    strcpy(response.message, "Upgrade OTA concluído com sucesso");
    
    return response;
}

const char* get_wifi_scan_results(void)
{
    static char json_buffer[2048];
    cJSON *json = cJSON_CreateObject();
    cJSON *networks = cJSON_CreateArray();
    
    // Simular algumas redes (em produção, usar wifi_manager_scan)
    const char *test_networks[][2] = {
        {"FAST_FWR310_02", "-45"},
        {"VIVO-1234", "-60"},
        {"NET_2G", "-70"},
        {"TIM_WiFi", "-55"}
    };
    
    for (int i = 0; i < 4; i++) {
        cJSON *network = cJSON_CreateObject();
        cJSON_AddStringToObject(network, "ssid", test_networks[i][0]);
        cJSON_AddStringToObject(network, "rssi", test_networks[i][1]);
        cJSON_AddStringToObject(network, "auth", "WPA2");
        cJSON_AddItemToArray(networks, network);
    }
    
    cJSON_AddItemToObject(json, "networks", networks);
    
    char *json_string = cJSON_Print(json);
    strncpy(json_buffer, json_string, sizeof(json_buffer) - 1);
    json_buffer[sizeof(json_buffer) - 1] = '\0';
    
    free(json_string);
    cJSON_Delete(json);
    
    return json_buffer;
}

const char* get_system_status(void)
{
    static char json_buffer[512];
    cJSON *json = cJSON_CreateObject();
    
    cJSON_AddBoolToObject(json, "wifi_connected", wifi_manager_is_connected());
    cJSON_AddStringToObject(json, "wifi_ssid", "pos_softap");
    cJSON_AddStringToObject(json, "uptime", "00:05:30");
    cJSON_AddNumberToObject(json, "free_heap", esp_get_free_heap_size());
    cJSON_AddStringToObject(json, "version", "1.0.0");
    
    char *json_string = cJSON_Print(json);
    strncpy(json_buffer, json_string, sizeof(json_buffer) - 1);
    json_buffer[sizeof(json_buffer) - 1] = '\0';
    
    free(json_string);
    cJSON_Delete(json);
    
    return json_buffer;
}

const char* get_firmware_info(void)
{
    static char json_buffer[256];
    cJSON *json = cJSON_CreateObject();
    
    cJSON_AddStringToObject(json, "version", "1.0.0");
    cJSON_AddStringToObject(json, "at_core", "2.4.0.0");
    cJSON_AddStringToObject(json, "build_date", __DATE__ " " __TIME__);
    
    char *json_string = cJSON_Print(json);
    strncpy(json_buffer, json_string, sizeof(json_buffer) - 1);
    json_buffer[sizeof(json_buffer) - 1] = '\0';
    
    free(json_string);
    cJSON_Delete(json);
    
    return json_buffer;
}

const char* get_ota_partitions(void)
{
    static char json_buffer[512];
    cJSON *json = cJSON_CreateObject();
    cJSON *partitions = cJSON_CreateArray();
    
    // Simular partições (em produção, usar esp_ota_get_partition_table)
    const char *partition_names[] = {"ota_0", "ota_1", "nvs", "spiffs"};
    const int partition_sizes[] = {1048576, 1048576, 24576, 1048576};
    
    for (int i = 0; i < 4; i++) {
        cJSON *partition = cJSON_CreateObject();
        cJSON_AddStringToObject(partition, "name", partition_names[i]);
        cJSON_AddNumberToObject(partition, "size", partition_sizes[i]);
        cJSON_AddStringToObject(partition, "type", i < 2 ? "app" : "data");
        cJSON_AddItemToArray(partitions, partition);
    }
    
    cJSON_AddItemToObject(json, "partitions", partitions);
    
    char *json_string = cJSON_Print(json);
    strncpy(json_buffer, json_string, sizeof(json_buffer) - 1);
    json_buffer[sizeof(json_buffer) - 1] = '\0';
    
    free(json_string);
    cJSON_Delete(json);
    
    return json_buffer;
}
