 /* Файл R_IR_service.h
 * Основан на ble_bas.h
 * Создан 2017.05.19
 * Написан Власовым Д.В. РЭМ-61
 */

#ifndef R_IR_SERVICE_H__
#define R_IR_SERVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*Типы событий сервиса, нотификация установлена или не установлена */
typedef enum
{
    R_IR_SERVICE_EVT_NOTIFICATION_ENABLED,                             /*Нотификация установлена */
    R_IR_SERVICE_EVT_NOTIFICATION_DISABLED                             /*Нотификация не установлена */
} R_IR_service_evt_type_t;                                             /*Перечисление нотификация*/

/*Событие сервиса */
typedef struct
{
    R_IR_service_evt_type_t evt_type;                                  /*Тип события */
} R_IR_service_evt_t;                                                  /*Событие сервиса*/

typedef struct R_IR_service_s R_IR_service_t;                            /*Переопределение названия структуры*/

/*Указатель для события сервиса R_IR */
typedef void (*R_IR_service_evt_handler_t) (R_IR_service_t * p_R_IR, R_IR_service_evt_t * p_evt);

/*Объявление для инициализации структуры R_IR сервиса. В структуре содержатся все данные и опции  для инициализации сервиса.*/
typedef struct
{
    R_IR_service_evt_handler_t    evt_handler;                    /*Указатель для хранения события сервиса R_IR*/
    bool                          support_notification;           /*Переменная для хранения информации о поддержке нотификации*/
    ble_srv_report_ref_t *        p_report_ref;                   /*Если не NULL то сервис поддерживает специальный дескриптор */
    uint32_t                      initial_IR_value;               /*Значение R*/
    uint32_t                      initial_R_value;                /*Значение IR*/
    ble_srv_cccd_security_mode_t  R_IR_level_char_attr_md;        /*Уровень защищённости сервиса батареи */
    ble_gap_conn_sec_mode_t       R_IR_level_report_read_perm;    /*Значение разрешения чтения*/
} R_IR_service_init_t;

/*Сервис R_IR. Структура содержит настройки сервиса и характеристики*/
struct R_IR_service_s
{
    R_IR_service_evt_handler_t    evt_handler;                    /*Указатель события сервиса*/
    uint16_t                      service_handle;                 /*Указатель на сервис батареи*/
    ble_gatts_char_handles_t      R_IR_level_handles;             /*Указатель на характеристику сервиса*/
    uint16_t                      report_ref_handle;              /*Указатель на дескриптор сервиса*/
    uint32_t                      IR_value_last;                  /*Последнее значение IR*/
    uint32_t                      R_value_last;                   /*Последнее значение R*/
    uint16_t                      conn_handle;                    /*Указатель на поддержку соединения*/
    bool                          is_notification_supported;      /*Указание на поддержку нотификации*/
};

/*Функция для инициализации сервиса R_IR.
 * p_R_IR_service Структура сервиса R_IR
 * p_R_IR_service_init  Структура инициализации сервиса R_IR.
 * Возвращаемое значение NRF_SUCCESS если инициализация сервиса произошла успешно.
 */
uint32_t R_IR_service_init(R_IR_service_t * p_R_IR_service, const R_IR_service_init_t * p_R_IR_service_init);

/* Функция для инициализации события сервиса R_IR
 * Замечания Если клиент отключился от сервиса, то перед переподключением необходимо
 * обновить значение при помощи функции R_IR_service_value_update
 * p_R_IR_service Указатель на сервис R_IR
 * p_ble_evt  Указатель на событие BLE стэка
 */

void R_IR_service_on_ble_evt(R_IR_service_t * p_R_IR_service, ble_evt_t * p_ble_evt);

/* Функция для обновления значения R_IR.
 * p_R_IR_service   Структура сервиса R_IR.
 * IR_value  Новое значение IR.
 * R_value   Новое значение R.
 * Возвращает NRF_SUCCESS если успешно обновлено значение.
 */
uint32_t R_IR_service_value_update(R_IR_service_t * p_R_IR_service, uint32_t IR_value, uint32_t R_value);


#ifdef __cplusplus
}
#endif

#endif // R_IR_SERVICE_H__

/** @} */
