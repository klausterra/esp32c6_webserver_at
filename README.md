# ESP32-C6 Web Server AT

## 📋 Visão Geral

Este projeto implementa um servidor web completo no ESP32-C6 usando o framework ESP-IDF, baseado na documentação oficial da Espressif para Web Server AT Examples. O sistema fornece funcionalidades de servidor web, SoftAP (Access Point), OTA (Over-The-Air) updates e Captive Portal.

## 🚀 Funcionalidades

- **SoftAP Wi-Fi**: Cria rede `pos_softap` para configuração
- **Servidor Web**: Interface HTTP na porta 80
- **OTA Updates**: Atualização de firmware via web
- **Captive Portal**: Redirecionamento automático para configuração
- **Comandos AT**: Interface para controle via comandos AT
- **NVS Storage**: Armazenamento persistente de configurações

## 🏗️ Arquitetura

```
webserver_at/
├── main/
│   └── main.c                 # Aplicação principal
├── src/
│   ├── wifi_manager.c/h       # Gerenciamento Wi-Fi
│   ├── web_server.c/h         # Servidor HTTP
│   ├── ota_handler.c/h        # Atualizações OTA
│   └── captive_portal.c/h     # Portal cativo
├── examples/
│   ├── basic_usage.c          # Exemplo de uso básico
│   └── at_commands_example.c  # Exemplo de comandos AT
├── docs/
│   ├── API_REFERENCE.md       # Referência da API
│   ├── AT_COMMANDS.md         # Documentação de comandos AT
│   └── TROUBLESHOOTING.md     # Guia de solução de problemas
├── CMakeLists.txt             # Configuração CMake principal
├── partitions.csv             # Tabela de partições (4MB)
├── sdkconfig.defaults         # Configurações padrão
└── README.md                  # Este arquivo
```

## ⚙️ Configuração

### Pré-requisitos

- ESP-IDF v6.0 ou superior
- ESP32-C6 com 4MB de flash
- Python 3.8+
- CMake 3.16+

### Configurações Padrão

```c
#define DEFAULT_AP_SSID     "pos_softap"
#define DEFAULT_AP_PASSWORD "espressif"
#define DEFAULT_AP_CHANNEL  11
#define DEFAULT_AP_MAX_CONN 3
#define DEFAULT_WEB_PORT    80
#define DEFAULT_WEB_TIMEOUT 25
```

### Partições (4MB Flash)

| Nome     | Tipo    | Subtipo | Offset  | Tamanho | Descrição |
|----------|---------|---------|---------|---------|-----------|
| nvs      | data    | nvs     | 0x9000  | 24KB    | Dados NVS |
| phy_init | data    | phy     | 0xf000  | 4KB     | RF data   |
| factory  | app     | factory | 0x10000 | 1MB     | App principal |
| ota_0    | app     | ota_0   | 0x110000| 1MB     | Partição OTA 0 |
| ota_1    | app     | ota_1   | 0x210000| 1MB     | Partição OTA 1 |
| spiffs   | data    | spiffs  | 0x310000| 896KB   | Sistema de arquivos |

## 🔧 Compilação e Upload

### 1. Configurar Ambiente

```bash
export IDF_PATH='/root/esp/esp-idf'
cd /root/webserver_at
idf.py set-target esp32c6
```

### 2. Compilar

```bash
idf.py build
```

### 3. Upload

```bash
idf.py -p /dev/ttyACM0 flash
```

### 4. Monitorar

```bash
idf.py -p /dev/ttyACM0 monitor
```

## 🌐 Uso

### 1. Conectar à Rede

1. Procure pela rede Wi-Fi `pos_softap`
2. Conecte-se com a senha `espressif`
3. Acesse `http://192.168.4.1` no navegador

### 2. Interface Web

- **Página Principal**: `http://192.168.4.1/`
- **Configuração Wi-Fi**: `http://192.168.4.1/wifi`
- **OTA Update**: `http://192.168.4.1/ota`
- **Status do Sistema**: `http://192.168.4.1/status`

### 3. Comandos AT

```bash
# Conectar via serial
screen /dev/ttyUSB0 115200

# Comandos básicos
AT
AT+RST
AT+GMR
AT+CWMODE=3
AT+CWJAP="SSID","PASSWORD"
```

## 📡 APIs Disponíveis

### Endpoints HTTP

| Endpoint | Método | Descrição |
|----------|--------|-----------|
| `/` | GET | Página principal |
| `/wifi` | GET/POST | Configuração Wi-Fi |
| `/ota` | GET/POST | Upload de firmware |
| `/status` | GET | Status do sistema |
| `/api/info` | GET | Informações do dispositivo |
| `/api/scan` | GET | Scan de redes Wi-Fi |

### Estruturas de Dados

```c
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
```

## 🔍 Solução de Problemas

### Problemas Comuns

1. **SoftAP não aparece**
   - Verifique se o ESP32 está inicializando corretamente
   - Confirme que não há conflitos de canal Wi-Fi
   - Verifique logs de inicialização

2. **Erro de compilação**
   - Verifique se o target está configurado para `esp32c6`
   - Confirme que todas as dependências estão instaladas
   - Limpe o build: `idf.py fullclean`

3. **Upload falha**
   - Verifique se a porta `/dev/ttyACM0` está disponível
   - Confirme que o ESP32 está em modo de programação
   - Tente resetar o dispositivo

### Logs Importantes

```
I (xxx) WEBSERVER_AT: SoftAP iniciado com sucesso
I (xxx) esp_netif_lwip: DHCP server started on interface WIFI_AP_DEF with IP: 192.168.4.1
I (xxx) WEB_SERVER: Servidor web iniciado com sucesso na porta 80
I (xxx) WEBSERVER_AT: Sistema pronto para comandos AT
```

## 📚 Referências

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [ESP32-C6 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf)
- [Web Server AT Examples](https://docs.espressif.com/projects/esp-at/en/latest/esp32c6/AT_Command_Examples/Web_server_AT_Examples.html)

## 🛠️ Desenvolvimento

### Estrutura do Código

- **main.c**: Inicialização principal e loop de aplicação
- **wifi_manager.c**: Gerenciamento de conexões Wi-Fi
- **web_server.c**: Implementação do servidor HTTP
- **ota_handler.c**: Lógica de atualizações OTA
- **captive_portal.c**: Portal cativo para configuração

### Adicionando Novos Endpoints

```c
static esp_err_t new_handler(httpd_req_t *req) {
    // Implementar lógica do endpoint
    return ESP_OK;
}

// Registrar no web_server.c
httpd_uri_t new_uri = {
    .uri = "/new",
    .method = HTTP_GET,
    .handler = new_handler,
    .user_ctx = NULL
};
httpd_register_uri_handler(server, &new_uri);
```

## 📄 Licença

Este projeto está baseado na documentação oficial da Espressif e segue as mesmas diretrizes de licenciamento.

## 🤝 Contribuição

1. Fork o projeto
2. Crie uma branch para sua feature
3. Commit suas mudanças
4. Push para a branch
5. Abra um Pull Request

## 📞 Suporte

Para suporte técnico, consulte:
- [ESP32 Forum](https://esp32.com/)
- [ESP-IDF Issues](https://github.com/espressif/esp-idf/issues)
- [Documentação Oficial](https://docs.espressif.com/)

---

**Versão**: 1.0.0  
**Data**: Setembro 2025  
**Autor**: Baseado na documentação oficial Espressif  
**Target**: ESP32-C6 (4MB Flash)