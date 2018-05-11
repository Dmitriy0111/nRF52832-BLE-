/*******************************************************************
 * Имя файла: main.c
 * Язык: C++
 * Автор: Власов Д.В., РЭМ-61
 * Дата создания: 01.09.2017
 * Среда разработки: IAR Embedded Workbench for ARM 8.11
*******************************************************************/


#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "fstorage.h"
#include "fds.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "nrf_gpio.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"


#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
   
#include "nrf_drv_uart.h" //Подключение файла для UART
#include "nrf_drv_spi.h" //Подключение файла для UART
#include "nrf_delay.h" //Подключение файла для UART
#include "nrf_drv_twi.h"  //Подключение файла для I2C
#include "app_uart.h"     //Подключение файла для UART
#include "AFE4400REGS.h"  //Подключение файла для AFE4400
#include "ble_bas.h"      //Подключение файла для сервиса батареи
#include "IR_service.h"   //Подклчюение сервиса для собственного сервиса
static ble_bas_t m_bas;                                   /**< Structure used to identify the battery service. */
static R_IR_service_t m_R_IR_service;                                   /**< Structure used to identify the battery service. */


#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2    /**< Reply when unsupported features are requested. */

#define DEVICE_NAME                     "NordicPulseOxymeter"                   /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "NordicSemiconductor"                   /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                300                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                     /**< The advertising timeout in units of seconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(10, UNIT_0_625_MS)        /**< Minimum acceptable connection interval (0.1 seconds). */ //Изменённый интервал 20170522
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_0_625_MS)        /**< Maximum acceptable connection interval (0.2 second). */  //Изменённый интервал 20170522
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                       /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                      /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

static uint16_t       m_conn_handle = BLE_CONN_HANDLE_INVALID;                  /**< Handle of the current connection. */
static nrf_ble_gatt_t m_gatt;                                                   /**< GATT module instance. */

#define R_IR_LEVEL_MEAS_INTERVAL      APP_TIMER_TICKS(7.5)                      /**< Battery level measurement interval (ticks). */   //Изменённый интервал 20170522
APP_TIMER_DEF(m_R_IR_timer_id);                        /**< Battery timer. */

union R_value
{
  uint32_t R32;
  uint8_t  R8[4];
};
union IR_value
{
  uint32_t IR32;
  uint8_t  IR8[4];
};

union R_value R_tx;
union IR_value IR_tx;

/* YOUR_JOB: Declare all services structure your application is using
   static ble_xx_service_t                     m_xxs;
   static ble_yy_service_t                     m_yys;
 */



// YOUR_JOB: Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE},
                                   {BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */

static void advertising_start(bool erase_bonds);

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            NRF_LOG_INFO("Connected to a previously bonded device.\r\n");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.\r\n",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            advertising_start(false);
        } break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        {
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}

static void battery_level_meas_timeout_handler(void * p_context)
{
    nrf_gpio_pin_write(5,1);
    UNUSED_PARAMETER(p_context);
    //R_tx.R8[0]+=1;
    //IR_tx.IR8[0]+=1;
    R_IR_service_value_update(&m_R_IR_service,R_tx.R32,IR_tx.IR32 );
    nrf_gpio_pin_write(5,0);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_create(&m_R_IR_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
       ret_code_t err_code;
       err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
       APP_ERROR_CHECK(err_code); */
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the YYY Service events.
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
static void on_yys_evt(ble_yy_service_t     * p_yy_service,
                       ble_yy_service_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. \r\n", p_evt->params.char_xx.value.p_str);
            break;

        default:
            // No implementation needed.
            break;
    }
}
*/

/**@brief Function for initializing services that will be used by the application.
 */

static void services_init(void)
{
    ret_code_t     err_code;
    
    //batservice init

    ble_bas_init_t bas_init;

    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);
    
    //IRserviceinit
    R_IR_service_init_t p_R_IR_service_init;

    // Initialize Battery Service.
    memset(&p_R_IR_service_init, 0, sizeof(p_R_IR_service_init));

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&p_R_IR_service_init.R_IR_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&p_R_IR_service_init.R_IR_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&p_R_IR_service_init.R_IR_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&p_R_IR_service_init.R_IR_level_report_read_perm);

    p_R_IR_service_init.evt_handler          = NULL;
    p_R_IR_service_init.support_notification = true;
    p_R_IR_service_init.p_report_ref         = NULL;
    p_R_IR_service_init.initial_IR_value   = 0xE055AA;
    p_R_IR_service_init.initial_R_value   = 0xB0FF77;

    err_code = R_IR_service_init(&m_R_IR_service, &p_R_IR_service_init);
    APP_ERROR_CHECK(err_code);
}

