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


Data packet:
	[0..4]					Address
	[5.5, 5.1, 5.1, 5.1]	Data len, Operation, Continuation, Last packet
	[6..7]					Field index
	[8..31]					Data
	
	Operation = 0 => read
	Operation = 1 => write
*/

#ifndef __OHC_CORE_REMOTE_H__
#define __OHC_CORE_REMOTE_H__
	
	//Exit codes
	#define CORE_REMOTE_ERROR_ADDR_TOO_SHORT	100
	#define CORE_REMOTE_ERROR_ADDR_TOO_LONG		101
	#define CORE_REMOTE_ERROR_PAYLOAD_TOO_LONG	102
	
	#define CORE_REMOTE_PACKET_OVERHEAD			8
		
	#if WIRELESS_PACK_LEN <= CORE_REMOTE_PACKET_OVERHEAD
		#error "NRF24L01 packet too short to store any data"
	#endif
	
	extern uint8_t core_remote_init(uint8_t* addr, uint8_t addr_len);
	extern uint8_t core_remote_write_field(uint8_t* addr, uint8_t addr_len, uint16_t id, uint8_t* data, uint16_t offset, uint16_t length);
	extern uint8_t core_remote_main(void);
	
	#include "core_remote.c"
#endif