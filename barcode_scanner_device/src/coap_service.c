/** 
 * Implementierung der CoAP Kommunikation.
 */

#include <stdbool.h>
#include <stdint.h>
#include "boards.h"
#include "nordic_common.h"
#include "sdk_config.h"
#include "nrf_sdm.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "app_button.h"
#include "lwip/init.h"
#include "lwip/inet6.h"
#include "lwip/ip6.h"
#include "lwip/ip6_addr.h"
#include "lwip/netif.h"
#include "lwip/timers.h"
#include "mqtt.h"
#include "nrf_platform_port.h"
#include "app_util_platform.h"
#include "iot_timer.h"
#include "coap_api.h"
#include "ipv6_medium.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/** Modify SERVER_IPV6_ADDRESS according to your setup.
 *  The address provided below is a place holder.  */
//  2003:e8:2712:0:1a13:bea1:da:901f
#define SERVER_IPV6_ADDRESS             0x20, 0x03, 0x00, 0xE8, 0x27, 0x12, 0x00, 0x00, \
                                        0x1A, 0x13, 0xBE, 0xA1, 0x00, 0xDA, 0x90, 0x1F        /**< IPv6 address of the server node. */

static const ipv6_addr_t               m_broker_addr =
{
    .u8 = {SERVER_IPV6_ADDRESS}   
};

// The CoAP default port number 5683 MUST be supported by a server.
#define LOCAL_PORT_NUM                  5683                                                  /**< CoAP default port number. */
#define REMOTE_PORT_NUM                 5683                                                  /**< Remote port number. */

#define COAP_MESSAGE_TYPE               COAP_TYPE_NON                                         /**< Message type for all outgoing CoAP requests. */

#ifdef COMMISSIONING_ENABLED
#define ERASE_BUTTON_PIN_NO             BSP_BUTTON_3                                          /**< Button used to erase commissioning settings. */
#endif // COMMISSIONING_ENABLED

#define LWIP_SYS_TICK_MS                100                                                   /**< Interval for timer used as trigger to send. */
#define COAP_TICK_INTERVAL_MS           1000                                                  /**< Interval between periodic callbacks to CoAP module. */

#define APP_RTR_SOLICITATION_DELAY      500                                                   /**< Time before host sends an initial solicitation in ms. */

#define MAX_LENGTH_FILENAME             128                                                   /**< Max length of filename to copy for the debug error handler. */

#define APP_ENABLE_LOGS                 1
#if (APP_ENABLE_LOGS == 1)
#define APPL_LOG  NRF_LOG_INFO
#define APPL_DUMP NRF_LOG_RAW_HEXDUMP_INFO
#define APPL_ADDR IPV6_ADDRESS_LOG
#else // APP_ENABLE_LOGS
#define APPL_LOG(...)
#define APPL_DUMP(...)
#define APPL_ADDR(...)
#endif // APP_ENABLE_LOGS

#define APP_MQTT_BROKER_PORT                8883                                                    /**< Port number of MQTT Broker being used. */
#define APP_MQTT_BROKER_NON_SECURE_PORT     1883                                                    /**< Port number of MQTT Broker being used. */
#define APP_MQTT_PUBLISH_TOPIC              "blescanner/barcode"                                       /**< MQTT topic to which this application publishes. */

/**@brief Application state with respect to MQTT. */
typedef enum
{
    APP_MQTT_STATE_IDLE,                                                                            /**< Indicates no MQTT connection exists. */
    APP_MQTT_STATE_CONNECTED                                                                        /**< Indicates MQTT connection is established. */
} app_mqtt_state_t;

typedef enum
{
    INACTIVE = 0,
    CONNECTABLE_MODE,
    IPV6_IF_DOWN,
    IPV6_IF_UP,
    CONNECTED_TO_BROKER
} ipv6_state_t;

APP_TIMER_DEF(m_iot_timer_tick_src_id);                                                      /**< System Timer used to service CoAP and LWIP periodically. */
eui64_t                            eui64_local_iid;                                          /**< Local EUI64 value that is used as the IID for SLAAC. */
static ipv6_medium_instance_t      m_ipv6_medium;
static bool                        m_is_interface_up;
static uint8_t                     m_well_known_core[100];
static ipv6_state_t                m_ipv6_state      = INACTIVE;                             /**< IPv6 state. */
static const char                  m_uri_part_blescanner[]  = "blescanner";
static const char                  m_uri_part_barcode[]    = "barcode";
static int                         m_temperature        = 21;
static uint16_t                    m_global_token_count = 0x0102;

