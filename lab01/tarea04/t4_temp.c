#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/sht11-sensor.h"

#include <stdio.h> /* For printf() */

#define SAMPLING_COUNT 		10
#define TEMP_POLL_DELAY_S 	5


typedef struct temp_s{
	int16_t temp;
	char unit[2];
}temp_t;


///////////////////////////////////////////////////////////////////////////////
//////////////////////// AUX FUNCTIONS ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int getTemp(){
	// activate sensor
	SENSORS_ACTIVATE(sht11_sensor); 
	// sht11_sensor.value(SHT11_SENSOR_TEMP); Return somethin proporcional to temp, not temp itself
	int val = ((0.01*sht11_sensor.value(SHT11_SENSOR_TEMP))-39.6); // T to oC (celcius)
	// deactivate sensor to save energy
	SENSORS_DEACTIVATE(sht11_sensor); 
	return val;
}
///////////////////////////////////////////////////////////////////////////////

// event to print
static process_event_t event_data_to_print_ready;
/*------------------- THREADS -----------------------------------------------*/
PROCESS(print_process, "Print process");
PROCESS(temp_sensing_process, "Temperature measuring");

/*------------------- BUTTON THREAD -----------------------------------------*/
static temp_t print_data;
AUTOSTART_PROCESSES(&temp_sensing_process);
PROCESS_THREAD(temp_sensing_process, ev, data)
{
	static int sampled_count = 0;
	static int temp_sum = 0;
  	PROCESS_BEGIN();

    	printf("Tarea 04 - Temperature measuring\n");
	// Start threads
	process_start(&print_process, (void*)NULL);
	// define etimer
	static struct etimer timer;
	// event each second
	etimer_set(&timer, TEMP_POLL_DELAY_S * CLOCK_SECOND );
	
	print_data.unit[0] = 'o';
	print_data.unit[1] = 'C';

	process_post(&print_process, event_data_to_print_ready, &print_data);		
	while(1){
		temp_sum = 0;
		temp_sum += getTemp();
		// firs sample is outside
		sampled_count = 1;
		while(sampled_count++ < SAMPLING_COUNT){
			// sleep to avoid burining all out
			PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
			temp_sum += getTemp();
			// reset timer to wait event again
			etimer_reset(&timer);
		}
		int t_avg = temp_sum / 10;
		print_data.temp = (int16_t) t_avg;
		process_post(&print_process, event_data_to_print_ready, &print_data);		

		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		// reset timer to wait event again
		etimer_reset(&timer);
	}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/

/***** concurrent process to do printing (int) ****/
PROCESS_THREAD(print_process, ev, in_data)
{
    PROCESS_BEGIN();

    while (1)
    {
        // wait until we get a data_ready event
        PROCESS_WAIT_EVENT_UNTIL(ev == event_data_to_print_ready);
        // display int
	temp_t data = *(temp_t*)in_data;
        printf("Temp: %d %c%c\n", data.temp, data.unit[0], data.unit[1]);
    }
    PROCESS_END();
}
