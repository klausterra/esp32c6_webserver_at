/*
 * AT Commands Example - ESP32-C6 Web Server AT
 * 
 * Este exemplo demonstra como implementar comandos AT
 * para controle do servidor web ESP32-C6.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "nvs_flash.h"

static const char *TAG = "AT_COMMANDS";

// Configurações UART
#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024
#define AT_QUEUE_SIZE 10

// Estrutura para comandos AT
typedef struct {
    char command[64];
    char response[256];
} at_command_t;

// Fila para comandos AT
static QueueHandle_t at_queue;

/**
 * @brief Processar comando AT básico
 */
static void process_at_command(const char* command, char* response)
{
    if (strcmp(command, "AT") == 0) {
        strcpy(response, "OK");
    }
    else if (strcmp(command, "AT+RST") == 0) {
        strcpy(response, "OK");
        ESP_LOGI(TAG, "Reset solicitado via AT");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
    }
    else if (strcmp(command, "AT+GMR") == 0) {
        snprintf(response, 256, 
            "AT version:2.4.0.0\r\n"
            "SDK version:v4.3.2-rc1\r\n"
            "compile time:Sep 29 2025 17:06:36\r\n"
            "Bin version:2.4.0(ESP32-C6)\r\n"
            "OK");
    }
    else if (strncmp(command, "AT+CWMODE=", 10) == 0) {
        int mode = atoi(command + 10);
        if (mode >= 1 && mode <= 3) {
            strcpy(response, "OK");
            ESP_LOGI(TAG, "Modo Wi-Fi configurado: %d", mode);
        } else {
            strcpy(response, "ERROR");
        }
    }
    else if (strncmp(command, "AT+CWJAP=", 9) == 0) {
        // Extrair SSID e senha do comando
        char* ssid_start = strchr(command, '"') + 1;
        char* ssid_end = strchr(ssid_start, '"');
        char* pass_start = strchr(ssid_end + 1, '"') + 1;
        char* pass_end = strchr(pass_start, '"');
        
        if (ssid_start && ssid_end && pass_start && pass_end) {
            *ssid_end = '\0';
            *pass_end = '\0';
            
            ESP_LOGI(TAG, "Conectando ao Wi-Fi: %s", ssid_start);
            strcpy(response, "OK");
        } else {
            strcpy(response, "ERROR");
        }
    }
    else if (strcmp(command, "AT+CWQAP") == 0) {
        strcpy(response, "OK");
        ESP_LOGI(TAG, "Desconectando do Wi-Fi");
    }
    else if (strcmp(command, "AT+CWLAP") == 0) {
        snprintf(response, 256,
            "+CWLAP:(3,\"MinhaRede\",-45,\"aa:bb:cc:dd:ee:ff\",6)\r\n"
            "+CWLAP:(4,\"OutraRede\",-60,\"11:22:33:44:55:66\",11)\r\n"
            "OK");
    }
    else if (strncmp(command, "AT+CWSAP=", 9) == 0) {
        // Configurar SoftAP via AT
        strcpy(response, "OK");
        ESP_LOGI(TAG, "SoftAP configurado via AT");
    }
    else if (strcmp(command, "AT+CIFSR") == 0) {
        snprintf(response, 256,
            "+CIFSR:APIP,\"192.168.4.1\"\r\n"
            "+CIFSR:APMAC,\"54:32:04:3f:21:e4\"\r\n"
            "+CIFSR:STAIP,\"0.0.0.0\"\r\n"
            "+CIFSR:STAMAC,\"54:32:04:3f:21:e5\"\r\n"
            "OK");
    }
    else if (strcmp(command, "AT+HTTPSTART") == 0) {
        strcpy(response, "OK");
        ESP_LOGI(TAG, "Servidor web iniciado via AT");
    }
    else if (strcmp(command, "AT+HTTPSTOP") == 0) {
        strcpy(response, "OK");
        ESP_LOGI(TAG, "Servidor web parado via AT");
    }
    else if (strcmp(command, "AT+SYSTEMSTATUS") == 0) {
        snprintf(response, 256,
            "+SYSTEMSTATUS:online,192.168.4.1,%lu,%lu\r\n"
            "OK",
            esp_timer_get_time() / 1000000,
            esp_get_free_heap_size());
    }
    else if (strcmp(command, "AT+HWINFO") == 0) {
        snprintf(response, 256,
            "+HWINFO:ESP32-C6,v0.2,160MHz,4MB,54:32:04:3f:21:e4\r\n"
            "OK");
    }
    else if (strcmp(command, "AT+CONFIG") == 0) {
        snprintf(response, 256,
            "+CONFIG:pos_softap,espressif,11,3,80,25,1,0\r\n"
            "OK");
    }
    else {
        strcpy(response, "ERROR");
    }
}

/**
 * @brief Task para processar comandos AT
 */
static void at_command_task(void *pvParameters)
{
    at_command_t cmd;
    
    while (1) {
        if (xQueueReceive(at_queue, &cmd, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Processando comando: %s", cmd.command);
            process_at_command(cmd.command, cmd.response);
            ESP_LOGI(TAG, "Resposta: %s", cmd.response);
        }
    }
}

/**
 * @brief Task para receber dados UART
 */
static void uart_receive_task(void *pvParameters)
{
    uint8_t data[BUF_SIZE];
    char command[64];
    int command_len = 0;
    at_command_t cmd;
    
    while (1) {
        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, portMAX_DELAY);
        
        for (int i = 0; i < len; i++) {
            char c = data[i];
            
            if (c == '\r' || c == '\n') {
                if (command_len > 0) {
                    command[command_len] = '\0';
                    
                    // Copiar comando para estrutura
                    strcpy(cmd.command, command);
                    
                    // Enviar para fila de processamento
                    if (xQueueSend(at_queue, &cmd, 0) != pdTRUE) {
                        ESP_LOGE(TAG, "Fila de comandos AT cheia");
                    }
                    
                    command_len = 0;
                }
            } else if (command_len < sizeof(command) - 1) {
                command[command_len++] = c;
            }
        }
    }
}

/**
 * @brief Inicializar UART para comandos AT
 */
static void init_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, 
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

/**
 * @brief Aplicação principal
 */
void app_main(void)
{
    ESP_LOGI(TAG, "=== Exemplo de Comandos AT ===");
    
    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Criar fila para comandos AT
    at_queue = xQueueCreate(AT_QUEUE_SIZE, sizeof(at_command_t));
    if (at_queue == NULL) {
        ESP_LOGE(TAG, "Erro ao criar fila de comandos AT");
        return;
    }
    
    // Inicializar UART
    init_uart();
    
    // Criar tasks
    xTaskCreate(at_command_task, "at_command_task", 4096, NULL, 5, NULL);
    xTaskCreate(uart_receive_task, "uart_receive_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Sistema de comandos AT inicializado");
    ESP_LOGI(TAG, "Envie comandos AT via UART (115200 baud)");
    ESP_LOGI(TAG, "Exemplo: AT, AT+GMR, AT+CWMODE=3");
    
    // Loop principal
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "Sistema AT rodando - Comandos na fila: %d", uxQueueMessagesWaiting(at_queue));
    }
}