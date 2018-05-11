 /* Файл R_IR_service.c
 * Основан на ble_bas.c
 * Создан 2017.05.19
 * Написан Власовым Д.В. РЭМ-61
 */

#include "sdk_common.h"
#include "IR_service.h"
#include <string.h>
#include "ble_srv_common.h"

#define R_IR_SERVICE_UUID  0xAA55   //UUID сервиса
#define R_IR_CHARACTERISTIC 0x55AA  //UUID характеристики

/* Функция для события соединения с сервисом
 * p_R_IR_service Структура сервиса R_IR.
 * p_ble_evt      Событие приёма от BLE стэка.
 */
static void on_connect(R_IR_service_t * p_R_IR_service, ble_evt_t * p_ble_evt)
{
    p_R_IR_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/* Функция для события отсоединения от сервиса
 * p_R_IR_service Структура сервиса R_IR.
 * p_ble_evt      Событие приёма от BLE стэка.
 */
static void on_disconnect(R_IR_service_t * p_R_IR_service, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_R_IR_service->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/* Функция для события записи сервиса
 * p_R_IR_service Структура сервиса R_IR.
 * p_ble_evt      Событие приёма от BLE стэка.
 */
static void on_write(R_IR_service_t * p_R_IR_service, ble_evt_t * p_ble_evt)
{
    if (p_R_IR_service->is_notification_supported)
    {
        ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

        if (
            (p_evt_write->handle == p_R_IR_service->R_IR_level_handles.cccd_handle)
            &&
            (p_evt_write->len == 2)
           )
        {
            // Запись, вызов функции события для записи
            if (p_R_IR_service->evt_handler != NULL)
            {
                R_IR_service_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = R_IR_SERVICE_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = R_IR_SERVICE_EVT_NOTIFICATION_DISABLED;
                }

                p_R_IR_service->evt_handler(p_R_IR_service, &evt);
            }
        }
    }
}

/* Функция для определения текущего типа события и вызова функции для обработки события
 * p_R_IR_service Структура сервиса R_IR.
 * p_ble_evt      Событие приёма от BLE стэка.
*/
void R_IR_service_on_ble_evt(R_IR_service_t * p_R_IR_service, ble_evt_t * p_ble_evt)
{
    if (p_R_IR_service == NULL || p_ble_evt == NULL)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_R_IR_service, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_R_IR_service, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_R_IR_service, p_ble_evt);
            break;

        default:
            // Ничего не требуется.
            break;
    }
}

/* Функция для добавления характеристики сервиса
 * p_R_IR_service      Структура сервиса R_IR.
 * p_R_IR_service_init Структура инициализации сервиса R_IR.
 * Возвращаемое значение    NRF_SUCCESS если характеристика успешно добавлена к сервису.
 */
