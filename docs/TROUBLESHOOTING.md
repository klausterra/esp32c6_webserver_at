# Troubleshooting - ESP32-C6 Web Server AT

## 🔍 Problemas Comuns

### 1. SoftAP não aparece

**Sintomas**:
- Rede `pos_softap` não é visível
- Dispositivos não conseguem conectar

**Possíveis Causas**:
- ESP32 não inicializou corretamente
- Conflito de canal Wi-Fi
- Problema na configuração do SoftAP

**Soluções**:
```bash
# Verificar logs de inicialização
idf.py monitor

# Procurar por estas mensagens:
I (xxx) WEBSERVER_AT: SoftAP iniciado com sucesso
I (xxx) esp_netif_lwip: DHCP server started on interface WIFI_AP_DEF with IP: 192.168.4.1
```

**Verificações**:
1. Confirme que o ESP32 está inicializando
2. Verifique se não há conflitos de canal
3. Teste com canal diferente: `AT+CWSAP="pos_softap","espressif",6,4`

### 2. Erro de Compilação

**Sintomas**:
- `idf.py build` falha
- Erros de dependências

**Possíveis Causas**:
- Target incorreto
- Dependências não instaladas
- Cache de build corrompido

**Soluções**:
```bash
# Limpar build
idf.py fullclean

# Configurar target correto
idf.py set-target esp32c6

# Reinstalar dependências
idf.py reconfigure

# Recompilar
idf.py build
```

### 3. Upload Falha

**Sintomas**:
- `idf.py flash` falha
- Erro de porta serial

**Possíveis Causas**:
- Porta serial ocupada
- ESP32 não em modo de programação
- Permissões de porta

**Soluções**:
```bash
# Verificar portas disponíveis
ls /dev/ttyACM*

# Verificar se porta está ocupada
lsof /dev/ttyACM0

# Matar processo se necessário
sudo kill <PID>

# Verificar permissões
sudo chmod 666 /dev/ttyACM0

# Tentar upload novamente
idf.py -p /dev/ttyACM0 flash
```

### 4. Servidor Web não Responde

**Sintomas**:
- Página não carrega
- Timeout de conexão
- Erro 404

**Possíveis Causas**:
- Servidor não iniciado
- Problema de rede
- Handler não registrado

**Soluções**:
```bash
# Verificar se servidor está rodando
idf.py monitor | grep "Servidor web"

# Verificar IP do SoftAP
AT+CIFSR

# Testar conectividade
ping 192.168.4.1

# Verificar handlers registrados
AT+HTTPHANDLERS
```

### 5. OTA Update Falha

**Sintomas**:
- Upload de firmware falha
- Erro durante atualização
- Dispositivo não reinicia

**Possíveis Causas**:
- URL inválida
- Firmware corrompido
- Espaço insuficiente em flash

**Soluções**:
```bash
# Verificar URL
AT+OTAURL

# Verificar espaço em flash
AT+FLASHINFO

# Verificar status OTA
AT+OTASTATUS

# Resetar configuração OTA
AT+OTARESET
```

## 🔧 Diagnósticos Avançados

### Verificar Logs Detalhados

```bash
# Habilitar logs verbosos
idf.py menuconfig
# Component config → Log output → Default log verbosity → Verbose

# Monitorar com logs detalhados
idf.py monitor --print_filter="*:V"
```

### Verificar Configurações

```bash
# Verificar configuração atual
AT+CONFIG

# Verificar status do sistema
AT+SYSTEMSTATUS

# Verificar informações de hardware
AT+HWINFO

# Verificar configuração de rede
AT+NETINFO
```

### Teste de Conectividade

```bash
# Teste de ping
AT+PING=8.8.8.8

# Scan de redes
AT+CWLAP

# Verificar IP atual
AT+CIFSR

# Teste de conectividade HTTP
AT+HTTPTEST=http://httpbin.org/get
```

