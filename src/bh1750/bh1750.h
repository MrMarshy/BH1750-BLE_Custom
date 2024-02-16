#ifndef __BH1750__H
#define __BH1750__H

#include <zephyr/drivers/i2c.h>

#define BH1750_CHECK(x, y) do { bh1750_err_t ret = (x); if(ret != BH1750_OK){(y) = ret;} } while (0)

#define BH1750_ADDR_LO  0x23    /* I2C address when ADDR pin floating/low */
#define BH1750_ADDR_HI  0x5C    /* I2C address when ADDR pin is high */

/* Error codes */
typedef enum{
    BH1750_ERR = -1,
    BH1750_OK = 0,
}bh1750_err_t;  

/* Measurement Mode */
typedef enum{
    BH1750_MODE_ONE_TIME = 0,   /* One time measurement */
    BH1750_MODE_CONTINUOUS      /* Continous measurement */
} bh1750_mode_t;

/* Measurement Resolution */
typedef enum{
    BH1750_RES_LOW = 0,         /* 4 lx resolution, measurement time ~16ms */
    BH1750_RES_HIGH,            /* 1 lx resolution, measurement time ~120ms */
    BH1750_RES_HIGH2,           /* 0.5 lx resolution, measurement time ~120ms */
} bh1750_resolution_t;

typedef enum{
    MODE = BH1750_MODE_ONE_TIME,
}bh1750_mode_state_t;

bh1750_err_t bh1750_setup(const struct device *i2c_dev, bh1750_mode_t mode, bh1750_resolution_t resolution, int16_t i2c_addr);
uint16_t bh1750_read(const struct device *i2c_dev);
bh1750_err_t bh1750_set_measurement_time(const struct device *i2c_dev, uint8_t time);
bh1750_err_t bh1750_power_down(const struct device *i2c_dev);
bh1750_err_t bh1750_power_on(const struct device *i2c_dev);


#endif // __BH1750__H
