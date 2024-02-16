#include "bh1750.h"

#define OPCODE_HIGH         0x0
#define OPCODE_HIGH2        0x1
#define OPCODE_LOW          0x3

#define OPCODE_CONT         0x10
#define OPCODE_OT           0x20

#define OPCODE_POWER_DOWN   0x00
#define OPCODE_POWER_ON     0x01
#define OPCODE_MT_HI        0x40
#define OPCODE_MT_LO        0x60

struct bh1750{
    int16_t i2c_addr;
    int mode_state;
};

static struct bh1750 bh1750;

bh1750_err_t bh1750_setup(const struct device *i2c_dev, bh1750_mode_t mode, bh1750_resolution_t resolution, int16_t i2c_addr){
    
    bh1750.i2c_addr = i2c_addr;
    if(mode == BH1750_MODE_CONTINUOUS){
        bh1750.mode_state = BH1750_MODE_CONTINUOUS; 
    }
    else{
        bh1750.mode_state = BH1750_MODE_ONE_TIME;
    }
    // printk("bh1750_setup - mode: %d\n", bh1750.mode_state);

    uint8_t opcode = mode = BH1750_MODE_CONTINUOUS ? OPCODE_CONT : OPCODE_OT;

    switch(resolution){
        case BH1750_RES_LOW: 
            opcode |= OPCODE_LOW; 
            break;
        case BH1750_RES_HIGH:
            opcode |= OPCODE_HIGH;
            break;
        default:
            opcode |= OPCODE_HIGH2;
            break;
    }    

    bh1750_err_t ret = BH1750_OK;
    // printk("bh1750_setup(ADDR = 0x%02x, opcode = 0x%02x)\n", bh1750.i2c_addr, opcode);
    ret = i2c_write(i2c_dev, &opcode, 1, bh1750.i2c_addr);

    return ret;
    
}

uint16_t bh1750_read(const struct device *i2c_dev){
    uint8_t buf[2];
    uint16_t lux_level;
    bh1750_err_t ret = BH1750_OK;

    if(bh1750.mode_state == BH1750_MODE_ONE_TIME){
        /* Sensor goes into power down mode after a read. 
          When it is powered up it goes back to default mode and will need to be
          reconfigured back into one time mode.
        */
        ret = bh1750_power_on(i2c_dev);
        if(ret == BH1750_OK){
            ret = bh1750_setup(i2c_dev, BH1750_MODE_ONE_TIME, BH1750_RES_HIGH, bh1750.i2c_addr);
            if(ret == BH1750_OK){
                ret = i2c_read(i2c_dev, buf, 2, bh1750.i2c_addr);
                k_msleep(250);
            }
        }
    }
    else{
        ret = i2c_read(i2c_dev, buf, 2, bh1750.i2c_addr);
    }

    lux_level = buf[0] << 8 | buf[1];
    lux_level = (lux_level * 10) / 12; // convert to LUX

    return lux_level;
}

bh1750_err_t bh1750_set_measurement_time(const struct device *i2c_dev, uint8_t time){
    bh1750_err_t ret = BH1750_OK;
    uint8_t opcode = OPCODE_MT_HI | (time >> 5);
    // printk("bh1750_set_measurement_time(ADDR = 0x%02x, opcode = 0x%02x)\n", bh1750.i2c_addr, opcode);
    ret = i2c_write(i2c_dev, &opcode, 1, bh1750.i2c_addr);

    if(ret == BH1750_OK){
        opcode = OPCODE_MT_LO | (time & 0x1F);
        // printk("bh1750_set_measurement_time(ADDR = 0x%02x, opcode = 0x%02x)\n", bh1750.i2c_addr, opcode);
        ret = i2c_write(i2c_dev, &opcode, 1, bh1750.i2c_addr);
    }

    return ret;   
}

bh1750_err_t bh1750_power_on(const struct device *i2c_dev){
    bh1750_err_t ret = BH1750_OK;
    uint8_t opcode = OPCODE_POWER_ON;
    // printk("bh1750_power_on(ADDR = 0x%02x, opcode = 0x%02x)\n", bh1750.i2c_addr, opcode);
    ret = i2c_write(i2c_dev, &opcode, 1, bh1750.i2c_addr);

    return ret;
}

bh1750_err_t bh1750_power_down(const struct device *i2c_dev){
    bh1750_err_t ret = BH1750_OK;
    uint8_t opcode = OPCODE_POWER_DOWN;
    // printk("bh1750_power_down(ADDR = 0x%02x, opcode = 0x%02x)\n", bh1750.i2c_addr, opcode);
    ret = i2c_write(i2c_dev, &opcode, 1, bh1750.i2c_addr);

    return ret;
}