## 🛠️ Soluções por Código de Erro

### ESP_ERR_INVALID_STATE
**Causa**: Ordem incorreta de inicialização  
**Solução**: Verificar se `esp_wifi_init()` é chamado antes de `esp_netif_create_default_wifi_ap()`

### ESP_ERR_NO_MEM
**Causa**: Memória insuficiente  
**Solução**: 
- Verificar heap disponível
- Reduzir número de conexões simultâneas
- Otimizar uso de memória

### ESP_ERR_TIMEOUT
**Causa**: Timeout de operação  
**Solução**:
- Aumentar timeout de configuração
- Verificar conectividade de rede
- Verificar se servidor está respondendo

### ESP_ERR_NOT_FOUND
**Causa**: Recurso não encontrado  
**Solução**:
- Verificar se partição existe
- Verificar se arquivo está presente
- Verificar permissões de acesso

## 📊 Monitoramento de Performance

### Verificar Uso de Memória

```c
// No código
ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
ESP_LOGI(TAG, "Min free heap: %d bytes", esp_get_minimum_free_heap_size());
```

### Verificar Uso de CPU

```bash
# Monitorar via serial
idf.py monitor | grep "cpu_freq"
```

### Verificar Temperatura

```bash
# Verificar temperatura do chip
AT+TEMP
```

## 🔄 Procedimentos de Recuperação

### Reset Completo

```bash
# 1. Parar monitoramento
Ctrl+C

# 2. Fazer reset do ESP32
idf.py -p /dev/ttyACM0 reset

# 3. Limpar flash completamente
idf.py -p /dev/ttyACM0 erase-flash

# 4. Fazer upload completo
idf.py -p /dev/ttyACM0 flash

# 5. Monitorar inicialização
idf.py -p /dev/ttyACM0 monitor
```

### Recuperação de Firmware Corrompido

```bash
# 1. Entrar em modo de recuperação
# Segurar BOOT e pressionar RESET

# 2. Fazer upload do bootloader
esptool.py --chip esp32c6 --port /dev/ttyACM0 write_flash 0x0 bootloader.bin

# 3. Fazer upload da partição
esptool.py --chip esp32c6 --port /dev/ttyACM0 write_flash 0x8000 partition-table.bin

# 4. Fazer upload da aplicação
esptool.py --chip esp32c6 --port /dev/ttyACM0 write_flash 0x10000 webserver_at.bin
```

### Restaurar Configurações Padrão

```bash
# Via comandos AT
AT+RESETCONFIG
AT+SAVECONFIG
AT+RST
```

## 📋 Checklist de Verificação

### Antes de Reportar Problema

- [ ] ESP32 está inicializando corretamente?
- [ ] Logs mostram erros específicos?
- [ ] Configuração está correta?
- [ ] Hardware está funcionando?
- [ ] Firmware está atualizado?
- [ ] Dependências estão instaladas?
- [ ] Porta serial está disponível?
- [ ] Rede Wi-Fi está funcionando?

### Informações para Suporte

1. **Logs completos** de inicialização
2. **Configuração atual** (`AT+CONFIG`)
3. **Status do sistema** (`AT+SYSTEMSTATUS`)
4. **Informações de hardware** (`AT+HWINFO`)
5. **Versão do firmware** (`AT+GMR`)
6. **Configuração de rede** (`AT+NETINFO`)

## 🆘 Contatos de Suporte

- **ESP32 Forum**: https://esp32.com/
- **ESP-IDF Issues**: https://github.com/espressif/esp-idf/issues
- **Documentação Oficial**: https://docs.espressif.com/
- **Exemplos Oficiais**: https://github.com/espressif/esp-idf/tree/master/examples

## 📚 Recursos Adicionais

- [ESP32-C6 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/)
- [Wi-Fi API Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/network/esp_wifi.html)
- [HTTP Server API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/protocols/esp_http_server.html)