static mqtt_client_t                        m_app_mqtt_client;                                      /**< MQTT Client instance reference provided by the MQTT module. */
static const char                           m_client_id[] = "nrfPublisher";                         /**< Unique MQTT client identifier. */
static app_mqtt_state_t                     m_connection_state = APP_MQTT_STATE_IDLE;               /**< MQTT Connection state. */
static bool                                 m_do_ind_err = false;
static uint8_t                              m_ind_err_count = 0;
static uint16_t                             m_message_counter = 1;                                  /**< Message counter used to generated message ids for MQTT messages. */


static const uint8_t identity[] = {0x43, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x69, 0x64, 0x65, 0x6e, 0x74, 0x69, 0x74, 0x79};
static const uint8_t shared_secret[] = {0x73, 0x65, 0x63, 0x72, 0x65, 0x74, 0x50, 0x53, 0x4b};

static nrf_tls_preshared_key_t m_preshared_key = {
    .p_identity     = &identity[0],
    .p_secret_key   = &shared_secret[0],
    .identity_len   = 15,
    .secret_key_len = 9
};

static nrf_tls_key_settings_t m_tls_keys = {
    .p_psk = &m_preshared_key
};


#ifdef COMMISSIONING_ENABLED
static bool                        m_power_off_on_failure = false;
static bool                        m_identity_mode_active;
#endif // COMMISSIONING_ENABLED


/**@brief Forward declarations. */
void app_mqtt_evt_handler(mqtt_client_t * const p_client, const mqtt_evt_t * p_evt);


//#define MQTT_SECURE_CONNECTION_ENABLED

#ifdef MQTT_SECURE_CONNECTION_ENABLED
/**@brief Connect to MQTT broker. */
static void app_mqtt_connect(void)
{
    mqtt_client_init(&m_app_mqtt_client);

    memcpy(m_app_mqtt_client.broker_addr.u8, m_broker_addr.u8, IPV6_ADDR_SIZE);
    m_app_mqtt_client.broker_port          = APP_MQTT_BROKER_PORT;
    m_app_mqtt_client.evt_cb               = app_mqtt_evt_handler;
    m_app_mqtt_client.client_id.p_utf_str  = (uint8_t *)m_client_id;
    m_app_mqtt_client.client_id.utf_strlen = strlen(m_client_id);
    m_app_mqtt_client.p_password           = NULL;
    m_app_mqtt_client.p_user_name          = NULL;
    m_app_mqtt_client.transport_type       = MQTT_TRANSPORT_SECURE;
    m_app_mqtt_client.p_security_settings  = &m_tls_keys;
    UNUSED_VARIABLE(mqtt_connect(&m_app_mqtt_client));
}
#endif // MQTT_SECURE_CONNECTION_ENABLED

#ifndef MQTT_SECURE_CONNECTION_ENABLED
/**@brief Connect to MQTT broker. */
static void app_mqtt_connect(void)
{
    mqtt_client_init(&m_app_mqtt_client);

    memcpy(m_app_mqtt_client.broker_addr.u8, m_broker_addr.u8, IPV6_ADDR_SIZE);
    m_app_mqtt_client.broker_port          = APP_MQTT_BROKER_NON_SECURE_PORT;
    m_app_mqtt_client.evt_cb               = app_mqtt_evt_handler;
    m_app_mqtt_client.client_id.p_utf_str  = (uint8_t *)m_client_id;
    m_app_mqtt_client.client_id.utf_strlen = strlen(m_client_id);
    m_app_mqtt_client.p_password           = NULL;
    m_app_mqtt_client.p_user_name          = NULL;
    m_app_mqtt_client.transport_type       = MQTT_TRANSPORT_NON_SECURE;
    m_app_mqtt_client.p_security_settings  = NULL;
    UNUSED_VARIABLE(mqtt_connect(&m_app_mqtt_client));
}
#endif // MQTT_SECURE_CONNECTION_ENABLED


/**@brief Function for handling the timer used to trigger TCP actions.
 *
 * @details This function is used to trigger TCP connection request and to send data on the TCP port.
 *
 * @param[in]   p_context   Pointer used for passing context. No context used in this application.
 */
