/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-debug.h"

// access to node_id
#include "sys/node-id.h"
// button
#include "dev/button-sensor.h"

#include "simple-udp.h"
#include "servreg-hack.h"

#include <stdio.h>
#include <string.h>

#include "sensor.h"
#include "dev/sht11-sensor.h"

#define UDP_PORT 1234
#define SERVICE_ID 190

#define SEND_INTERVAL		(60 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

// one second
#define TIMER_INTERVAL		(CLOCK_CONF_SECOND)

static struct simple_udp_connection unicast_connection;

/*---------------------------------------------------------------------------*/
//PROCESS(unicast_sender_process, "Unicast sender example process");
//AUTOSTART_PROCESSES(&unicast_sender_process);
/*---------------------------------------------------------------------------*/
	static void
receiver(struct simple_udp_connection *c,
		const uip_ipaddr_t *sender_addr,
		uint16_t sender_port,
		const uip_ipaddr_t *receiver_addr,
		uint16_t receiver_port,
		const uint8_t *data,
		uint16_t datalen)
{
	printf("Data received on port %d from port %d with length %d\n",
			receiver_port, sender_port, datalen);
}
/*---------------------------------------------------------------------------*/
	static void
set_global_address(void)
{
	uip_ipaddr_t ipaddr;
	int i;
	uint8_t state;

	uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
	uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

	printf("IPv6 addresses: ");
	for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
		state = uip_ds6_if.addr_list[i].state;
		if(uip_ds6_if.addr_list[i].isused &&
				(state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
			uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
			printf("\n");
		}
	}
}
/*---------------------------------------------------------------------------*/

static sensor_t leer_temp(){
	uint16_t val;
	sensor_t temp;

	// activo el sensor
	SENSORS_ACTIVATE(sht11_sensor); 
	// sht11_sensor.value(SHT11_SENSOR_TEMP); devuelve algo proporcional a la temperatura, pero no la temperatura
	// hay que masticarla
	val = ((0.01*sht11_sensor.value(SHT11_SENSOR_TEMP))-39.6); // T to oC (grados celcius)
	// desactivo el sensor para ahorrar energia
	SENSORS_DEACTIVATE(sht11_sensor); 
	temp.valor = val;
	temp.unidades[0] = 'o';
	temp.unidades[1] = 'C';
	temp.time_stamp = clock_seconds();
	return temp;
}

static struct ctimer timer;
void timer_cb(){
	uip_ipaddr_t addr;
	uip_ip6addr(&addr, 0xaaaa,0,0,0,0x212, 0x7401,0x01,0x101);

	static unsigned int message_number = 1;

	char buf[sizeof(pkg_t)];

	pkg_t pkg_to_send;

	pkg_to_send.temp = leer_temp();

	pkg_to_send.sender_node_id = (unsigned char) node_id;
	pkg_to_send.mssg_number = (unsigned char) message_number;

	// copy message to buffer
	memcpy(buf, &pkg_to_send, sizeof(pkg_t) );

	printf("Sending unicast to ");
	uip_debug_ipaddr_print(&addr);
	printf("\n");
	//sprintf(buf, "Message %d", message_number);
	simple_udp_sendto(&unicast_connection, buf, sizeof(buf), &addr);
	message_number++;
	ctimer_restart(&timer);
}
/*------------------- BUTTON THREAD -----------------------------------------*/
PROCESS(t1_cant_btn_process, "Button press count process");
AUTOSTART_PROCESSES(&t1_cant_btn_process);
PROCESS_THREAD(t1_cant_btn_process, ev, data)
{
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(button_sensor);

	set_global_address();

	simple_udp_register(&unicast_connection, UDP_PORT,
			NULL, UDP_PORT, receiver);
        printf("Tarea 01 - Button press counter\n");
        printf("Sending frecuency: 1000 ms\n");
	ctimer_set(&timer, TIMER_INTERVAL , (void*)timer_cb, NULL);

	static int btn_press_count = 0;
	while(1){
    		PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
		btn_press_count++;
		unsigned long frec = TIMER_INTERVAL / (btn_press_count * 4 );
		ctimer_set(&timer, frec, (void*)timer_cb, NULL);
		printf("Button press count: %i. Temperature measuring freciency: %i ms \n", btn_press_count, frec);
	}

	PROCESS_END();
}