//static void services_init(void)
//{
    /* YOUR_JOB: Add code to initialize the services used by the application.
       ret_code_t                         err_code;
       ble_xxs_init_t                     xxs_init;
       ble_yys_init_t                     yys_init;

       // Initialize XXX Service.
       memset(&xxs_init, 0, sizeof(xxs_init));

       xxs_init.evt_handler                = NULL;
       xxs_init.is_xxx_notify_supported    = true;
       xxs_init.ble_xx_initial_value.level = 100;

       err_code = ble_bas_init(&m_xxs, &xxs_init);
       APP_ERROR_CHECK(err_code);

       // Initialize YYY Service.
       memset(&yys_init, 0, sizeof(yys_init));
       yys_init.evt_handler                  = on_yys_evt;
       yys_init.ble_yy_initial_value.counter = 0;

       err_code = ble_yy_service_init(&yys_init, &yy_init);
       APP_ERROR_CHECK(err_code);
     */
//}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
 */
static void application_timers_start(void)
{
  //ret_code_t err_code;

    // Start application timers.
    //err_code = app_timer_start(m_R_IR_timer_id, R_IR_LEVEL_MEAS_INTERVAL, NULL);
    //APP_ERROR_CHECK(err_code);
    /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
       ret_code_t err_code;
       err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
       APP_ERROR_CHECK(err_code); */

}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.\r\n");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        default:
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    ret_code_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected.\r\n");
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.\r\n");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTC_EVT_TIMEOUT

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_TIMEOUT

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_EVT_USER_MEM_REQUEST

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    /** The Connection state module has to be fed BLE events in order to function correctly
     * Remember to call ble_conn_state_on_ble_evt before calling any ble_conns_state_* functions. */
    ble_conn_state_on_ble_evt(p_ble_evt);
    pm_on_ble_evt(p_ble_evt);
    
    R_IR_service_on_ble_evt(&m_R_IR_service, p_ble_evt);
    
    ble_conn_params_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    nrf_ble_gatt_on_ble_evt(&m_gatt, p_ble_evt);
    
    /*YOUR_JOB add calls to _on_ble_evt functions from each service your application is using
       ble_xxs_on_ble_evt(&m_xxs, p_ble_evt);
       ble_yys_on_ble_evt(&m_yys, p_ble_evt);
     */
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    // Dispatch the system event to the fstorage module, where it will be
    // dispatched to the Flash Data Storage (FDS) module.
    fs_sys_event_handler(sys_evt);

    // Dispatch to the Advertising module last, since it will check if there are any
    // pending flash operations in fstorage. Let fstorage process system events first,
    // so that it can report correctly to the Advertising module.
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = softdevice_app_ram_start_get(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Overwrite some of the default configurations for the BLE stack.
    ble_cfg_t ble_cfg;

    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 0;
    err_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum number of connections.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = BLE_GAP_ROLE_COUNT_PERIPH_DEFAULT;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = softdevice_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!\r\n");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated when button is pressed.
 */
static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break; // BSP_EVENT_SLEEP

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break; // BSP_EVENT_DISCONNECT

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist();
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break; // BSP_EVENT_KEY_0

        default:
            break;
    }
}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advdata_t          advdata;
    ble_adv_modes_config_t options;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    ret_code_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting advertising.
 */
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED evetnt
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(BLE_ADV_MODE_FAST);

        APP_ERROR_CHECK(err_code);
    }
}




/****************************************************************************
* PortInt
* @Описание: Функция для инициализации вывода прерывания nRF52832
* @Заметки: Данная функция настраивает порт 4 для работы в режиме прерывания
* по спаду сигнала
*****************************************************************************/

