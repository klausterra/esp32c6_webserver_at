# Maya Wi-Fi Zigbee Gateway

## 🎯 Visão Geral

O **Maya Wi-Fi Zigbee Gateway** é um gateway inteligente para IoT e automação desenvolvido pelo Eng. Klaus Q. Terra da Hiperenge. Baseado no ESP32-C6, o Maya oferece uma interface web moderna e funcionalidades avançadas para controle e monitoramento de dispositivos IoT.

## ✨ Características Principais

### 🌐 **Gateway Inteligente**
- **Wi-Fi**: Conectividade sem fio robusta
- **Zigbee**: Suporte para protocolo Zigbee (em desenvolvimento)
- **Web Server**: Interface web moderna e responsiva
- **OTA Updates**: Atualizações over-the-air

### 🎨 **Interface Moderna**
- **Design Personalizado**: Branding Maya com logo e background exclusivos
- **Dashboard em Tempo Real**: Monitoramento completo do sistema
- **Responsivo**: Funciona perfeitamente em mobile e desktop
- **Tema Profissional**: Cores e tipografia da marca Maya

### 🔧 **Funcionalidades Técnicas**
- **ESP32-C6**: Microcontrolador de última geração
- **4MB Flash**: Armazenamento amplo para firmware e dados
- **SoftAP**: Rede própria para configuração
- **NVS Storage**: Armazenamento persistente de configurações

## 🚀 Início Rápido

### 1. **Conectar ao Gateway**
1. Procure pela rede Wi-Fi `pos_softap`
2. Conecte-se com a senha `espressif`
3. Acesse `http://192.168.4.1` no navegador

### 2. **Interface Principal**
- **Logo Maya**: Identidade visual da marca
- **Título**: "Maya Wi-Fi Zigbee Gateway"
- **Autor**: Eng. Klaus Q. Terra - Hiperenge
- **Background**: Imagem personalizada da Maya

### 3. **Dashboard Avançado**
- Acesse `http://192.168.4.1/dashboard`
- Monitoramento em tempo real
- Métricas de sistema e rede
- Controles de configuração

## 📊 Recursos do Dashboard

### 🌐 **Status de Rede**
- Conexão Wi-Fi atual
- Status do SoftAP
- Endereço IP
- Número de conexões ativas

### 💻 **Sistema**
- Uptime do dispositivo
- Memória livre disponível
- Frequência da CPU
- Temperatura (quando disponível)

### 📈 **Performance**
- Requisições por minuto
- Gráficos de performance
- Alertas do sistema

## 🎨 Branding Maya

### **Identidade Visual**
- **Logo**: Maya S personalizado
- **Cores**: Paleta profissional azul e verde
- **Background**: Imagem exclusiva da Maya
- **Tipografia**: Segoe UI para legibilidade

### **Elementos Personalizados**
- Header com logo da Maya
- Título "Maya Wi-Fi Zigbee Gateway"
- Créditos "Eng. Klaus Q. Terra - Hiperenge"
- Footer com copyright Hiperenge

## 🔧 Configuração Técnica

### **Hardware**
- **Microcontrolador**: ESP32-C6
- **Flash**: 4MB
- **Wi-Fi**: 802.11 b/g/n
- **Zigbee**: Em desenvolvimento

### **Software**
- **Framework**: ESP-IDF v6.0
- **Web Server**: HTTP Server nativo
- **Storage**: NVS (Non-Volatile Storage)
- **OTA**: Sistema de atualização over-the-air

## 📱 Interface Web

### **Páginas Disponíveis**
- **`/`**: Página principal com branding Maya
- **`/dashboard`**: Dashboard de monitoramento
- **`/wifi`**: Configuração de rede Wi-Fi
- **`/ota`**: Atualização de firmware
- **`/api/status`**: API de status do sistema

### **Recursos Visuais**
- Background personalizado da Maya
- Logo Maya S integrado
- Design responsivo e moderno
- Animações e efeitos visuais

## 🛠️ Desenvolvimento

### **Estrutura do Projeto**
```
webserver_at/
├── web/assets/images/     # Imagens da Maya
│   ├── maya-logo.png      # Logo Maya S
│   └── maya-background.jpg # Background personalizado
├── src/                   # Código fonte
├── main/                  # Aplicação principal
└── docs/                  # Documentação
```

### **Personalização**
- Logo: Substitua `maya-logo.png`
- Background: Substitua `maya-background.jpg`
- Cores: Modifique o CSS no código
- Textos: Atualize os títulos e descrições

## 📞 Suporte

**Desenvolvido por**: Eng. Klaus Q. Terra  
**Empresa**: Hiperenge  
**Versão**: 1.0.0  
**Data**: Setembro 2025  

### **Contato**
- **GitHub**: [klausterra/esp32c6_webserver_at](https://github.com/klausterra/esp32c6_webserver_at)
- **Documentação**: Veja `docs/` para mais detalhes
- **Issues**: Reporte problemas no GitHub

## 📄 Licença

Este projeto é propriedade da Hiperenge e foi desenvolvido pelo Eng. Klaus Q. Terra. Todos os direitos reservados.

---

**Maya Wi-Fi Zigbee Gateway** - Gateway Inteligente para IoT e Automação  
© 2025 Hiperenge - Eng. Klaus Q. Terra
