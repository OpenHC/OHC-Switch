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

#ifndef __OHC_CORE_H__
#define __OHC_CORE_H__

	#include <stdlib.h>
	#include <string.h>
	
	//Typedef
	typedef struct  
	{
		uint8_t*	data;
		uint16_t	length;
		struct
		{
			uint8_t r:1;
			uint8_t w:1;
		}
		perms;
	}
	ohc_core_field_t;
	
	//Exit codes
	#define CORE_OK 0
	#define CORE_ERROR_OUT_OF_MEM 1
	#define CORE_ERROR_NO_SUCH_FIELD 2
	#define CORE_ERROR_EMPTY_FIELD 3
	#define CORE_ERROR_OUT_OF_FIELD_BOUNDS 4
	#define CORE_ERROR_PERMISSION_DENIED 5
	
	extern uint8_t core_init(uint16_t fieldnum);
	extern uint8_t core_register_field(uint16_t id, uint8_t* fieldptr, uint16_t length, uint8_t r, uint8_t w);
	extern uint8_t core_unregister_field(uint16_t id);
	extern uint8_t core_write_field_ext(uint16_t id, uint8_t* data, uint16_t offset, uint16_t length);
	extern uint8_t core_read_field(uint16_t id, uint8_t* data, uint16_t offset, uint16_t length);
	extern uint8_t core_write_field(uint16_t id, uint8_t* data, uint16_t offset, uint16_t length);
	extern uint8_t core_read_field_ext(uint16_t id, uint8_t* data, uint16_t offset, uint16_t length);
	extern uint8_t core_set_write_callback(void (*callback)(uint16_t));
	extern uint8_t core_clear_write_callback(void);
	
	#include "core.c"
	#ifdef __NRF24L01_LIB_H__
		#include "core_remote.h"
	#endif
#endif