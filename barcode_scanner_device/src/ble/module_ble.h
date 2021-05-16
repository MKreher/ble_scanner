 /** @file BLE handling module
 *
 * @defgroup m_ble BLE
 * @{
 * @ingroup modules
 * @brief BLE handling module API.
 *
 */

#ifndef __MODULE_BLE_H__
#define __MODULE_BLE_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"

extern bool g_ble_tx_busy;

void start_ble_services(bool erase_bond);
void hid_send_barcode(uint8_t * p_barcode, uint8_t barcode_len);
bool hid_is_connected(void);
#endif

/** @} */