static void app_lwip_time_tick(iot_timer_time_in_ms_t wall_clock_value)
{
    UNUSED_VARIABLE(wall_clock_value);

    sys_check_timeouts();

    UNUSED_VARIABLE(mqtt_live());
}

/**@brief Function for catering CoAP module with periodic time ticks.
*/
static void app_coap_time_tick(iot_timer_time_in_ms_t wall_clock_value)
{
    // Pass a tick to CoAP in order to re-transmit any pending messages.
    (void)coap_time_tick();
}


/**@brief Function for updating the wall clock of the IoT Timer module.
 */
static void iot_timer_tick_callback(void * p_context)
{
    UNUSED_VARIABLE(p_context);
    uint32_t err_code = iot_timer_update();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module.
    //APP_ERROR_CHECK(app_timer_init());

    // Create a sys timer.
    err_code = app_timer_create(&m_iot_timer_tick_src_id,
                                APP_TIMER_MODE_REPEATED,
                                iot_timer_tick_callback);
    APP_ERROR_CHECK(err_code);
}


static void coap_response_handler(uint32_t status, void * p_arg, coap_message_t * p_response)
{
    APPL_LOG("Response Code : %d", p_response->header.code);
    if (p_response->header.code == COAP_CODE_204_CHANGED)
    {
        // success
        APPL_LOG("CoAP request success response.");
    }
    else
    {
        // failure
        APPL_LOG("CoAP request error response.");
    }
}


/**@brief Function for sending barcode via CoAP.
 *
 * @param[in]   barcode        The barcode value.
 */
void coap_send_barcode(char *barcode)
{
    APPL_LOG("coap_send_barcode()....");
    uint32_t err_code;

#ifdef COMMISSIONING_ENABLED
    if ((button_action == APP_BUTTON_PUSH) && (pin_no == ERASE_BUTTON_PIN_NO))
    {
        APPL_LOG("Erasing all commissioning settings from persistent storage...");
        commissioning_settings_clear();
        return;
    }
#endif // COMMISSIONING_ENABLED

    if (m_is_interface_up == false)
    {
        APPL_LOG("IPv6 interface not up and running. CoAP request can not be send.");
        return;
    }

    coap_message_t      * p_request;
    coap_message_conf_t   message_conf;
    memset(&message_conf, 0x00, sizeof(message_conf));
    coap_remote_t         remote_device;

    message_conf.type             = COAP_MESSAGE_TYPE;
    message_conf.code             = COAP_CODE_PUT;
    message_conf.port.port_number = LOCAL_PORT_NUM;
    message_conf.id               = 0; // Auto-generate message ID.

    (void)uint16_encode(HTONS(m_global_token_count), message_conf.token);
    m_global_token_count++;

    message_conf.token_len = 2;
    message_conf.response_callback = coap_response_handler;
    

    err_code = coap_message_new(&p_request, &message_conf);

    APP_ERROR_CHECK(err_code);
    memcpy(&remote_device.addr[0], (uint8_t []){SERVER_IPV6_ADDRESS}, IPV6_ADDR_SIZE);
    remote_device.port_number = REMOTE_PORT_NUM;
    err_code = coap_message_remote_addr_set(p_request, &remote_device);
    APP_ERROR_CHECK(err_code);

    err_code = coap_message_opt_str_add(p_request, COAP_OPT_URI_PATH, (uint8_t *)m_uri_part_blescanner, strlen(m_uri_part_blescanner));
    APP_ERROR_CHECK(err_code);

    err_code = coap_message_opt_str_add(p_request, COAP_OPT_URI_PATH, (uint8_t *)m_uri_part_barcode, strlen(m_uri_part_barcode));
    APP_ERROR_CHECK(err_code);

    //uint8_t payload[] = {COMMAND_TOGGLE};
    err_code = coap_message_payload_set(p_request, (void *) barcode, strlen(barcode));
    APP_ERROR_CHECK(err_code);

    uint32_t handle;
    err_code = coap_message_send(&handle, p_request);
    if (err_code != NRF_SUCCESS)
    {
        APPL_LOG("CoAP Message skipped, error code = 0x%08lX.", err_code);
    }
    err_code = coap_message_delete(p_request);
    APP_ERROR_CHECK(err_code);

    APPL_LOG("coap_send_barcode().");
}

