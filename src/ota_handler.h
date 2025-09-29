/**
 * @file ota_handler.h
 * @brief Handler OTA para ESP32-C6 Web Server AT
 * 
 * Este módulo implementa as funcionalidades de Over-The-Air (OTA)
 * upgrade para o ESP32-C6.
 */

#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include "esp_err.h"
#include "esp_ota_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

// Estrutura para informações de partição
typedef struct {
    char name[16];
    esp_partition_type_t type;
    esp_partition_subtype_t subtype;
    size_t size;
    size_t address;
    bool is_boot;
} ota_partition_info_t;

// Estrutura para progresso OTA
typedef struct {
    size_t bytes_written;
    size_t total_bytes;
    int percentage;
    bool in_progress;
    char status_message[64];
} ota_progress_t;

// Callbacks
typedef void (*ota_progress_cb_t)(const ota_progress_t *progress);
typedef void (*ota_complete_cb_t)(esp_err_t result);
typedef void (*ota_error_cb_t)(esp_err_t error);

/**
 * @brief Inicializar handler OTA
 * 
 * @return esp_err_t 
 */
esp_err_t init_ota_handler(void);

/**
 * @brief Obter informações de todas as partições
 * 
 * @param partitions Array para armazenar informações
 * @param max_partitions Tamanho máximo do array
 * @param count Ponteiro para número de partições encontradas
 * @return esp_err_t 
 */
esp_err_t ota_get_partitions(ota_partition_info_t *partitions, 
                            uint16_t max_partitions, 
                            uint16_t *count);

/**
 * @brief Obter partição atualmente em execução
 * 
 * @return const esp_partition_t* 
 */
const esp_partition_t* ota_get_running_partition(void);

/**
 * @brief Obter próxima partição OTA
 * 
 * @return const esp_partition_t* 
 */
const esp_partition_t* ota_get_next_partition(void);

/**
 * @brief Iniciar upgrade OTA
 * 
 * @param partition_name Nome da partição de destino
 * @param data Dados do firmware
 * @param size Tamanho dos dados
 * @return esp_err_t 
 */
esp_err_t ota_start_upgrade(const char *partition_name, 
                           const uint8_t *data, 
                           size_t size);

/**
 * @brief Escrever dados durante upgrade OTA
 * 
 * @param data Dados a serem escritos
 * @param size Tamanho dos dados
 * @return esp_err_t 
 */
esp_err_t ota_write_data(const uint8_t *data, size_t size);

/**
 * @brief Finalizar upgrade OTA
 * 
 * @return esp_err_t 
 */
esp_err_t ota_finish_upgrade(void);

/**
 * @brief Abortar upgrade OTA
 * 
 * @return esp_err_t 
 */
esp_err_t ota_abort_upgrade(void);

/**
 * @brief Verificar se upgrade está em progresso
 * 
 * @return true se em progresso
 */
bool ota_is_upgrading(void);

/**
 * @brief Obter progresso atual do upgrade
 * 
 * @param progress Ponteiro para estrutura de progresso
 * @return esp_err_t 
 */
esp_err_t ota_get_progress(ota_progress_t *progress);

/**
 * @brief Verificar integridade do firmware
 * 
 * @param data Dados do firmware
 * @param size Tamanho dos dados
 * @return esp_err_t 
 */
esp_err_t ota_verify_firmware(const uint8_t *data, size_t size);

/**
 * @brief Obter informações de versão do firmware
 * 
 * @param version Buffer para armazenar versão
 * @param version_size Tamanho do buffer
 * @return esp_err_t 
 */
esp_err_t ota_get_firmware_version(char *version, size_t version_size);

/**
 * @brief Registrar callback de progresso
 * 
 * @param cb Callback
 */
void ota_register_progress_cb(ota_progress_cb_t cb);

/**
 * @brief Registrar callback de conclusão
 * 
 * @param cb Callback
 */
void ota_register_complete_cb(ota_complete_cb_t cb);

/**
 * @brief Registrar callback de erro
 * 
 * @param cb Callback
 */
void ota_register_error_cb(ota_error_cb_t cb);

/**
 * @brief Reiniciar sistema após upgrade
 * 
 * @param delay_ms Delay antes do reinício (ms)
 * @return esp_err_t 
 */
esp_err_t ota_restart_after_upgrade(uint32_t delay_ms);

/**
 * @brief Obter hash do firmware atual
 * 
 * @param hash Buffer para armazenar hash
 * @param hash_size Tamanho do buffer
 * @return esp_err_t 
 */
esp_err_t ota_get_firmware_hash(uint8_t *hash, size_t hash_size);

/**
 * @brief Verificar se partição é válida para OTA
 * 
 * @param partition_name Nome da partição
 * @return true se válida
 */
bool ota_is_valid_partition(const char *partition_name);

/**
 * @brief Obter tamanho disponível na partição
 * 
 * @param partition_name Nome da partição
 * @return size_t Tamanho disponível
 */
size_t ota_get_partition_free_space(const char *partition_name);

#ifdef __cplusplus
}
#endif

#endif // OTA_HANDLER_H
