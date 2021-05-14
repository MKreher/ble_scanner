#ifndef BLE_HID_SERVICE_H__
#define BLE_HID_SERVICE_H__

#include "ble.h"
#include "ble_srv_common.h"
#include "app_util_platform.h"
#include <stdint.h>
#include <stdbool.h>

void hids_init(uint16_t a_conn_handle);
void hid_dequeue_keys(bool tx_flag);
void hid_send_barcode(uint8_t * p_barcode, uint8_t barcode_len);

#endif // BLE_HID_BARCODE_H__