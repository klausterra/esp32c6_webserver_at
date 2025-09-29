# AT Commands - ESP32-C6 Web Server

## 📋 Comandos Básicos

### Teste de Comunicação
```
AT
```
**Resposta**: `OK`  
**Descrição**: Testa se o módulo está respondendo

### Reset
```
AT+RST
```
**Resposta**: `OK`  
**Descrição**: Reinicia o módulo

### Informações de Versão
```
AT+GMR
```
**Resposta**:
```
AT version:2.4.0.0
SDK version:v4.3.2-rc1
compile time:Jul 15 2021 18:34:26
Bin version:2.4.0(WROOM-02)
OK
```

## 📡 Comandos Wi-Fi

### Configurar Modo Wi-Fi
```
AT+CWMODE=<mode>
```
**Parâmetros**:
- `1`: Modo Station (cliente)
- `2`: Modo SoftAP (ponto de acesso)
- `3`: Modo Station + SoftAP

**Exemplo**:
```
AT+CWMODE=3
OK
```

### Conectar a Rede Wi-Fi
```
AT+CWJAP="<ssid>","<password>"
```
**Exemplo**:
```
AT+CWJAP="MinhaRede","minhasenha123"
OK
```

### Desconectar do Wi-Fi
```
AT+CWQAP
```
**Resposta**: `OK`

### Listar Redes Disponíveis
```
AT+CWLAP
```
**Resposta**:
```
+CWLAP:(3,"MinhaRede",-45,"aa:bb:cc:dd:ee:ff",6)
+CWLAP:(4,"OutraRede",-60,"11:22:33:44:55:66",11)
OK
```

### Configurar SoftAP
```
AT+CWSAP="<ssid>","<password>",<channel>,<encryption>
```
**Parâmetros**:
- `ssid`: Nome da rede
- `password`: Senha (mínimo 8 caracteres)
- `channel`: Canal Wi-Fi (1-13)
- `encryption`: Tipo de criptografia (0-4)

**Exemplo**:
```
AT+CWSAP="pos_softap","espressif",11,4
OK
```

### Obter Endereço IP
```
AT+CIFSR
```
**Resposta**:
```
+CIFSR:APIP,"192.168.4.1"
+CIFSR:APMAC,"aa:bb:cc:dd:ee:ff"
+CIFSR:STAIP,"192.168.1.100"
+CIFSR:STAMAC,"11:22:33:44:55:66"
OK
```

## 🌐 Comandos HTTP

### Iniciar Servidor Web
```
AT+HTTPSTART
```
**Resposta**: `OK`  
**Descrição**: Inicia o servidor web na porta 80

### Parar Servidor Web
```
AT+HTTPSTOP
```
**Resposta**: `OK`  
**Descrição**: Para o servidor web

### Configurar Servidor Web
```
AT+HTTPCONFIG=<port>,<max_conn>
```
**Parâmetros**:
- `port`: Porta do servidor (padrão: 80)
- `max_conn`: Máximo de conexões (padrão: 4)

**Exemplo**:
```
AT+HTTPCONFIG=80,8
OK
```

### Registrar Handler HTTP
```
AT+HTTPHANDLER=<uri>,<method>,<handler_id>
```
**Parâmetros**:
- `uri`: Caminho da URL (ex: "/api/status")
- `method`: Método HTTP (GET, POST, PUT, DELETE)
- `handler_id`: ID do handler (0-7)

**Exemplo**:
```
AT+HTTPHANDLER="/api/status",GET,0
OK
```

## 🔄 Comandos OTA

### Configurar URL OTA
```
AT+OTAURL="<url>"
```
**Exemplo**:
```
AT+OTAURL="http://exemplo.com/firmware.bin"
OK
```

### Iniciar Atualização OTA
```
AT+OTASTART
```
**Resposta**: `OK`  
**Descrição**: Inicia o processo de atualização OTA

### Verificar Status OTA
```
AT+OTASTATUS
```
**Resposta**:
```
+OTASTATUS:0,100
OK
```
**Descrição**: Retorna status (0=sucesso) e progresso (0-100%)

## 📊 Comandos de Status

