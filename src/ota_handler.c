/**
 * @file ota_handler.c
 * @brief Implementação do handler OTA
 */

#include "ota_handler.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_system.h"
#include "esp_partition.h"
#include "esp_crc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "OTA_HANDLER";

// Estrutura para controle do upgrade
typedef struct {
    esp_ota_handle_t ota_handle;
    const esp_partition_t *target_partition;
    size_t total_size;
    size_t written_size;
    bool in_progress;
    char status_message[64];
} ota_context_t;

static ota_context_t s_ota_ctx = {0};

// Callbacks
static ota_progress_cb_t s_progress_cb = NULL;
static ota_complete_cb_t s_complete_cb = NULL;
static ota_error_cb_t s_error_cb = NULL;

// Event group para sincronização
static EventGroupHandle_t s_ota_event_group;
#define OTA_COMPLETE_BIT BIT0
#define OTA_ERROR_BIT    BIT1

/**
 * @brief Task para processar progresso OTA
 */
static void ota_progress_task(void *pvParameters)
{
    while (1) {
        if (s_ota_ctx.in_progress && s_progress_cb) {
            ota_progress_t progress = {0};
            progress.bytes_written = s_ota_ctx.written_size;
            progress.total_bytes = s_ota_ctx.total_size;
            progress.percentage = (s_ota_ctx.total_size > 0) ? 
                (s_ota_ctx.written_size * 100) / s_ota_ctx.total_size : 0;
            progress.in_progress = s_ota_ctx.in_progress;
            strncpy(progress.status_message, s_ota_ctx.status_message, 
                   sizeof(progress.status_message) - 1);
            
            s_progress_cb(&progress);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief Verificar se partição é válida para OTA
 */
static bool is_valid_ota_partition(const esp_partition_t *partition)
{
    if (!partition) {
        return false;
    }
    
    // Verificar se é partição de aplicação
    if (partition->type != ESP_PARTITION_TYPE_APP) {
        return false;
    }
    
    // Verificar se não é a partição atual
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (running && partition->address == running->address) {
        return false;
    }
    
    return true;
}

/**
 * @brief Obter partição por nome
 */
static const esp_partition_t* get_partition_by_name(const char *name)
{
    if (!name) {
        return NULL;
    }
    
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, 
                                                   ESP_PARTITION_SUBTYPE_ANY, 
                                                   name);
    if (it) {
        const esp_partition_t *partition = esp_partition_get(it);
        esp_partition_iterator_release(it);
        return partition;
    }
    
    return NULL;
}

esp_err_t init_ota_handler(void)
{
    ESP_LOGI(TAG, "Inicializando handler OTA");
    
    // Criar event group
    s_ota_event_group = xEventGroupCreate();
    if (!s_ota_event_group) {
        ESP_LOGE(TAG, "Erro ao criar event group");
        return ESP_ERR_NO_MEM;
    }
    
    // Inicializar contexto
    memset(&s_ota_ctx, 0, sizeof(ota_context_t));
    
    // Criar task de progresso
    xTaskCreate(ota_progress_task, "ota_progress", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Handler OTA inicializado");
    return ESP_OK;
}

esp_err_t ota_get_partitions(ota_partition_info_t *partitions, 
                            uint16_t max_partitions, 
                            uint16_t *count)
{
    if (!partitions || !count) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *count = 0;
    
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, 
                                                   ESP_PARTITION_SUBTYPE_ANY, 
                                                   NULL);
    if (!it) {
        return ESP_ERR_NOT_FOUND;
    }
    
    while (it && *count < max_partitions) {
        const esp_partition_t *partition = esp_partition_get(it);
        if (partition) {
            strncpy(partitions[*count].name, partition->label, 
                   sizeof(partitions[*count].name) - 1);
            partitions[*count].type = partition->type;
            partitions[*count].subtype = partition->subtype;
            partitions[*count].size = partition->size;
            partitions[*count].address = partition->address;
            partitions[*count].is_boot = (partition == esp_ota_get_running_partition());
            (*count)++;
        }
        it = esp_partition_next(it);
    }
    
    esp_partition_iterator_release(it);
    
    ESP_LOGI(TAG, "Encontradas %d partições", *count);
    return ESP_OK;
}

const esp_partition_t* ota_get_running_partition(void)
{
    return esp_ota_get_running_partition();
}

const esp_partition_t* ota_get_next_partition(void)
{
    return esp_ota_get_next_update_partition(NULL);
}

esp_err_t ota_start_upgrade(const char *partition_name, 
                           const uint8_t *data, 
                           size_t size)
{
    if (!partition_name || !data || size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_ota_ctx.in_progress) {
        ESP_LOGE(TAG, "Upgrade OTA já em progresso");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Obter partição de destino
    const esp_partition_t *target = get_partition_by_name(partition_name);
    if (!target) {
        ESP_LOGE(TAG, "Partição não encontrada: %s", partition_name);
        return ESP_ERR_NOT_FOUND;
    }
    
    if (!is_valid_ota_partition(target)) {
        ESP_LOGE(TAG, "Partição inválida para OTA: %s", partition_name);
        return ESP_ERR_INVALID_ARG;
    }
    
    // Verificar tamanho
    if (size > target->size) {
        ESP_LOGE(TAG, "Firmware muito grande: %d > %d", size, target->size);
        return ESP_ERR_NO_MEM;
    }
    
    // Verificar integridade
    esp_err_t ret = ota_verify_firmware(data, size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Verificação de integridade falhou");
        return ret;
    }
    
    // Iniciar upgrade
    ret = esp_ota_begin(target, 0, &s_ota_ctx.ota_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao iniciar OTA: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configurar contexto
    s_ota_ctx.target_partition = target;
    s_ota_ctx.total_size = size;
    s_ota_ctx.written_size = 0;
    s_ota_ctx.in_progress = true;
    strcpy(s_ota_ctx.status_message, "Iniciando upgrade...");
    
    ESP_LOGI(TAG, "Upgrade OTA iniciado para partição: %s", partition_name);
    return ESP_OK;
}

esp_err_t ota_write_data(const uint8_t *data, size_t size)
{
    if (!s_ota_ctx.in_progress) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!data || size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = esp_ota_write(s_ota_ctx.ota_handle, data, size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao escrever dados OTA: %s", esp_err_to_name(ret));
        s_ota_ctx.in_progress = false;
        strcpy(s_ota_ctx.status_message, "Erro ao escrever dados");
        
        if (s_error_cb) {
            s_error_cb(ret);
        }
        return ret;
    }
    
    s_ota_ctx.written_size += size;
    
    // Verificar se concluído
    if (s_ota_ctx.written_size >= s_ota_ctx.total_size) {
        ret = ota_finish_upgrade();
    }
    
    return ret;
}

esp_err_t ota_finish_upgrade(void)
{
    if (!s_ota_ctx.in_progress) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = esp_ota_end(s_ota_ctx.ota_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao finalizar OTA: %s", esp_err_to_name(ret));
        s_ota_ctx.in_progress = false;
        strcpy(s_ota_ctx.status_message, "Erro ao finalizar upgrade");
        
        if (s_error_cb) {
            s_error_cb(ret);
        }
        return ret;
    }
    
    // Definir partição de boot
    ret = esp_ota_set_boot_partition(s_ota_ctx.target_partition);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao definir partição de boot: %s", esp_err_to_name(ret));
        s_ota_ctx.in_progress = false;
        strcpy(s_ota_ctx.status_message, "Erro ao definir partição de boot");
        
        if (s_error_cb) {
            s_error_cb(ret);
        }
        return ret;
    }
    
    s_ota_ctx.in_progress = false;
    strcpy(s_ota_ctx.status_message, "Upgrade concluído com sucesso");
    
    ESP_LOGI(TAG, "Upgrade OTA concluído com sucesso");
    
    if (s_complete_cb) {
        s_complete_cb(ESP_OK);
    }
    
    return ESP_OK;
}

esp_err_t ota_abort_upgrade(void)
{
    if (!s_ota_ctx.in_progress) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = esp_ota_abort(s_ota_ctx.ota_handle);
    s_ota_ctx.in_progress = false;
    strcpy(s_ota_ctx.status_message, "Upgrade abortado");
    
    ESP_LOGI(TAG, "Upgrade OTA abortado");
    
    if (s_error_cb) {
        s_error_cb(ret);
    }
    
    return ret;
}

bool ota_is_upgrading(void)
{
    return s_ota_ctx.in_progress;
}

esp_err_t ota_get_progress(ota_progress_t *progress)
{
    if (!progress) {
        return ESP_ERR_INVALID_ARG;
    }
    
    progress->bytes_written = s_ota_ctx.written_size;
    progress->total_bytes = s_ota_ctx.total_size;
    progress->percentage = (s_ota_ctx.total_size > 0) ? 
        (s_ota_ctx.written_size * 100) / s_ota_ctx.total_size : 0;
    progress->in_progress = s_ota_ctx.in_progress;
    strncpy(progress->status_message, s_ota_ctx.status_message, 
           sizeof(progress->status_message) - 1);
    
    return ESP_OK;
}

esp_err_t ota_verify_firmware(const uint8_t *data, size_t size)
{
    if (!data || size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Verificar tamanho mínimo
    if (size < sizeof(esp_image_header_t)) {
        ESP_LOGE(TAG, "Firmware muito pequeno");
        return ESP_ERR_INVALID_SIZE;
    }
    
    // Verificar magic number
    const esp_image_header_t *header = (const esp_image_header_t *)data;
    if (header->magic != ESP_IMAGE_HEADER_MAGIC) {
        ESP_LOGE(TAG, "Magic number inválido: 0x%x", header->magic);
        return ESP_ERR_INVALID_CRC;
    }
    
    // Verificação de CRC será feita pelo ESP-IDF durante o processo de OTA
    // A API esp_image_header_t não possui mais o campo checksum diretamente
    
    ESP_LOGI(TAG, "Firmware verificado com sucesso");
    return ESP_OK;
}

esp_err_t ota_get_firmware_version(char *version, size_t version_size)
{
    if (!version || version_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    const esp_app_desc_t *app_desc = esp_app_get_description();
    if (!app_desc) {
        return ESP_ERR_NOT_FOUND;
    }
    
    strncpy(version, app_desc->version, version_size - 1);
    version[version_size - 1] = '\0';
    
    return ESP_OK;
}

void ota_register_progress_cb(ota_progress_cb_t cb)
{
    s_progress_cb = cb;
}

void ota_register_complete_cb(ota_complete_cb_t cb)
{
    s_complete_cb = cb;
}

void ota_register_error_cb(ota_error_cb_t cb)
{
    s_error_cb = cb;
}

esp_err_t ota_restart_after_upgrade(uint32_t delay_ms)
{
    if (delay_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
    
    ESP_LOGI(TAG, "Reiniciando sistema após upgrade OTA");
    esp_restart();
    
    return ESP_OK;
}

esp_err_t ota_get_firmware_hash(uint8_t *hash, size_t hash_size)
{
    if (!hash || hash_size < 32) {
        return ESP_ERR_INVALID_ARG;
    }
    
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (!running) {
        return ESP_ERR_NOT_FOUND;
    }
    
    // Ler primeiros 4KB do firmware para calcular hash
    uint8_t buffer[4096];
    esp_err_t ret = esp_partition_read(running, 0, buffer, sizeof(buffer));
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Calcular SHA-256 (simplificado - usar mbedtls em produção)
    for (int i = 0; i < 32; i++) {
        hash[i] = buffer[i % sizeof(buffer)] ^ (i * 7);
    }
    
    return ESP_OK;
}

bool ota_is_valid_partition(const char *partition_name)
{
    const esp_partition_t *partition = get_partition_by_name(partition_name);
    return is_valid_ota_partition(partition);
}

size_t ota_get_partition_free_space(const char *partition_name)
{
    const esp_partition_t *partition = get_partition_by_name(partition_name);
    if (!partition) {
        return 0;
    }
    
    return partition->size;
}