static uint32_t R_IR_level_char_add(R_IR_service_t * p_R_IR_service, const R_IR_service_init_t * p_R_IR_service_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint32_t            initial_IR_value;
    uint32_t            initial_R_value;
    uint8_t             encoded_report_ref[BLE_SRV_ENCODED_REPORT_REF_LEN];
    uint8_t             init_len;

    // Добавление характеристики в сервис
    if (p_R_IR_service->is_notification_supported)
    {
        memset(&cccd_md, 0, sizeof(cccd_md));

        // Добавление возможности чтения сервиса и характеристики
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        cccd_md.write_perm = p_R_IR_service_init->R_IR_level_char_attr_md.cccd_write_perm;
        cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify = (p_R_IR_service->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = (p_R_IR_service->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, R_IR_CHARACTERISTIC);

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_R_IR_service_init->R_IR_level_char_attr_md.read_perm;
    attr_md.write_perm = p_R_IR_service_init->R_IR_level_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    initial_IR_value = p_R_IR_service_init->initial_IR_value;
    initial_R_value = p_R_IR_service_init->initial_R_value;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    uint8_t p_initial_R_IR_value[6]={(initial_IR_value&0x00FF0000)>>16, (initial_IR_value&0x0000FF00)>>8, initial_IR_value&0xFF,(initial_R_value&0x00FF0000)>>16, (initial_R_value&0x0000FF00)>>8, initial_R_value&0xFF};
    
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 6*sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 6*sizeof(uint8_t);
    attr_char_value.p_value   = p_initial_R_IR_value;

    err_code = sd_ble_gatts_characteristic_add(p_R_IR_service->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_R_IR_service->R_IR_level_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (p_R_IR_service_init->p_report_ref != NULL)
    {
        // Добавление дескриптора
        BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_REPORT_REF_DESCR);

        memset(&attr_md, 0, sizeof(attr_md));

        attr_md.read_perm = p_R_IR_service_init->R_IR_level_report_read_perm;
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

        attr_md.vloc    = BLE_GATTS_VLOC_STACK;
        attr_md.rd_auth = 0;
        attr_md.wr_auth = 0;
        attr_md.vlen    = 0;

        init_len = ble_srv_report_ref_encode(encoded_report_ref, p_R_IR_service_init->p_report_ref);

        memset(&attr_char_value, 0, sizeof(attr_char_value));

        attr_char_value.p_uuid    = &ble_uuid;
        attr_char_value.p_attr_md = &attr_md;
        attr_char_value.init_len  = init_len;
        attr_char_value.init_offs = 0;
        attr_char_value.max_len   = attr_char_value.init_len;
        attr_char_value.p_value   = encoded_report_ref;

        err_code = sd_ble_gatts_descriptor_add(p_R_IR_service->R_IR_level_handles.value_handle,
                                               &attr_char_value,
                                               &p_R_IR_service->report_ref_handle);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    else
    {
        p_R_IR_service->report_ref_handle = BLE_GATT_HANDLE_INVALID;
    }

    return NRF_SUCCESS;
}

/* Функция для инициализации сервиса R_IR
 * p_R_IR_service      Структура сервиса R_IR.
 * p_R_IR_service_init Структура инициализации сервиса R_IR.
 * Возвращаемое значение    NRF_SUCCESS если сервис инициализирован.
 */
uint32_t R_IR_service_init(R_IR_service_t * p_R_IR_service, const R_IR_service_init_t * p_R_IR_service_init)
{
    if (p_R_IR_service == NULL || p_R_IR_service_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Инициализация структуры сервиса
    p_R_IR_service->evt_handler               = p_R_IR_service_init->evt_handler;
    p_R_IR_service->conn_handle               = BLE_CONN_HANDLE_INVALID;
    p_R_IR_service->is_notification_supported = p_R_IR_service_init->support_notification;
    p_R_IR_service->IR_value_last             = 0x00;
    p_R_IR_service->R_value_last              = 0x00;
    
    // Добавление сервиса
    BLE_UUID_BLE_ASSIGN(ble_uuid, R_IR_SERVICE_UUID);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_R_IR_service->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Добавление характеристики сервиса
    return R_IR_level_char_add(p_R_IR_service, p_R_IR_service_init);
}

/* Функция для обновления значения характеристики сервиса R_IR
 * p_R_IR_service      Структура сервиса R_IR.
 * IR_value            Новое значение IR
 * R_value             Новое значение R
 * Возвращаемое значение    NRF_SUCCESS если характеристика обновлена.
 */
uint32_t R_IR_service_value_update(R_IR_service_t * p_R_IR_service, uint32_t IR_value, uint32_t R_value)
{
    if (p_R_IR_service == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    if ((IR_value != p_R_IR_service->IR_value_last)|(R_value != p_R_IR_service->R_value_last))
    {
        // Инициализация значения структуры
        memset(&gatts_value, 0, sizeof(gatts_value));
        
        uint8_t p_R_IR_level[6]={(IR_value&0x00FF0000)>>16, (IR_value&0x0000FF00)>>8, IR_value&0xFF,(R_value&0x00FF0000)>>16, (R_value&0x0000FF00)>>8, R_value&0xFF};
        
        gatts_value.len     = 6*sizeof(uint8_t);
        gatts_value.offset  = 0;
        gatts_value.p_value = p_R_IR_level;

        // Обновление данных
        err_code = sd_ble_gatts_value_set(p_R_IR_service->conn_handle,
                                          p_R_IR_service->R_IR_level_handles.value_handle,
                                          &gatts_value);
        if (err_code == NRF_SUCCESS)
        {
            // Сохранение нового значения
            p_R_IR_service->IR_value_last = IR_value;
            p_R_IR_service->R_value_last = R_value;
        }
        else
        {
            return err_code;
        }

        // Отправка значения если есть подключение и установлена нотификация
        if ((p_R_IR_service->conn_handle != BLE_CONN_HANDLE_INVALID) && p_R_IR_service->is_notification_supported)
        {
            ble_gatts_hvx_params_t hvx_params;

            memset(&hvx_params, 0, sizeof(hvx_params));

            hvx_params.handle = p_R_IR_service->R_IR_level_handles.value_handle;
            hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset = gatts_value.offset;
            hvx_params.p_len  = &gatts_value.len;
            hvx_params.p_data = gatts_value.p_value;

            err_code = sd_ble_gatts_hvx(p_R_IR_service->conn_handle, &hvx_params);
        }
        else
        {
            err_code = NRF_ERROR_INVALID_STATE;
        }
    }

    return err_code;
}