#include "ble_custom.h"


LOG_MODULE_REGISTER(ble_custom);

extern struct k_sem bh1750_lux_sem;
static struct bh1750_event *_bh1750_evts;
static struct bt_conn *current_conn;
static struct k_work_delayable ble_work;
static volatile bool ble_ready = false;
static ssize_t read_lux_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
	uint16_t len, uint16_t offset);
static void lux_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
static int bt_le_lux_publish(struct bt_conn *conn, uint16_t lux_value);


static enum {
	BLE_DISCONNECTED,
	BLE_ADV_START,
	BLE_ADVERTISING,
	BLE_CONNECTED,
} ble_state;
    
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_LUX_SERV_VAL),
};

/* Make sure this is updated to match BT_GATT_SERVICE_DEFINE below */
enum bh1750_severice_characterisation_position{
    LUX_ATTR_POS = 2,
};

BT_GATT_SERVICE_DEFINE(lux_srv,			// POS 0
    BT_GATT_PRIMARY_SERVICE(BT_UUID_CUSTOM_LUX_SERV),  // POS 1
    BT_GATT_CHARACTERISTIC(BT_UUID_LUX_CHRC, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ, read_lux_cb, NULL, NULL), // POS 2
	BT_GATT_CCC(lux_ccc_cfg_changed, // POS 3
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), // POS 4
);


static int init_bt_le_adv(void);
static void bt_ready(int err);
static uint32_t adv_timeout(void);
static void ble_worker_fn(struct k_work *work);
static void on_connected(struct bt_conn *conn, uint8_t err);
static void on_disconnected(struct bt_conn *conn, uint8_t reason);


static struct bt_conn_cb conn_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};

int init_bt_le_custom(struct bh1750_event* bh1750_evts){
	LOG_INF("init ble custom");
	int ret = 0;

	bt_conn_cb_register(&conn_callbacks);

	ret = bt_enable(bt_ready);
	if(ret){
		LOG_ERR("bt_enable failed (err %d)\n", ret);
		ble_state = BLE_DISCONNECTED;
	}
    else{
        LOG_INF("bt enable success!");
		ble_state = BLE_ADV_START;
    }

	_bh1750_evts = bh1750_evts;

	k_work_init_delayable(&ble_work, ble_worker_fn);
	k_work_reschedule(&ble_work, K_MSEC(10));

	return ret;
}

static int init_bt_le_adv(void){
   
    LOG_INF("init bt le adv");
    int ret; 
    ret = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if(ret){
        LOG_ERR("bt_le_adv_start failed (err %d)\n", ret);
    }
    else{
        LOG_INF("bt le adv start success!");
    }
   
    return ret;
}

static void bt_ready(int err){
	if(err){
		LOG_ERR("bt enble return %d\n", err);
	}
	else{
		LOG_INF("bt ready!\n");
	}
	ble_ready = true;
}

static void on_connected(struct bt_conn *conn, uint8_t err){
	int rc;

	if(err){
		LOG_ERR("Connection failled (err 0x%02x)", err);
	}
	if(ble_state == BLE_ADVERTISING){
		rc = bt_le_adv_stop();
		if(rc < 0){
			LOG_ERR("BLE Advertising stop failed (err 0x%02x)", rc);
		}
	}
	if(!current_conn){
		current_conn = bt_conn_ref(conn);
	}

	ble_state = BLE_CONNECTED;

	k_work_reschedule(&ble_work, K_NO_WAIT);
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason){
    LOG_INF("BLE Disconnected (reason 0x%02x)\n", reason);
	if(current_conn){
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
	if(ble_state == BLE_CONNECTED){
		ble_state = BLE_DISCONNECTED;
	}

	k_work_reschedule(&ble_work, K_NO_WAIT);
}

static uint32_t adv_timeout(void){
	uint32_t timeout;

	if (bt_rand(&timeout, sizeof(timeout)) < 0) {
		return 10 * MSEC_PER_SEC;
	}

	timeout %= (10 * MSEC_PER_SEC);

	return timeout + (1 * MSEC_PER_SEC);
}


static void ble_worker_fn(struct k_work *work){
	switch(ble_state){

		case BLE_CONNECTED:
			LOG_INF("BLE_CONNECTED EVENT");
			break;
		case BLE_ADV_START:
			LOG_INF("BLE_ADV_START EVENT");
			init_bt_le_adv();
			ble_state = BLE_ADVERTISING;
			k_work_reschedule(&ble_work, K_MSEC(adv_timeout()));
			break;
		case BLE_ADVERTISING:
			LOG_INF("BLE_ADVERTISING EVENT");
			break;
		case BLE_DISCONNECTED:
			LOG_INF("BLE_DISCONNECTED EVENT");
			ble_state = BLE_ADV_START;
			k_work_reschedule(&ble_work, K_NO_WAIT);
		default:
			;
	}
}

static ssize_t read_lux_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
	uint16_t len, uint16_t offset){
	
    uint16_t lux = 0;

	k_sem_take(&bh1750_lux_sem, K_FOREVER);
	lux = _bh1750_evts->data.lux_value;
	k_sem_give(&bh1750_lux_sem);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &lux, sizeof(lux));
}

static void lux_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value){

	ARG_UNUSED(attr);

	bool notify_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("BH1750 Lux Notifications %s", notify_enabled ? "enabled": "disabled");
}

int bt_le_lux_publish_sensor_data(uint16_t lux_value){
	return bt_le_lux_publish(current_conn, lux_value);
}

static int bt_le_lux_publish(struct bt_conn *conn, uint16_t lux_value){
	struct bt_gatt_notify_params params = {0};
	const struct bt_gatt_attr *attr = NULL;

	attr = &lux_srv.attrs[LUX_ATTR_POS];
	params.attr = attr;
	params.data = (void*)&lux_value;
	params.len = sizeof(lux_value);
	params.func = NULL;

	if(!conn){
		LOG_DBG("Notification sent to all connected peers");
		return bt_gatt_notify_cb(NULL, &params);
	}
	else if(bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY)){
		return bt_gatt_notify_cb(conn, &params);
	}
	else{
		return -EINVAL;
	}

	// int rc;
	// rc = bt_gatt_notify_uuid(current_bt_conn, BT_UUID_TEMPERATURE, NULL, &temp, sizeof(temp));
	// rc = bt_gatt_notify_uuid(current_bt_conn, BT_UUID_PRESSURE, NULL, &press, sizeof(press));
	// rc = bt_gatt_notify_uuid(current_bt_conn, BT_UUID_HUMIDITY, NULL, &humid, sizeof(humid));
	// return rc == -ENOTCONN ? 0 : rc;
}
