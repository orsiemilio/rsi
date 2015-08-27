#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"

#include <stdio.h> /* For printf() */

static process_event_t event_data_to_print_ready;

/*------------------- THREADS -----------------------------------------------*/
PROCESS(int_print_process, "Print process");
PROCESS(t1_led_toggle_process, "Led toggle process");
PROCESS(t1_cant_btn_process, "Button press count process");

/*------------------- BUTTON THREAD -----------------------------------------*/
AUTOSTART_PROCESSES(&t1_cant_btn_process);
PROCESS_THREAD(t1_cant_btn_process, ev, data)
{
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(button_sensor);

	// Start threads
	process_start(&int_print_process, (void*)NULL);
	
	static int btn_press_count = 0;
	while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
		btn_press_count++;
    //printf("%i\n", btn_press_count);
		process_post(&int_print_process, event_data_to_print_ready, &btn_press_count);		
	}

	PROCESS_END();
}

/*------------------- LED THREAD -----------------------------------------*/
static struct etimer timer;
PROCESS_THREAD(t1_led_toggle_process, ev, data)
{
  PROCESS_BEGIN();

	// event each second
	etimer_set(&timer, CLOCK_CONF_SECOND);
	leds_on(LEDS_RED);
	leds_off(LEDS_GREEN);
	while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		leds_toggle(LEDS_RED);
		leds_toggle(LEDS_GREEN);
		// reset timer to wait event again
		etimer_reset(&timer);
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/***** concurrent process to do printing (int) ****/
PROCESS_THREAD(int_print_process, ev, data)
{
    PROCESS_BEGIN();

    while (1)
    {
        // wait until we get a data_ready event
        PROCESS_WAIT_EVENT_UNTIL(ev == event_data_to_print_ready);
        // display int
        printf("%i\n", *(int *)data);

    }
    PROCESS_END();
}
