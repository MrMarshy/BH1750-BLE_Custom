#ifndef _BH1750_EVENT_MANAGER_H
#define _BH1750_EVENT_MANAGER_H

#include <zephyr/kernel.h>

#define BH1750_EVENT_QUEUE_SIZE	24

struct bh1750_event;

int bh1750_event_manager_push(struct bh1750_event *p_evt);

int bh1750_event_manager_get(struct bh1750_event *p_evt);

/**
 * @brief Used to interact with different functionality
 * in this application
 */
enum bh1750_event_type{
    BH1750_EVENT_INIT,
    BH1750_EVENT_READ_ONE_TIME,
    BH1750_EVENT_READ_CONTINUOS,
    BH1750_EVENT_POWER_ON,
    BH1750_EVENT_POWER_DOWN,
    BH1750_EVENT_SET_MEASUREMENT_TIME,
    BH1750_EVENT_ERROR_INIT,
    BH1750_EVENT_UNKNOWN,
    BH1750_EVENT_MAX = BH1750_EVENT_UNKNOWN
};

/**
 * @brief Simplified macro for pushing an app event without data
 * 
 */
#define BH1750_EVENT_MANAGER_PUSH(x) 	\
	struct bh1750_event app_event = { 	\
		.type = x, 					\
	}; 								\
	bh1750_event_manager_push(&app_event);


struct bh1750_data{
    int16_t lux_value;
};

/**
 * @brief Application event that can be passed back to the main
 * context
 * 
 */
struct bh1750_event{													
	enum bh1750_event_type type;								
	union{															
		struct bh1750_data data; 									
		int err;													
	};																
};
#endif // _BH1750_EVENT_MANAGER_H