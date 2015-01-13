/*
Copyright (c) 2014 Tobias Schramm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

uint8_t* core_remote_addr = NULL;
uint8_t core_remote_addr_len = 0;

struct
{
	uint8_t sending:1;
	uint8_t data_ready:1;
} core_remote_NRF;

//Sets up NRF24L01 lib and module. Must be called before usage of any
//core_remote_* functions
uint8_t core_remote_init(uint8_t* addr, uint8_t addr_len)
{
	//Initialze flags
	core_remote_NRF.sending = FALSE;
	core_remote_NRF.data_ready = FALSE;
	//Interrupts
	EICRA = (1<<ISC01);	//Set external interrupt on falling edge for INT0
	EIMSK = (1<<INT0);	//Enable INT0
	//NRF setup
	if(addr_len < NRF24L01_CONST_MIN_ADDR_LEN)
		return CORE_REMOTE_ERROR_ADDR_TOO_SHORT;
	if(addr_len > NRF24L01_CONST_MAX_ADDR_LEN)
		return CORE_REMOTE_ERROR_ADDR_TOO_LONG;
	core_remote_addr = malloc(addr_len);
	if(core_remote_addr == NULL)
		return CORE_ERROR_OUT_OF_MEM;
	memcpy(core_remote_addr, addr, addr_len);
	core_remote_addr_len = addr_len;
	NRF24L01_init();
	_delay_ms(200);
	NRF24L01_set_rx_addr(1, addr, addr_len);
	return CORE_OK;
}

//Writes a field on a remote device if permitted
uint8_t core_remote_write_field(uint8_t* addr, uint8_t addr_len, uint16_t id, uint8_t* data, uint16_t offset, uint16_t length)
{
	if(addr_len < NRF24L01_CONST_MIN_ADDR_LEN)
		return CORE_REMOTE_ERROR_ADDR_TOO_SHORT;
	if(addr_len > NRF24L01_CONST_MAX_ADDR_LEN)
		return CORE_REMOTE_ERROR_ADDR_TOO_LONG;
	if(length > (WIRELESS_PACK_LEN - CORE_REMOTE_PACKET_OVERHEAD))
		return CORE_REMOTE_ERROR_PAYLOAD_TOO_LONG;
	NRF24L01_set_tx_addr(addr, addr_len);
	NRF24L01_set_rx_addr(0, addr, addr_len);
	NRF24L01_set_tx();
	uint8_t* packet = malloc(WIRELESS_PACK_LEN);
	if(packet == NULL)
		return CORE_ERROR_OUT_OF_MEM;
	uint8_t* packptr = packet;
	memcpy(packet, addr, addr_len); //Copy address
	packptr += 5;
	*packptr =	length |	//Data length
				(1 << 5) |	//Write data
				(0 << 6) |	//Nope. This is not a continuation
				(1 << 7);	//For now this is the last packet
	packptr += 1;
	memcpy(packptr, &id, 2); //Copy field address
	packptr += 2;
	memcpy(packptr, data + offset, length);
	NRF24L01_send_data(packet, WIRELESS_PACK_LEN);
	free(packet);
	core_remote_NRF.sending = TRUE;
	return CORE_OK;
}

//Call in main program loop. Handles data reception.
//Main program loop must use sleep functions and wake
//up on external interrupt
uint8_t core_remote_main(void)
{
	uint8_t exit_code = CORE_OK;
	if(core_remote_NRF.data_ready)
	{
		uint8_t* data = malloc(WIRELESS_PACK_LEN);
		if(data == NULL)
			return CORE_ERROR_OUT_OF_MEM;
		NRF24L01_get_received_data(data, WIRELESS_PACK_LEN);
		uint8_t* packptr = data;
		packptr += 5;
		uint8_t data_len = *packptr & 0b11111;
		uint8_t rw = *packptr & 0b100000; //Ignoring bit 6 and 7 so far
		packptr += 1;
		uint16_t id;
		memcpy(&id, packptr, 2);
		packptr += 2;
		core_remote_NRF.data_ready = NRF24L01_data_ready();
		if(rw)
		{
			//Write
			exit_code = core_write_field_ext(id, packptr, 0, data_len);
		}
		else
		{
			//Read	
		}
		free(data);
	}
	return exit_code;
}

//NRF24L01 event handler. Sets flags for core_remote_main
ISR(INT0_vect)
{
	uint8_t status = NRF24L01_get_status();
	if(status & NRF24L01_MASK_STATUS_TX_DS)
	{
		NRF24L01_set_rx();
		core_remote_NRF.sending = FALSE;
	}
	if(status & NRF24L01_MASK_STATUS_RX_DR)
	{
		core_remote_NRF.data_ready = TRUE;
	}
	NRF24L01_LOW_set_register(NRF24L01_REG_STATUS, status & (NRF24L01_MASK_STATUS_TX_DS | NRF24L01_MASK_STATUS_MAX_RT));
	if(status & NRF24L01_MASK_STATUS_MAX_RT)
	{
		
	}
}
