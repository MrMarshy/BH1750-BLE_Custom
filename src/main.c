#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2c.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>

#include "ble_custom.h"
#include "bh1750/bh1750.h"
#include "bh1750/bh1750_event_manager.h"

LOG_MODULE_REGISTER(ble_bh1750);

#define SLEEP_TIME_MS   200

#define I2C_NODE	DT_NODELABEL(i2c0)
static const struct device *i2c0_dev = DEVICE_DT_GET(I2C_NODE);

static struct bh1750_event bh1750_evts;
static bool bh1750_is_setup = false;
extern struct k_sem bh1750_lux_sem;

void bh1750_expiry_function(struct k_timer *tmr){
	(void)tmr;
	BH1750_EVENT_MANAGER_PUSH(bh1750_evts.type);
}

/* Timers */
K_TIMER_DEFINE(bh1750_read_timer, bh1750_expiry_function, NULL);

int main(void){
	bh1750_err_t ret; 

	if(!device_is_ready(i2c0_dev)){
		printk("i2c0_dev not ready\n");
		return -1;
	}

	if((ret = bh1750_setup(i2c0_dev, BH1750_MODE_CONTINUOUS, 
		BH1750_RES_HIGH, BH1750_ADDR_LO)) != BH1750_OK){
		printk("bh1750 - Unable to setup device: %d\n", ret);
		return BH1750_ERR; 
	}
	k_sem_give(&bh1750_lux_sem);
	BH1750_EVENT_MANAGER_PUSH(BH1750_EVENT_INIT);
	k_timer_start(&bh1750_read_timer, K_SECONDS(2), K_SECONDS(2));

	while (1) {
		bh1750_event_manager_get(&bh1750_evts);

		switch(bh1750_evts.type){
			case BH1750_EVENT_INIT:
				if(!bh1750_is_setup){
					if((ret = bh1750_setup(i2c0_dev, BH1750_MODE_ONE_TIME, 
							BH1750_RES_HIGH, BH1750_ADDR_LO)) != BH1750_OK){

						LOG_ERR("bh1750 - Unable to setup device: %d\n", ret);
						bh1750_evts.type = BH1750_EVENT_ERROR_INIT;
					}
					else{
						bh1750_is_setup = true;
						bh1750_evts.type = BH1750_EVENT_READ_ONE_TIME;

						/* Start BLE */
						init_bt_le_custom(&bh1750_evts);
					}
				}
				break;
			
			case BH1750_EVENT_READ_ONE_TIME:
				k_sem_take(&bh1750_lux_sem, K_NO_WAIT);
				bh1750_evts.data.lux_value = bh1750_read(i2c0_dev);
				/* send notification */
				bt_le_lux_publish_sensor_data(bh1750_evts.data.lux_value);
				k_sem_give(&bh1750_lux_sem);
				LOG_INF("Lux: %d\n", bh1750_evts.data.lux_value);
				break;

			case BH1750_EVENT_READ_CONTINUOS:
				k_sem_take(&bh1750_lux_sem, K_NO_WAIT);
				bh1750_evts.data.lux_value = bh1750_read(i2c0_dev);
				/* send notification */
				bt_le_lux_publish_sensor_data(bh1750_evts.data.lux_value);
				k_sem_give(&bh1750_lux_sem);
				LOG_INF("Lux: %d\n", bh1750_evts.data.lux_value);
				break;

			case BH1750_EVENT_POWER_DOWN:
				if((ret = bh1750_power_down(i2c0_dev)) != BH1750_OK){
					LOG_ERR("Could not power down BH1750 sensor: %d\n", ret);
				}
				bh1750_evts.type = BH1750_EVENT_UNKNOWN;
				break;

			case BH1750_EVENT_ERROR_INIT:
				LOG_INF("BH1750_EVENT_ERROR_INIT");
				LOG_INF("Attempting to initialise sensor again in two seconds");
				bh1750_evts.type = BH1750_EVENT_INIT;
				break;
			default:
				;
		}
	}

	return 0;
}
