#include <stdint.h>
#include <string.h>
#include <math.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "app_error.h"
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"
#include "ble_advdata.h"
#include "ble_hids.h"
#include "ble_conn_params.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "peer_manager.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "peer_manager_handler.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "utils.h"
#include "hid_keymap.h"

#define OUTPUT_REPORT_INDEX                 0                                          /**< Index of Output Report. */
#define OUTPUT_REPORT_MAX_LEN               1                                          /**< Maximum length of Output Report. */
#define INPUT_REPORT_KEYS_INDEX             0                                          /**< Index of Input Report. */
#define INPUT_REP_REF_ID                    0                                          /**< Id of reference to Keyboard Input Report. */
#define OUTPUT_REP_REF_ID                   0                                          /**< Id of reference to Keyboard Output Report. */
#define FEATURE_REP_REF_ID                  0                                          /**< ID of reference to Keyboard Feature Report. */
#define FEATURE_REPORT_MAX_LEN              2                                          /**< Maximum length of Feature Report. */
#define FEATURE_REPORT_INDEX                0                                          /**< Index of Feature Report. */

#define MAX_BUFFER_ENTRIES                  20                                         /**< Number of elements that can be enqueued */

#define BASE_USB_HID_SPEC_VERSION           0x0101                                     /**< Version number of base USB HID Specification implemented by this application. */

#define INPUT_REPORT_KEYS_MAX_LEN           8                                          /**< Maximum length of the Input Report characteristic. */

#define MODIFIER_KEY_POS                    0                                          /**< Position of the modifier byte in the Input Report. */
#define SCAN_CODE_POS                       2                                          /**< The start position of the key scan code in a HID Report. */
#define SHIFT_KEY_CODE                      0x02                                       /**< Key code indicating the press of the Shift Key. */

#define MAX_KEYS_IN_ONE_REPORT              (INPUT_REPORT_KEYS_MAX_LEN - SCAN_CODE_POS)/**< Maximum number of key presses that can be sent in one Input Report. */

BLE_HIDS_DEF(m_hids,                                                /**< Structure used to identify the HID service. */
             NRF_SDH_BLE_TOTAL_LINK_COUNT,
             INPUT_REPORT_KEYS_MAX_LEN,
             OUTPUT_REPORT_MAX_LEN,
             FEATURE_REPORT_MAX_LEN);

#define HID_TX_TIMER_INTERVAL         APP_TIMER_TICKS(1)            /**< Timer fires every 1 milliseconds */
APP_TIMER_DEF(m_hid_tx_timer_id);                                   /**< HID transmission timer. */

static uint8_t**         m_tx_key_events = NULL;
static uint8_t           m_tx_key_event_count = 0;
static uint8_t           m_tx_key_offset = 0;                        /**< char offset of the current transmission */

static bool              m_in_boot_mode = false;                    /**< Current protocol mode. */
static uint16_t          m_conn_handle  = BLE_CONN_HANDLE_INVALID;  /**< Handle of the current connection. */

/*
  Nützliche Links:
  https://wiki.micropython.org/USB-HID-Keyboard-mode-example-a-password-dongle
  https://github.com/NicoHood/HID
  https://www.usb.org/hid
*/