void PortInt(void)
{
  nrf_gpio_cfg_input(12, NRF_GPIO_PIN_NOPULL);

  // Enable sense on the Menu button
  nrf_gpio_cfg_sense_input(12, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);

  // Set the GPIOTE PORT event as interrupt source
  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;

  // Enable interrupts for GPIOTE
  NVIC_EnableIRQ(GPIOTE_IRQn);
}



/**@brief Function for application main entry.
 */
//nrf_drv_twi_config_t pI2C=NRF_DRV_TWI_DEFAULT_CONFIG;

//const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(TWI0_INSTANCE_INDEX);  /**< SPI instance. */

uint8_t Rnew[4], IRnew[4];
//uint8_t txtx[10]={0x55,0x55,0x55,0x55};
uint8_t PWR=0x00;
extern nrf_drv_uart_t app_uart_inst=NRF_DRV_UART_INSTANCE(APP_UART_DRIVER_INSTANCE);

/****************************************************************************
* MAX30101Init
* @Описание: Функция для инициализации микросхемы MAX30101
* @Заметки: Данная функция предназначена конфигурации микросхемы MAX30101
* в режим работы SPo2
****************************************************************************/

/*void MAX30101Init(void)
{
  
  uint8_t initialData[2];

  uint8_t ID=0x00;
  while (ID!=0x15)
  {
    ID=0xFF;
    nrf_drv_twi_tx(&twi,0x57,&ID,1,false);
    nrf_drv_twi_rx(&twi,0x57,&ID,1);
  }
  PWR=1;

  initialData[0]=0x0C;
  initialData[1]=0x1F;
  nrf_drv_twi_tx(&twi,0x57,initialData,2,false); //RLedCurrent 3.1 mA
  initialData[0]=0x0D;
  initialData[1]=0x1F;
  nrf_drv_twi_tx(&twi,0x57,initialData,2,false); //IRLedCurrent 3.1 mA

  initialData[0]=0x0A;
  initialData[1]=0x63;
  nrf_drv_twi_tx(&twi,0x57,initialData,2,false); //SpO2: PulseWidth=118 us & 50 sapmles per second
  initialData[0]=0x09;
  initialData[1]=0x03;
  nrf_drv_twi_tx(&twi,0x57,initialData,2,false); //SpO2 Mode Enable
  initialData[0]=0x02;
  initialData[1]=0x40;
  nrf_drv_twi_tx(&twi,0x57,initialData,2,false); //SpO2 InterruptEnable
  initialData[0]=0x03;
  initialData[1]=0x02;
  nrf_drv_twi_tx(&twi,0x57,initialData,2,false); //SpO2 InterruptEnable
}*/
nrf_drv_spi_config_t pSPI1=NRF_DRV_SPI_DEFAULT_CONFIG;
const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI0_INSTANCE_INDEX);  /**< SPI instance. */

/****************************************************************************
* GPIOTE_IRQHandler
* @Описание: Функция прерывания выводов nRF52832
* @Заметки: Данная функция вызывается всякий раз, когда происходит переход
* состояния вывода 4 из '1' в '0'
****************************************************************************/
uint8_t ha=0;
void GPIOTE_IRQHandler(void) {
  // If PORT event has been called, clear event and toggle LED
  if (NRF_GPIOTE->EVENTS_PORT) {
    nrf_gpio_pin_write(19,1);
    NRF_GPIOTE->EVENTS_PORT = 12;
    uint8_t       AFE_tx_buf[4];
    AFE_tx_buf[0]=0x2A;
    AFE_tx_buf[1]=0x00;
    AFE_tx_buf[2]=0x00;
    AFE_tx_buf[3]=0x00;
    nrf_drv_spi_transfer(&spi, AFE_tx_buf, 4, Rnew, 4);
    AFE_tx_buf[0]=0x2B;
    nrf_drv_spi_transfer(&spi, AFE_tx_buf, 4, IRnew, 4); 
    
    R_tx.R8[0]=Rnew[3];
    R_tx.R8[1]=Rnew[2];
    R_tx.R8[2]=Rnew[1];
    IR_tx.IR8[0]=IRnew[3];
    IR_tx.IR8[1]=IRnew[2];
    IR_tx.IR8[2]=IRnew[1];
    /*R_tx.R8[0]=ha;
    R_tx.R8[1]=ha;
    R_tx.R8[2]=ha;
    IR_tx.IR8[0]=ha;
    IR_tx.IR8[1]=ha;
    IR_tx.IR8[2]=ha;
    ha++;*/
    R_IR_service_value_update(&m_R_IR_service,R_tx.R32,IR_tx.IR32 );
    nrf_gpio_pin_write(19,0);
  }
}
void uart_event_handle(app_uart_evt_t * p_event){}

