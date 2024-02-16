#ifndef __BLE_CUSTOM_H
#define __BLE_CUSTOM_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/logging/log.h>

#include "bh1750/bh1750_event_manager.h"

/* UUID of the custom lux service */
#define BT_UUID_CUSTOM_LUX_SERV_VAL BT_UUID_128_ENCODE(0xe552241e,0x773c,4678,0x987d,0x8417d304edb5)
#define BT_UUID_CUSTOM_LUX_SERV BT_UUID_DECLARE_128(BT_UUID_CUSTOM_LUX_SERV_VAL)

/* UUID of the custom lux characteristic */
#define BT_UUID_LUX_CHRC_VAL BT_UUID_128_ENCODE(0xe552241e,0x773c,4678,0x987d,0x8417d304edb6)
#define BT_UUID_LUX_CHRC BT_UUID_DECLARE_128(BT_UUID_LUX_CHRC_VAL)


int init_bt_le_custom(struct bh1750_event *bh1750_evts);

int bt_le_lux_publish_sensor_data(uint16_t lux_value);

#endif // __BLE_CUSTOM_H