// forward declarations
static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt);
static void hid_tx_timeout_handler();

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code;

    // Create HID transmission timer.
    err_code = app_timer_create(&m_hid_tx_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                hid_tx_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

static void hid_tx_timer_start(void)
{
    NRF_LOG_INFO("hid_tx_timer_start()");
    ret_code_t err_code;

    err_code = app_timer_start(m_hid_tx_timer_id, HID_TX_TIMER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

static void hid_tx_timer_stop(void)
{
    ret_code_t err_code;

    err_code = app_timer_stop(m_hid_tx_timer_id);
    APP_ERROR_CHECK(err_code);
}

static void hid_tx_timeout_handler()
{
      if (m_tx_key_offset == sizeof(m_tx_key_event_count))
      {
          NRF_LOG_INFO("Transfer completed.");
          // transfer completed
          hid_tx_timer_stop();
          m_tx_key_offset = 0;
          m_tx_key_event_count = 0;
          if (m_tx_key_events != NULL)
          {
              free(m_tx_key_events);
          }
          return;
      }

      if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
      {
          //NRF_LOG_INFO("Transmission....");
          uint8_t* input_report = m_tx_key_events[m_tx_key_offset];
          
          ret_code_t err_code;
          err_code = ble_hids_inp_rep_send(&m_hids,
                                           INPUT_REPORT_KEYS_INDEX,
                                           INPUT_REPORT_KEYS_MAX_LEN,
                                           input_report,
                                           m_conn_handle);
          if ((err_code == NRF_ERROR_RESOURCES) && (m_tx_key_offset <= m_tx_key_event_count))
          {
              // Transmission could not be completed, so retransmit next time
              //NRF_LOG_INFO("NRF_ERROR_RESOURCES");
          }
          else
          {
              // transfer next key
              m_tx_key_offset++;
              //NRF_LOG_INFO("transfer next key");
          }
      }
}


/**@brief Function for handling the HID Report Characteristic Write event.
 *
 * @param[in]   p_evt   HID service event.
 */
static void on_hid_rep_char_write(ble_hids_evt_t * p_evt)
{
    NRF_LOG_INFO("on_hid_rep_char_write()");
    if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT)
    {
        ret_code_t err_code;
        uint8_t  report_val;
        uint8_t  report_index = p_evt->params.char_write.char_id.rep_index;

        if (report_index == OUTPUT_REPORT_INDEX)
        {
            // This code assumes that the output report is one byte long. Hence the following
            // static assert is made.
            STATIC_ASSERT(OUTPUT_REPORT_MAX_LEN == 1);

            err_code = ble_hids_outp_rep_get(&m_hids,
                                             report_index,
                                             OUTPUT_REPORT_MAX_LEN,
                                             0,
                                             m_conn_handle,
                                             &report_val);
            APP_ERROR_CHECK(err_code);

            //keys_send(m_sample_keys_str, sizeof(m_sample_keys_str));
        }
    }
}


/**@brief Function for handling HID events.
 *
 * @details This function will be called for all HID events which are passed to the application.
 *
 * @param[in]   p_hids  HID service structure.
 * @param[in]   p_evt   Event received from the HID service.
 */
static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt)
{
    NRF_LOG_INFO("on_hids_evt()");
    switch (p_evt->evt_type)
    {
        case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
            NRF_LOG_INFO("HID boot mode entered.");
            m_in_boot_mode = true;
            break;

        case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
            NRF_LOG_INFO("HID report mode entered.");
            m_in_boot_mode = false;
            break;

        case BLE_HIDS_EVT_REP_CHAR_WRITE:
            NRF_LOG_INFO("HID report char write.");
            on_hid_rep_char_write(p_evt);
            break;

        case BLE_HIDS_EVT_NOTIF_ENABLED:
            NRF_LOG_INFO("HID notification enabled.");
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error)
{
    NRF_LOG_INFO("service_error_handler()");
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing HID Service.
 */
void hids_init(uint16_t a_conn_handle)
{
    NRF_LOG_INFO("hids_init()");

    //buffer_init();

    m_conn_handle = a_conn_handle;

    ret_code_t                    err_code;
    ble_hids_init_t               hids_init_obj;
    ble_hids_inp_rep_init_t     * p_input_report;
    ble_hids_outp_rep_init_t    * p_output_report;
    ble_hids_feature_rep_init_t * p_feature_report;
    uint8_t                       hid_info_flags;

    static ble_hids_inp_rep_init_t     input_report_array[1];
    static ble_hids_outp_rep_init_t    output_report_array[1];
    static ble_hids_feature_rep_init_t feature_report_array[1];
    static uint8_t                     report_map_data[] =
    {
        0x05, 0x01,       // Usage Page (Generic Desktop)
        0x09, 0x06,       // Usage (Keyboard)
        0xA1, 0x01,       // Collection (Application)
        0x05, 0x07,       // Usage Page (Key Codes)
        0x19, 0xe0,       // Usage Minimum (224)
        0x29, 0xe7,       // Usage Maximum (231)
        0x15, 0x00,       // Logical Minimum (0)
        0x25, 0x01,       // Logical Maximum (1)
        0x75, 0x01,       // Report Size (1)
        0x95, 0x08,       // Report Count (8)
        0x81, 0x02,       // Input (Data, Variable, Absolute)

        0x95, 0x01,       // Report Count (1)
        0x75, 0x08,       // Report Size (8)
        0x81, 0x01,       // Input (Constant) reserved byte(1)

        0x95, 0x05,       // Report Count (5)
        0x75, 0x01,       // Report Size (1)
        0x05, 0x08,       // Usage Page (Page# for LEDs)
        0x19, 0x01,       // Usage Minimum (1)
        0x29, 0x05,       // Usage Maximum (5)
        0x91, 0x02,       // Output (Data, Variable, Absolute), Led report
        0x95, 0x01,       // Report Count (1)
        0x75, 0x03,       // Report Size (3)
        0x91, 0x01,       // Output (Data, Variable, Absolute), Led report padding

        0x95, 0x06,       // Report Count (6)
        0x75, 0x08,       // Report Size (8)
        0x15, 0x00,       // Logical Minimum (0)
        0x25, 0x65,       // Logical Maximum (101)
        0x05, 0x07,       // Usage Page (Key codes)
        0x19, 0x00,       // Usage Minimum (0)
        0x29, 0x65,       // Usage Maximum (101)
        0x81, 0x00,       // Input (Data, Array) Key array(6 bytes)

        0x09, 0x05,       // Usage (Vendor Defined)
        0x15, 0x00,       // Logical Minimum (0)
        0x26, 0xFF, 0x00, // Logical Maximum (255)
        0x75, 0x08,       // Report Size (8 bit)
        0x95, 0x02,       // Report Count (2)
        0xB1, 0x02,       // Feature (Data, Variable, Absolute)

        0xC0              // End Collection (Application)
    };

    memset((void *)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void *)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));
    memset((void *)feature_report_array, 0, sizeof(ble_hids_feature_rep_init_t));

    // Initialize HID Service
    p_input_report                      = &input_report_array[INPUT_REPORT_KEYS_INDEX];
    p_input_report->max_len             = INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = INPUT_REP_REF_ID;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    p_input_report->sec.cccd_wr = SEC_JUST_WORKS;
    p_input_report->sec.wr      = SEC_JUST_WORKS;
    p_input_report->sec.rd      = SEC_JUST_WORKS;

    p_output_report                      = &output_report_array[OUTPUT_REPORT_INDEX];
    p_output_report->max_len             = OUTPUT_REPORT_MAX_LEN;
    p_output_report->rep_ref.report_id   = OUTPUT_REP_REF_ID;
    p_output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

    p_output_report->sec.wr = SEC_JUST_WORKS;
    p_output_report->sec.rd = SEC_JUST_WORKS;

    p_feature_report                      = &feature_report_array[FEATURE_REPORT_INDEX];
    p_feature_report->max_len             = FEATURE_REPORT_MAX_LEN;
    p_feature_report->rep_ref.report_id   = FEATURE_REP_REF_ID;
    p_feature_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_FEATURE;

    p_feature_report->sec.rd              = SEC_JUST_WORKS;
    p_feature_report->sec.wr              = SEC_JUST_WORKS;

    hid_info_flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler                    = on_hids_evt;
    hids_init_obj.error_handler                  = service_error_handler;
    hids_init_obj.is_kb                          = true;
    hids_init_obj.is_mouse                       = false;
    hids_init_obj.inp_rep_count                  = 1;
    hids_init_obj.p_inp_rep_array                = input_report_array;
    hids_init_obj.outp_rep_count                 = 1;
    hids_init_obj.p_outp_rep_array               = output_report_array;
    hids_init_obj.feature_rep_count              = 1;
    hids_init_obj.p_feature_rep_array            = feature_report_array;
    hids_init_obj.rep_map.data_len               = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data                 = report_map_data;
    hids_init_obj.hid_information.bcd_hid        = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags          = hid_info_flags;
    hids_init_obj.included_services_count        = 0;
    hids_init_obj.p_included_services_array      = NULL;

    hids_init_obj.rep_map.rd_sec         = SEC_JUST_WORKS;
    hids_init_obj.hid_information.rd_sec = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_inp_rep_sec.cccd_wr = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_inp_rep_sec.rd      = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_outp_rep_sec.rd = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_outp_rep_sec.wr = SEC_JUST_WORKS;

    hids_init_obj.protocol_mode_rd_sec = SEC_JUST_WORKS;
    hids_init_obj.protocol_mode_wr_sec = SEC_JUST_WORKS;
    hids_init_obj.ctrl_point_wr_sec    = SEC_JUST_WORKS;

    err_code = ble_hids_init(&m_hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);

    timers_init();
}

void hid_send_barcode(uint8_t * p_barcode, uint8_t barcode_len) {
    NRF_LOG_INFO("hid_send_barcode()");
    
    m_tx_key_offset = 0;
    m_tx_key_event_count = barcode_len * 2; // after every key we must send a NULL keycode to signal key release
    uint8_t n=0;

    if (m_tx_key_events != NULL)
    {
        NRF_LOG_INFO("Free allocated memory for keycode array.");
        free(m_tx_key_events);
    }

    m_tx_key_events = (uint8_t **) malloc(sizeof(uint8_t*) * m_tx_key_event_count);

    if (m_tx_key_events == NULL)
    {
        NRF_LOG_INFO("Could not allocate memory for keycode array.");
        return;
    }

    uint8_t* NULL_KEY = get_hid_report_for_char(0);
    uint8_t* keydata;

    for (uint8_t i=0; i<barcode_len; i++)
    {        
        keydata = get_hid_report_for_char(p_barcode[i]);
        //NRF_LOG_HEXDUMP_INFO(keydata, INPUT_REPORT_KEYS_MAX_LEN);
        m_tx_key_events[n++] = keydata;
        m_tx_key_events[n++] = NULL_KEY;
    }

    NRF_LOG_INFO("Keycodes:");

    uint8_t* keycode;

    for (uint8_t x=0; x<m_tx_key_event_count; x++)
    {
        keycode = m_tx_key_events[x];
        
        NRF_LOG_HEXDUMP_INFO(keycode, INPUT_REPORT_KEYS_MAX_LEN);
    }

    //hid_tx_timer_start();
 }
