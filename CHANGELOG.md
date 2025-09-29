# Changelog - ESP32-C6 Web Server AT

## [1.0.0] - 2025-09-29

### ✨ Adicionado
- Implementação inicial do servidor web ESP32-C6
- SoftAP Wi-Fi com configuração `pos_softap`
- Servidor HTTP na porta 80
- Sistema de atualização OTA (Over-The-Air)
- Captive Portal para configuração
- Suporte a comandos AT
- Armazenamento NVS para configurações
- Documentação completa (README, API, AT Commands, Troubleshooting)
- Exemplos de uso básico e comandos AT
- Configuração de partições para 4MB de flash
- Sistema de logs detalhado

### 🔧 Configurações
- **SoftAP**: `pos_softap` / `espressif`
- **Canal Wi-Fi**: 11
- **Porta Web**: 80
- **Máximo de conexões**: 3
- **Timeout HTTP**: 25 segundos

### 📁 Estrutura do Projeto
```
webserver_at/
├── main/main.c                 # Aplicação principal
├── src/                        # Código fonte dos módulos
├── examples/                   # Exemplos de uso
├── docs/                       # Documentação
├── partitions.csv              # Tabela de partições
├── sdkconfig.defaults          # Configurações padrão
└── README.md                   # Documentação principal
```

### 🛠️ Correções Técnicas
- Ordem correta de inicialização: `esp_wifi_init()` antes de `esp_netif_create_default_wifi_ap()`
- Configuração de partições otimizada para ESP32-C6 (4MB)
- Desabilitação de NVS encryption para compatibilidade
- Correção de tipos de retorno para funções `esp_netif_create_default_wifi_*()`
- Simplificação do handler de eventos Wi-Fi para foco no SoftAP

### 📊 Partições (4MB Flash)
| Nome     | Tipo    | Tamanho | Descrição |
|----------|---------|---------|-----------|
| nvs      | data    | 24KB    | Dados NVS |
| phy_init | data    | 4KB     | RF data   |
| factory  | app     | 1MB     | App principal |
| ota_0    | app     | 1MB     | Partição OTA 0 |
| ota_1    | app     | 1MB     | Partição OTA 1 |
| spiffs   | data    | 896KB   | Sistema de arquivos |

### 🌐 Endpoints HTTP
- `GET /` - Página principal
- `GET /wifi` - Configuração Wi-Fi
- `POST /wifi` - Salvar configuração Wi-Fi
- `GET /ota` - Interface de atualização OTA
- `POST /ota` - Upload de firmware
- `GET /status` - Status do sistema
- `GET /api/info` - Informações do dispositivo
- `GET /api/scan` - Scan de redes Wi-Fi

### 🔧 Comandos AT Suportados
- `AT` - Teste de comunicação
- `AT+RST` - Reset do sistema
- `AT+GMR` - Informações de versão
- `AT+CWMODE` - Configurar modo Wi-Fi
- `AT+CWJAP` - Conectar ao Wi-Fi
- `AT+CWSAP` - Configurar SoftAP
- `AT+HTTPSTART/STOP` - Controlar servidor web
- `AT+SYSTEMSTATUS` - Status do sistema
- `AT+HWINFO` - Informações de hardware

### 📚 Documentação
- **README.md**: Visão geral e guia de uso
- **API_REFERENCE.md**: Referência completa da API
- **AT_COMMANDS.md**: Documentação de comandos AT
- **TROUBLESHOOTING.md**: Guia de solução de problemas
- **CHANGELOG.md**: Histórico de mudanças

### 🧪 Testes
- Compilação bem-sucedida para ESP32-C6
- Upload de firmware via `/dev/ttyACM0`
- Inicialização correta do SoftAP
- Criação da rede `pos_softap`
- Servidor web respondendo em `http://192.168.4.1`
- Sistema de comandos AT funcional

### ⚠️ Limitações Conhecidas
- Máximo de 8 handlers HTTP simultâneos
- Timeout de 25 segundos para requisições
- Suporte limitado a 3 conexões simultâneas no SoftAP
- Tamanho máximo de arquivo OTA: 1MB

### 🔄 Próximas Versões
- [ ] Suporte a HTTPS
- [ ] Interface web melhorada
- [ ] Mais comandos AT
- [ ] Suporte a WeChat
- [ ] Configuração via app mobile
- [ ] Sistema de backup/restore
- [ ] Monitoramento de performance
- [ ] Suporte a múltiplas redes Wi-Fi

---

**Versão**: 1.0.0  
**Data**: 29 de Setembro de 2025  
**Target**: ESP32-C6 (4MB Flash)  
**ESP-IDF**: v6.0-dev-2594-g25c40d4563  
**Status**: ✅ Funcional e Testado