/****************************************************************************
* MyUartInit
* @Описание: Функция инициализации UART интерфейса nRF52832
* @Заметки: Функция используемая для настройки UART интерфейса nRF52832
****************************************************************************/
void MyUartInit(void)
{
  uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                       1,
                       1,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}




/****************************************************************************
* MyI2CInit
* @Описание: Функция инициализации I2C интерфейса nRF52832
* @Заметки: Данная функция настраивает выводы и I2C интерфейс nRF52832 для
* работы с микросхемой MAX30101
****************************************************************************/
/*void MyI2CInit(void)
{
  pI2C.scl=19;
  pI2C.sda=17;
  pI2C.frequency=(nrf_twi_frequency_t)104857600;
  ret_code_t err_code =nrf_drv_twi_init(&twi, &pI2C, NULL, NULL);
  
  APP_ERROR_CHECK(err_code);
  nrf_drv_twi_enable(&twi);
}*/



void MySPIInit(void)
{
  pSPI1.frequency=NRF_DRV_SPI_FREQ_8M;
  pSPI1.mode=NRF_DRV_SPI_MODE_0;
  pSPI1.bit_order=NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
  pSPI1.ss_pin=13;
  pSPI1.miso_pin=15;
  pSPI1.mosi_pin=14;
  pSPI1.sck_pin=16;
  ret_code_t err_code =nrf_drv_spi_init(&spi, &pSPI1, NULL, NULL);
  APP_ERROR_CHECK(err_code);
  nrf_gpio_cfg_output(11);
  nrf_gpio_cfg_output(18);
  nrf_gpio_cfg_output(19);
  nrf_gpio_pin_write(11,1);
  nrf_gpio_pin_write(18,1);
}

void AFE4400Init()
{
  uint8_t       AFE_tx_buf[4];
  for (uint8_t i=0;i<=35;i++)
  {
      AFE_tx_buf[0]=i;
      AFE_tx_buf[1]=AFE4400_REGS[i]>>16;
      AFE_tx_buf[2]=(AFE4400_REGS[i]&0x00FF00)>>8;
      AFE_tx_buf[3]=AFE4400_REGS[i]&0x0000FF;
      nrf_drv_spi_transfer(&spi, AFE_tx_buf, 4, NULL, 4);
  }
  AFE_tx_buf[0]=0x00;
  AFE_tx_buf[1]=0x00;
  AFE_tx_buf[2]=0x00;
  AFE_tx_buf[3]=0x01;
  nrf_drv_spi_transfer(&spi, AFE_tx_buf, 4, NULL, 4);
}


int main(void)
{
    bool erase_bonds;
nrf_delay_ms(1000);
    
    // Initialize.
    
    timers_init();
    buttons_leds_init(&erase_bonds);
    MySPIInit();
    //MyI2CInit();
    AFE4400Init();
    MyUartInit();
    log_init();
    
    ble_stack_init();
    gap_params_init();
    gatt_init();
    advertising_init();
    services_init();
    conn_params_init();
    peer_manager_init();
    

    // Start execution.
    NRF_LOG_INFO("Template example started.\r\n");
    application_timers_start();

    advertising_start(erase_bonds);
    //sd_ble_gap_tx_power_set(-20);
    PortInt();
   
    
    //uint8_t ID=0xFF;
    //nrf_drv_twi_tx(&twi,0x57,&ID,1,false);
    //nrf_drv_twi_rx(&twi,0x57,&ID,1);
    //ID=0x00;
    // Enter main loop.
    for (;;)
    {
      
        if (NRF_LOG_PROCESS() == false)
        {
            power_manage();
        }
    }
}


/**
 * @}
 */