### Status do Sistema
```
AT+SYSTEMSTATUS
```
**Resposta**:
```
+SYSTEMSTATUS:online,192.168.4.1,12345,123456
OK
```
**Descrição**: Status, IP, uptime, memória livre

### Informações do Hardware
```
AT+HWINFO
```
**Resposta**:
```
+HWINFO:ESP32-C6,v0.2,160MHz,4MB,54:32:04:3f:21:e4
OK
```

### Configurações Atuais
```
AT+CONFIG
```
**Resposta**:
```
+CONFIG:pos_softap,espressif,11,3,80,25,1,0
OK
```

## 🔧 Comandos de Configuração

### Salvar Configuração
```
AT+SAVECONFIG
```
**Resposta**: `OK`  
**Descrição**: Salva configurações atuais na NVS

### Carregar Configuração
```
AT+LOADCONFIG
```
**Resposta**: `OK`  
**Descrição**: Carrega configurações da NVS

### Reset de Configuração
```
AT+RESETCONFIG
```
**Resposta**: `OK`  
**Descrição**: Restaura configurações padrão

### Definir Timeout
```
AT+TIMEOUT=<seconds>
```
**Exemplo**:
```
AT+TIMEOUT=30
OK
```

## 📝 Comandos de Log

### Configurar Nível de Log
```
AT+LOGLEVEL=<level>
```
**Parâmetros**:
- `0`: Nenhum log
- `1`: Erro
- `2`: Aviso
- `3`: Info
- `4`: Debug
- `5`: Verbose

**Exemplo**:
```
AT+LOGLEVEL=3
OK
```

### Habilitar/Desabilitar Logs
```
AT+LOGENABLE=<enable>
```
**Parâmetros**:
- `0`: Desabilitar logs
- `1`: Habilitar logs

## 🔍 Comandos de Debug

### Teste de Conectividade
```
AT+PING=<host>
```
**Exemplo**:
```
AT+PING=8.8.8.8
+PING:64
OK
```

### Scan de Portas
```
AT+PORTSCAN=<host>,<start_port>,<end_port>
```
**Exemplo**:
```
AT+PORTSCAN=192.168.1.1,80,90
+PORTSCAN:80,443,8080
OK
```

### Informações de Rede
```
AT+NETINFO
```
**Resposta**:
```
+NETINFO:192.168.4.1,255.255.255.0,192.168.4.1,8.8.8.8
OK
```

## ⚠️ Códigos de Erro

| Código | Descrição |
|--------|-----------|
| `OK` | Comando executado com sucesso |
| `ERROR` | Erro genérico |
| `+CME ERROR:1` | Falha de operação |
| `+CME ERROR:2` | Argumento inválido |
| `+CME ERROR:3` | Sem memória |
| `+CME ERROR:4` | Timeout |
| `+CME ERROR:5` | Recurso não encontrado |

## 📋 Exemplos Práticos

### Configuração Completa
```
AT+CWMODE=3
OK
AT+CWSAP="pos_softap","espressif",11,4
OK
AT+HTTPCONFIG=80,8
OK
AT+HTTPSTART
OK
AT+SAVECONFIG
OK
```

### Conectar e Configurar
```
AT+CWMODE=1
OK
AT+CWJAP="MinhaRede","minhasenha123"
OK
AT+CIFSR
+CIFSR:STAIP,"192.168.1.100"
OK
```

### Atualização OTA
```
AT+OTAURL="http://exemplo.com/firmware.bin"
OK
AT+OTASTART
OK
AT+OTASTATUS
+OTASTATUS:0,100
OK
```

## 🔧 Troubleshooting

### Problemas Comuns

1. **Comando não reconhecido**
   - Verifique a sintaxe
   - Confirme que o módulo está inicializado

2. **Falha de conexão Wi-Fi**
   - Verifique SSID e senha
   - Confirme que a rede está no alcance

3. **Servidor web não responde**
   - Verifique se foi iniciado com `AT+HTTPSTART`
   - Confirme que a porta está correta

4. **OTA falha**
   - Verifique a URL do firmware
   - Confirme conectividade de rede
   - Verifique espaço em flash

### Logs de Debug
```
AT+LOGLEVEL=5
AT+LOGENABLE=1
```

### Reset Completo
```
AT+RST
AT+RESETCONFIG
AT+SAVECONFIG
```