void well_known_core_callback(coap_resource_t * p_resource, coap_message_t * p_request)
{
    coap_message_conf_t response_config;
    memset(&response_config, 0x00, sizeof(coap_message_conf_t));

    if (p_request->header.type == COAP_TYPE_NON)
    {
        response_config.type = COAP_TYPE_NON;
    }
    else if (p_request->header.type == COAP_TYPE_CON)
    {
        response_config.type = COAP_TYPE_ACK;
    }

    // PIGGY BACKED RESPONSE
    response_config.code = COAP_CODE_205_CONTENT;
    // Copy message ID.
    response_config.id = p_request->header.id;
    // Set local port number to use.
    response_config.port.port_number = LOCAL_PORT_NUM;
    // Copy token.
    memcpy(&response_config.token[0], &p_request->token[0], p_request->header.token_len);
    // Copy token length.
    response_config.token_len = p_request->header.token_len;

    coap_message_t * p_response;
    uint32_t err_code = coap_message_new(&p_response, &response_config);
    APP_ERROR_CHECK(err_code);

    err_code = coap_message_remote_addr_set(p_response, &p_request->remote);
    APP_ERROR_CHECK(err_code);

    err_code = coap_message_opt_uint_add(p_response, COAP_OPT_CONTENT_FORMAT,
                                         COAP_CT_APP_LINK_FORMAT);
    APP_ERROR_CHECK(err_code);

    err_code = coap_message_payload_set(p_response, m_well_known_core,
                                        strlen((char *)m_well_known_core));
    APP_ERROR_CHECK(err_code);

    uint32_t handle;
    err_code = coap_message_send(&handle, p_response);
    APP_ERROR_CHECK(err_code);

    err_code = coap_message_delete(p_response);
    APP_ERROR_CHECK(err_code);
}


static void thermometer_callback(coap_resource_t * p_resource, coap_message_t * p_request)
{
    coap_message_conf_t response_config;
    memset(&response_config, 0x00, sizeof(coap_message_conf_t));

    if (p_request->header.type == COAP_TYPE_NON)
    {
        response_config.type = COAP_TYPE_NON;
    }
    else if (p_request->header.type == COAP_TYPE_CON)
    {
        response_config.type = COAP_TYPE_ACK;
    }

    // PIGGY BACKED RESPONSE
    response_config.code = COAP_CODE_405_METHOD_NOT_ALLOWED;
    // Copy message ID.
    response_config.id = p_request->header.id;
    // Set local port number to use.
    response_config.port.port_number = LOCAL_PORT_NUM;
    // Copy token.
    memcpy(&response_config.token[0], &p_request->token[0], p_request->header.token_len);
    // Copy token length.
    response_config.token_len = p_request->header.token_len;

    coap_message_t * p_response;
    uint32_t err_code = coap_message_new(&p_response, &response_config);
    APP_ERROR_CHECK(err_code);

    err_code = coap_message_remote_addr_set(p_response, &p_request->remote);
    APP_ERROR_CHECK(err_code);

    switch (p_request->header.code)
    {
        case COAP_CODE_GET:
        {
            p_response->header.code = COAP_CODE_205_CONTENT;

            // Set response payload to actual thermometer value.
            char response_str[5];
            memset(response_str, '\0', sizeof(response_str));
            sprintf(response_str, "%d", m_temperature);
            err_code = coap_message_payload_set(p_response, response_str, strlen(response_str));
            APP_ERROR_CHECK(err_code);

            break;
        }
        case COAP_CODE_PUT:
        {
            if ((p_request->payload_len == 0) || (p_request->payload_len > 4))  // Input value cannot be more than 4 characters (1 sign + 3 digits).
            {
                p_response->header.code = COAP_CODE_400_BAD_REQUEST;
                break;
            }

            uint32_t i;
            for (i = 0; i < p_request->payload_len; ++i)
            {
                if (i == 0)
                {
                    // The first digit of the input value must be the ASCII code of a decimal number or a minus sign or a plus sign.
                    if ((((p_request->p_payload[i] < 0x30) && (p_request->p_payload[i] != 0x2B)) &&
                        ((p_request->p_payload[i] < 0x30) && (p_request->p_payload[i] != 0x2D))) ||
                        (p_request->p_payload[i] > 0x39))
                    {
                        p_response->header.code = COAP_CODE_400_BAD_REQUEST;
                        break;
                    }
                }
                else
                {
                    // The remaining digits of the input value must be ASCII codes of decimal numbers.
                    if ((p_request->p_payload[i] < 0x30) || (p_request->p_payload[i] > 0x39))
                    {
                        p_response->header.code = COAP_CODE_400_BAD_REQUEST;
                        break;
                    }
                }
            }

            char input_str[5];
            memset(input_str, '\0', sizeof(input_str));
            memcpy(input_str, p_request->p_payload, p_request->payload_len);

            if ((atoi(input_str) < -100) || (atoi(input_str) > 100))    // Input value must be in valid range.
            {
                p_response->header.code = COAP_CODE_400_BAD_REQUEST;
                break;
            }

            m_temperature = atoi(input_str);

            p_response->header.code = COAP_CODE_204_CHANGED;
            break;
        }
        default:
        {
            p_response->header.code = COAP_CODE_400_BAD_REQUEST;
            break;
        }
    }

    uint32_t handle;
    err_code = coap_message_send(&handle, p_response);
    APP_ERROR_CHECK(err_code);

    err_code = coap_message_delete(p_response);
    APP_ERROR_CHECK(err_code);
}


