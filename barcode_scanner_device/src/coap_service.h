#ifndef BLE_SCANNER_H__
#define BLE_SCANNER_H__

#include <stdbool.h>
#include <stdint.h>

/**@brief Function for initializing the CoAP stack. */
void init_coap();

/**@brief Function for sending barcode via CoAP.
 *
 * @param[in]   barcode        The barcode value.
 */
static void coap_send_barcode(char *barcode);

#endif // COAP_SERVICE_H__