#ifndef BLE_SERVICE_H__
#define BLE_SERVICE_H__

#include <stdbool.h>
#include <stdint.h>

/**@brief Function for initializing the BLE HID stack. */
void init_ble_hid();

/**@brief Function for sending barcode via BLE HID service.
 *
 * @param[in]   barcode        The barcode value.
 */
void ble_hid_send_barcode(char *barcode);

#endif // BLE_SERVICE_H__