static void coap_endpoints_init(void)
{
    uint32_t err_code;

    static coap_resource_t root;
    err_code = coap_resource_create(&root, "/");
    APP_ERROR_CHECK(err_code);

    static coap_resource_t well_known;
    err_code = coap_resource_create(&well_known, ".well-known");
    APP_ERROR_CHECK(err_code);
    err_code = coap_resource_child_add(&root, &well_known);
    APP_ERROR_CHECK(err_code);

    static coap_resource_t core;
    err_code = coap_resource_create(&core, "core");
    APP_ERROR_CHECK(err_code);

    core.permission = COAP_PERM_GET;
    core.callback   = well_known_core_callback;
    err_code = coap_resource_child_add(&well_known, &core);
    APP_ERROR_CHECK(err_code);

    static coap_resource_t sensors;
    err_code = coap_resource_create(&sensors, "sensors");
    APP_ERROR_CHECK(err_code);
    err_code = coap_resource_child_add(&root, &sensors);
    APP_ERROR_CHECK(err_code);

    static coap_resource_t thermometer;
    err_code = coap_resource_create(&thermometer, "thermometer");
    APP_ERROR_CHECK(err_code);

    thermometer.permission = (COAP_PERM_GET | COAP_PERM_PUT);
    thermometer.callback = thermometer_callback;
    err_code = coap_resource_child_add(&sensors, &thermometer);
    APP_ERROR_CHECK(err_code);

    uint16_t size = sizeof(m_well_known_core);
    err_code = coap_resource_well_known_generate(m_well_known_core, &size);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function to handle interface up event. */
void nrf_driver_interface_up(iot_interface_t const * p_interface)
{
    UNUSED_PARAMETER(p_interface);

    m_is_interface_up = true;

#ifdef COMMISSIONING_ENABLED
    commissioning_joining_mode_timer_ctrl(JOINING_MODE_TIMER_STOP_RESET);
#endif // COMMISSIONING_ENABLED

    APPL_LOG ("IPv6 interface up.");

    sys_check_timeouts();
    m_ipv6_state = IPV6_IF_UP;
}


/**@brief Function to handle interface down event. */
void nrf_driver_interface_down(iot_interface_t const * p_interface)
{
    UNUSED_PARAMETER(p_interface);

    m_is_interface_up = false;

#ifdef COMMISSIONING_ENABLED
    commissioning_joining_mode_timer_ctrl(JOINING_MODE_TIMER_START);
#endif // COMMISSIONING_ENABLED

    APPL_LOG ("IPv6 interface down.");

    m_ipv6_state = IPV6_IF_DOWN;
}


/**@brief Function for initializing IP stack.
 *
 * @details Initialize the IP Stack and its driver.
 */
static void ip_stack_init(void)
{
    APPL_LOG("ip_stack_init()....");

    uint32_t err_code;
    err_code = ipv6_medium_eui64_get(m_ipv6_medium.ipv6_medium_instance_id,
                                     &eui64_local_iid);
    APP_ERROR_CHECK(err_code);

    //Initialize memory manager.
    err_code = nrf_mem_init();
    APP_ERROR_CHECK(err_code);

    //Initialize lwip stack driver.
    err_code = nrf_driver_init();
    APP_ERROR_CHECK(err_code);

    //Initialize lwip stack.
    lwip_init();

    APPL_LOG("ip_stack_init().");
}


/**@brief Function for initializing the IoT Timer. */
static void iot_timer_init(void)
{
    uint32_t err_code;

    static const iot_timer_client_t list_of_clients[] =
    {
        {app_lwip_time_tick,      LWIP_SYS_TICK_MS},
        {app_coap_time_tick,      COAP_TICK_INTERVAL_MS},
#ifdef COMMISSIONING_ENABLED
        {commissioning_time_tick, SEC_TO_MILLISEC(COMMISSIONING_TICK_INTERVAL_SEC)}
#endif // COMMISSIONING_ENABLED
    };

    // The list of IoT Timer clients is declared as a constant.
    static const iot_timer_clients_list_t iot_timer_clients =
    {
        (sizeof(list_of_clients) / sizeof(iot_timer_client_t)),
        &(list_of_clients[0]),
    };

    // Passing the list of clients to the IoT Timer module.
    err_code = iot_timer_client_list_set(&iot_timer_clients);
    APP_ERROR_CHECK(err_code);

    // Starting the app timer instance that is the tick source for the IoT Timer.
    err_code = app_timer_start(m_iot_timer_tick_src_id,
                               APP_TIMER_TICKS(IOT_TIMER_RESOLUTION_IN_MS),
                               NULL);
    APP_ERROR_CHECK(err_code);
}

void app_mqtt_evt_handler(mqtt_client_t * const p_client, const mqtt_evt_t * p_evt)
{
    switch (p_evt->id)
    {
        case MQTT_EVT_CONNACK:
        {
            APPL_LOG (">> MQTT_EVT_CONNACK, result %08lx", p_evt->result);
            if (p_evt->result == NRF_SUCCESS)
            {
                m_connection_state = APP_MQTT_STATE_CONNECTED;
                m_ipv6_state = CONNECTED_TO_BROKER;
            }
            else
            {
                m_connection_state = APP_MQTT_STATE_IDLE;
                m_ipv6_state = IPV6_IF_UP;
            }
            break;
        }
        case MQTT_EVT_PUBACK:
        {
            APPL_LOG (">> MQTT_EVT_PUBACK");
            break;
        }
        case MQTT_EVT_DISCONNECT:
        {
            APPL_LOG (">> MQTT_EVT_DISCONNECT");
            m_connection_state = APP_MQTT_STATE_IDLE;
            m_ipv6_state = IPV6_IF_UP;
            break;
        }
        default:
            break;
    }
}


/**@brief Function for sending barcode via MQTT.
 *
 * @param[in]   barcode        The barcode value.
 */
void mqtt_send_barcode(char *barcode)
{
    APPL_LOG("mqtt_send_barcode()....");

#ifdef COMMISSIONING_ENABLED
    if ((button_action == APP_BUTTON_PUSH) && (pin_no == ERASE_BUTTON_PIN_NO))
    {
        APPL_LOG("Erasing all commissioning settings from persistent storage...");
        commissioning_settings_clear();
        return;
    }
#endif // COMMISSIONING_ENABLED

    if (m_connection_state != APP_MQTT_STATE_CONNECTED)
    {
        APPL_LOG("MQTT not connected. Connecting....");

        app_mqtt_connect();

        sys_check_timeouts();

        if (m_connection_state == APP_MQTT_STATE_CONNECTED)
        {
            APPL_LOG("MQTT successfuly connected.");
        } else {
            APPL_LOG("MQTT failed to connect.");
            return;
        }
    }

    // Set topic to be published.
    const char * topic_str = APP_MQTT_PUBLISH_TOPIC;

    mqtt_publish_param_t param;

    param.message.topic.qos              = MQTT_QoS_1_ATLEAST_ONCE;
    param.message.topic.topic.p_utf_str  = (uint8_t *)topic_str;
    param.message.topic.topic.utf_strlen = strlen(topic_str);
    param.message.payload.p_bin_str      = (uint8_t *)barcode,
    param.message.payload.bin_strlen     = strlen(barcode);
    param.message_id                     = m_message_counter;
    param.dup_flag                       = 0;
    param.retain_flag                    = 0;

    uint32_t err_code = mqtt_publish(&m_app_mqtt_client, &param);
    APPL_LOG("mqtt_publish result 0x%08lx", err_code);

    if (err_code == NRF_SUCCESS)
    {
        APPL_LOG("MQTT publish successfuly.");
        // Avoid ever sending invalid message id 0.
        m_message_counter+= 2;
    }
    else
    {
        APPL_LOG("MQTT publish failed. Error code: 0x%08lx.", err_code);
        m_do_ind_err = true;
    }

    APPL_LOG("mqtt_send_barcode().");
}

/**@brief Function for starting connectable mode.
 */
static void connectable_mode_enter(void)
{
    uint32_t err_code = ipv6_medium_connectable_mode_enter(m_ipv6_medium.ipv6_medium_instance_id);
    APP_ERROR_CHECK(err_code);

    APPL_LOG("Physical layer in connectable mode.");
    m_ipv6_state = CONNECTABLE_MODE;
}


static void on_ipv6_medium_evt(ipv6_medium_evt_t * p_ipv6_medium_evt)
{
    switch (p_ipv6_medium_evt->ipv6_medium_evt_id)
    {
        case IPV6_MEDIUM_EVT_CONN_UP:
        {
            APPL_LOG("Physical layer: connected.");
            m_ipv6_state = IPV6_IF_DOWN;
            break;
        }
        case IPV6_MEDIUM_EVT_CONN_DOWN:
        {
            APPL_LOG("Physical layer: disconnected.");
            connectable_mode_enter();
            break;
        }
        default:
        {
            break;
        }
    }
}


static void on_ipv6_medium_error(ipv6_medium_error_t * p_ipv6_medium_error)
{
    APPL_LOG("on_ipv6_medium_error() -> not handled.");
    // Do something.
}


#ifdef COMMISSIONING_ENABLED
void commissioning_id_mode_cb(mode_control_cmd_t control_command)
{
    switch (control_command)
    {
        case CMD_IDENTITY_MODE_ENTER:
        {
            LEDS_OFF(LED_THREE | LED_FOUR);
            m_identity_mode_active = true;

            break;
        }
        case CMD_IDENTITY_MODE_EXIT:
        {
            m_identity_mode_active = false;
            LEDS_OFF((LED_THREE | LED_FOUR));

            break;
        }
        default:
        {

            break;
        }
    }
}


void commissioning_power_off_cb(bool power_off_on_failure)
{
    m_power_off_on_failure = power_off_on_failure;

    APPL_LOG("Commissioning: do power_off on failure: %s.",
             m_power_off_on_failure ? "true" : "false");
}
#endif // COMMISSIONING_ENABLED


/**@brief Function for initialization of the CoAP stack.
 */
int init_coap(void)
{
    uint32_t err_code;

    timers_init();
    iot_timer_init();

    static ipv6_medium_init_params_t ipv6_medium_init_params;
    memset(&ipv6_medium_init_params, 0x00, sizeof(ipv6_medium_init_params));
    ipv6_medium_init_params.ipv6_medium_evt_handler    = on_ipv6_medium_evt;
    ipv6_medium_init_params.ipv6_medium_error_handler  = on_ipv6_medium_error;
#ifdef COMMISSIONING_ENABLED
    ipv6_medium_init_params.commissioning_id_mode_cb   = commissioning_id_mode_cb;
    ipv6_medium_init_params.commissioning_power_off_cb = commissioning_power_off_cb;
#endif // COMMISSIONING_ENABLED

    err_code = ipv6_medium_init(&ipv6_medium_init_params,
                                IPV6_MEDIUM_ID_BLE,
                                &m_ipv6_medium);
    APP_ERROR_CHECK(err_code);

    eui48_t ipv6_medium_eui48;
    err_code = ipv6_medium_eui48_get(m_ipv6_medium.ipv6_medium_instance_id,
                                     &ipv6_medium_eui48);

    ipv6_medium_eui48.identifier[EUI_48_SIZE - 1] = 0x00;

    err_code = ipv6_medium_eui48_set(m_ipv6_medium.ipv6_medium_instance_id,
                                     &ipv6_medium_eui48);
    APP_ERROR_CHECK(err_code);

    ip_stack_init();

    coap_port_t local_port_list[COAP_PORT_COUNT] =
    {
        {.port_number = LOCAL_PORT_NUM}
    };

    coap_transport_init_t port_list;
    port_list.p_port_table = &local_port_list[0];

    err_code = coap_init(17, &port_list);
    APP_ERROR_CHECK(err_code);

    coap_endpoints_init();

    // Start execution
    connectable_mode_enter();

    APPL_LOG("CoAP stack started.");
}

/**
 * @}
 */
