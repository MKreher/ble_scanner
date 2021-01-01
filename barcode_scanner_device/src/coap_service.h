#ifndef BLE_SCANNER_H__
#define BLE_SCANNER_H__

#include <stdbool.h>
#include <stdint.h>

/**@brief Function for initializing the CoAP stack. */
void coap_ipv6_init();

/**@brief Function for sending barcode via CoAP.
 *
 * @param[in]   barcode        The barcode value.
 */
void coap_send_barcode(const char * barcode);

/**@brief Function for sending barcode via MQTT.
 *
 * @param[in]   barcode        The barcode value.
 */
void mqtt_send_barcode(const char * barcode);

#endif // COAP_SERVICE_H__