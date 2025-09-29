# API Reference - ESP32-C6 Web Server AT

## 📡 Endpoints HTTP

### Página Principal
```
GET /
```
**Descrição**: Página principal do servidor web  
**Resposta**: HTML da interface principal

### Configuração Wi-Fi
```
GET /wifi
POST /wifi
```
**Descrição**: Configuração de rede Wi-Fi  
**Parâmetros POST**:
- `ssid`: Nome da rede Wi-Fi
- `password`: Senha da rede
- `auto_connect`: Conectar automaticamente (true/false)

### Atualização OTA
```
GET /ota
POST /ota
```
**Descrição**: Upload e instalação de firmware  
**Parâmetros POST**:
- `firmware`: Arquivo binário do firmware

### Status do Sistema
```
GET /status
```
**Descrição**: Informações de status do dispositivo  
**Resposta JSON**:
```json
{
  "status": "online",
  "wifi_connected": true,
  "ap_active": true,
  "uptime": 12345,
  "free_heap": 123456,
  "firmware_version": "1.0.0"
}
```

### Informações do Dispositivo
```
GET /api/info
```
**Resposta JSON**:
```json
{
  "chip_model": "ESP32-C6",
  "chip_revision": "v0.2",
  "cpu_freq": 160,
  "flash_size": 4194304,
  "free_heap": 123456,
  "uptime": 12345,
  "wifi_mac": "54:32:04:3f:21:e4"
}
```

### Scan de Redes Wi-Fi
```
GET /api/scan
```
**Resposta JSON**:
```json
{
  "networks": [
    {
      "ssid": "MinhaRede",
      "rssi": -45,
      "auth": "WPA2_PSK",
      "channel": 6
    }
  ]
}
```

## 🔧 Funções C

### WiFi Manager

#### `wifi_init()`
```c
esp_err_t wifi_init(void);
```
**Descrição**: Inicializa o sistema Wi-Fi  
**Retorno**: `ESP_OK` em caso de sucesso

#### `setup_softap()`
```c
esp_err_t setup_softap(void);
```
**Descrição**: Configura o SoftAP  
**Retorno**: `ESP_OK` em caso de sucesso

#### `wifi_connect_sta()`
```c
esp_err_t wifi_connect_sta(const char* ssid, const char* password);
```
**Descrição**: Conecta ao Wi-Fi como cliente  
**Parâmetros**:
- `ssid`: Nome da rede
- `password`: Senha da rede
**Retorno**: `ESP_OK` em caso de sucesso

### Web Server

#### `web_server_init()`
```c
esp_err_t web_server_init(void);
```
**Descrição**: Inicializa o servidor HTTP  
**Retorno**: `ESP_OK` em caso de sucesso

#### `register_web_handlers()`
```c
void register_web_handlers(httpd_handle_t server);
```
**Descrição**: Registra todos os handlers HTTP  
**Parâmetros**:
- `server`: Handle do servidor HTTP

### OTA Handler

#### `init_ota_handler()`
```c
esp_err_t init_ota_handler(void);
```
**Descrição**: Inicializa o sistema OTA  
**Retorno**: `ESP_OK` em caso de sucesso

#### `ota_update()`
```c
esp_err_t ota_update(const char* url);
```
**Descrição**: Atualiza firmware via URL  
**Parâmetros**:
- `url`: URL do firmware
**Retorno**: `ESP_OK` em caso de sucesso

### Captive Portal

#### `init_captive_portal()`
```c
esp_err_t init_captive_portal(void);
```
**Descrição**: Inicializa o portal cativo  
**Retorno**: `ESP_OK` em caso de sucesso

## 📊 Estruturas de Dados

### `webserver_config_t`
```c
typedef struct {
    char ap_ssid[32];              // SSID do SoftAP
    char ap_password[64];          // Senha do SoftAP
    uint8_t ap_channel;            // Canal Wi-Fi
    uint8_t ap_max_conn;           // Máximo de conexões
    uint16_t web_port;             // Porta do servidor web
    uint16_t web_timeout;          // Timeout HTTP
    bool captive_portal_enabled;   // Portal cativo habilitado
    bool wechat_support_enabled;   // Suporte WeChat habilitado
} webserver_config_t;
```

### `wifi_scan_result_t`
```c
typedef struct {
    char ssid[32];
    int8_t rssi;
    wifi_auth_mode_t auth_mode;
    uint8_t channel;
} wifi_scan_result_t;
```

## 🔄 Eventos Wi-Fi

### `wifi_event_handler()`
```c
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data);
```
**Descrição**: Handler de eventos Wi-Fi  
**Eventos suportados**:
- `WIFI_EVENT_AP_START`: SoftAP iniciado
- `WIFI_EVENT_AP_STOP`: SoftAP parado
- `WIFI_EVENT_AP_STACONNECTED`: Dispositivo conectado
- `WIFI_EVENT_AP_STADISCONNECTED`: Dispositivo desconectado

## 🛠️ Configurações

### Constantes
```c
#define DEFAULT_AP_SSID     "pos_softap"
#define DEFAULT_AP_PASSWORD "espressif"
#define DEFAULT_AP_CHANNEL  11
#define DEFAULT_AP_MAX_CONN 3
#define DEFAULT_WEB_PORT    80
#define DEFAULT_WEB_TIMEOUT 25
```

### Configurações NVS
- `wifi_ssid`: SSID salvo
- `wifi_password`: Senha salva
- `ap_config`: Configuração do SoftAP
- `ota_url`: URL para atualizações OTA

## 📝 Exemplos de Uso

### Conectar ao Wi-Fi
```c
// Conectar a uma rede
wifi_connect_sta("MinhaRede", "minhasenha123");

// Configurar SoftAP
setup_softap();
```

### Registrar Handler Personalizado
```c
static esp_err_t custom_handler(httpd_req_t *req) {
    // Implementar lógica
    httpd_resp_send(req, "Resposta", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Registrar
httpd_uri_t custom_uri = {
    .uri = "/custom",
    .method = HTTP_GET,
    .handler = custom_handler
};
httpd_register_uri_handler(server, &custom_uri);
```

### Atualização OTA
```c
// Atualizar via URL
ota_update("http://exemplo.com/firmware.bin");

// Verificar status
esp_ota_img_states_t ota_state;
esp_ota_get_state_partition(ota_partition, &ota_state);
```

## ⚠️ Limitações

- Máximo de 8 handlers HTTP simultâneos
- Timeout de 25 segundos para requisições
- Suporte limitado a 3 conexões simultâneas no SoftAP
- Tamanho máximo de arquivo OTA: 1MB

## 🔍 Debugging

### Logs Importantes
```
I (xxx) WEBSERVER_AT: SoftAP iniciado com sucesso
I (xxx) WEB_SERVER: Servidor web iniciado com sucesso na porta 80
I (xxx) OTA_HANDLER: Handler OTA inicializado
I (xxx) WEBSERVER_AT: Sistema pronto para comandos AT
```

### Códigos de Erro
- `ESP_OK`: Sucesso
- `ESP_ERR_INVALID_ARG`: Argumento inválido
- `ESP_ERR_NO_MEM`: Sem memória disponível
- `ESP_ERR_TIMEOUT`: Timeout da operação
- `ESP_ERR_NOT_FOUND`: Recurso não encontrado