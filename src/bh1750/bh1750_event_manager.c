#include "bh1750_event_manager.h"

/* Define Semaphore to protect access to the lux value */
K_SEM_DEFINE(bh1750_lux_sem, 0, 1);

/* Define message queue */
K_MSGQ_DEFINE(bh1750_event_msq, sizeof(struct bh1750_event), BH1750_EVENT_QUEUE_SIZE, 4);

int bh1750_event_manager_push(struct bh1750_event *p_evt){
	
	return k_msgq_put(&bh1750_event_msq, p_evt, K_NO_WAIT);
}

int bh1750_event_manager_get(struct bh1750_event *p_evt){
	return k_msgq_get(&bh1750_event_msq, p_evt, K_FOREVER);
}

const char *bh1750_event_type_to_string(enum bh1750_event_type type){
	const char* bh1750_event_type_str[] = {
		"BH1750_EVENT_INIT",
		"BH1750_EVENT_READ_ONE_TIME",
		"BH1750_EVENT_READ_CONTINUOS",
		"BH1750_EVENT_POWER_ON",
		"BH1750_EVENT_POWER_DOWN",
		"BH1750_EVENT_SET_MEASUREMENT_TIME",
        "BH1750_EVENT_ERROR_INIT",
		"BH1750_EVENT_UNKNOWN"
	};

	return bh1750_event_type_str[type];

}