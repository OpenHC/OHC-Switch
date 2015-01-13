/*
 * OHC_Core.c
 *
 * Created: 17.11.2014 18:53:03
 *  Author: Tobias
 */ 


#include <avr/io.h>

#define F_CPU 8000000UL

#include <stdio.h>
#include "lib/UART/uart.h"
#include "lib/NRF24L01/NRF24L01.h"
#include "lib/ohc_core/core.h"
#include <avr/sleep.h>

#define RX TRUE

void callback(uint16_t id)
{
	uart_write_async("CALLBACK_RX:");
	uint8_t* data = malloc(5);
	core_read_field(id, data, 0 ,5);
	uart_send_async(data, 0 ,5);
	uart_write_async("\n");
	free(data);
	if(id == 0)
	{
		uint8_t state = 0;
		core_read_field(id, &state, 0 ,1);
		if(state)
		{
			PORTC |= (1<<PINC0);
		}
		else
		{
			PORTC &= ~(1<<PINC0);
		}
	}
}

void dump_config(uint8_t regaddr, uint8_t len)
{
	uint8_t* buff = malloc(len);
	uint8_t* buffptr = buff;
	NRF24L01_LOW_read_register(regaddr, buff, len);
	while(len-- > 0)
		uart_send_byte(*buffptr++);
	free(buff);
	uart_write_async("\n");
}

int main(void)
{
	DDRC = (1<<PINC0);
	TCCR0B = (1<<CS02); //timer0: Prescaler = 256 => ~122 Hz
	TIMSK0 = (1<<TOIE0); //timer0: Enable overflow interrupt
	uart_init();
	uart_init_tx();
	sei();
	core_init(2);
	uint8_t* state = malloc(1);
	uint8_t exit_code = core_register_field(0, state, 1, TRUE, TRUE);
	if(exit_code)
	{
		char* data = malloc(16);
		sprintf(data, "REG_ERROR: %u\n", exit_code);
		uart_write_async(data);
		free(data);
	}
	uint8_t* data = malloc(10);
	exit_code = core_register_field(1, data, 10, TRUE, TRUE);
	if(exit_code)
	{
		char* data = malloc(16);
		sprintf(data, "REG_ERROR: %u\n", exit_code);
		uart_write_async(data);
		free(data);
	}
	core_set_write_callback(callback);
	uart_write_async("INIT_CORE: DONE\n");
	#ifdef TX
		uint8_t addr_one[5] = {0x13, 0x37, 0x13, 0x37, 0x42};
		uint8_t addr_two[5] = {0x42, 0x42, 0x42, 0x42, 0x42};
		uart_write_async("INIT_REMOTE.RX_ADDR: 0x13 0x37 0x13 0x37 0x42\n");
		uart_write_async("INIT_REMOTE.TX_ADDR: 0x42 0x42 0x42 0x42 0x42\n");
	#endif
	#ifdef RX
		uint8_t addr_one[5] = {0x42, 0x42, 0x42, 0x42, 0x42};
		//uint8_t addr_two[5] = {0x13, 0x37, 0x13, 0x37, 0x42};
		uart_write_async("INIT_REMOTE.RX_ADDR: 0x42 0x42 0x42 0x42 0x42\n");
	#endif
	core_remote_init(addr_one, 5);
	uart_write_async("INIT_REMOTE: DONE\n");
	for(uint8_t i = 0; i <= 0x09; i++)
	{
		dump_config(i, 1);
	}
	dump_config(0x0A, 5);
	dump_config(0x0B, 5);
	for(uint8_t i = 0x0C; i <= 0x0F; i++)
	{
		dump_config(i, 1);
	}
	dump_config(0x10, 5);
	for(uint8_t i = 0x11; i <= 0x17; i++)
	{
		dump_config(i, 1);
	}
	dump_config(0x1C, 1);
	dump_config(0x1D, 1);
	#ifdef TX
		uint8_t str[5] = {0x74, 0x65, 0x73, 0x74, 0x00};
		core_remote_write_field(addr_two, 5, 1, str, 0, 5);
		uart_write_async("TX: DATA WRITTEN\n");
	#endif
	while(1)
    {
        set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_enable();
		sleep_cpu();
		#ifdef RX
			exit_code = core_remote_main();
			if(exit_code)
			{
				char* data = malloc(16);
				sprintf(data, "ERROR: %u\n", exit_code);
				uart_write_async(data);
				free(data);
			}
		#endif
    }
}

ISR(TIMER0_OVF_vect)
{
	
}