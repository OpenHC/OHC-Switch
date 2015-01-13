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

ohc_core_field_t**	core_fields = NULL;
uint16_t			core_fieldnum = 0;
void				(*core_write_callback)(uint16_t);

//Initializes OHC_CORE. Must be called before calling any other functions from core
uint8_t core_init(uint16_t fieldnum)
{
	if(fieldnum > 0)
	{
		core_fieldnum = fieldnum;
		core_fields = malloc(fieldnum * sizeof(ohc_core_field_t*));
		if(core_fields == NULL)
			return CORE_ERROR_OUT_OF_MEM;
	}
	return CORE_OK;
}

//Register a data field on this device. IDs start at 0
uint8_t core_register_field(uint16_t id, uint8_t* fieldptr, uint16_t length, uint8_t r, uint8_t w)
{
	if(id >= core_fieldnum)
		return CORE_ERROR_NO_SUCH_FIELD;
	ohc_core_field_t* field = malloc(sizeof(ohc_core_field_t));
	if(field == NULL)
		return CORE_ERROR_OUT_OF_MEM;
	field->data = fieldptr;
	field->length = length;
	field->perms.r = r;
	field->perms.w = w;
	core_fields[id] = field;
	return CORE_OK;
}

//Unregisters a data field on this device. IDs start at 0
uint8_t core_unregister_field(uint16_t id)
{
	if(id >= core_fieldnum)
		return CORE_ERROR_NO_SUCH_FIELD;
	free(core_fields[id]);
	core_fields[id] = NULL;
	return CORE_OK;
}

//Writes data to a field. Use for local field writes.
//Ignores field permissions
uint8_t core_write_field(uint16_t id, uint8_t* data, uint16_t offset, uint16_t length)
{
	if(id >= core_fieldnum)
		return CORE_ERROR_NO_SUCH_FIELD;
	ohc_core_field_t* field = core_fields[id];
	if(field == NULL)
		return CORE_ERROR_EMPTY_FIELD;
	if(length > field->length)
		return CORE_ERROR_OUT_OF_FIELD_BOUNDS;
	memcpy(field->data, data + offset, length);
	if(core_write_callback != NULL)
		core_write_callback(id);
	return CORE_OK;
}

//Reads data from a field. Use for local field reads.
//Ignores field permissions
uint8_t core_read_field(uint16_t id, uint8_t* data, uint16_t offset, uint16_t length)
{
	if(id >= core_fieldnum)
		return CORE_ERROR_NO_SUCH_FIELD;
	ohc_core_field_t* field = core_fields[id];
	if(field == NULL)
		return CORE_ERROR_EMPTY_FIELD;
	if((length + offset) > field->length)
		return CORE_ERROR_OUT_OF_FIELD_BOUNDS;
	memcpy(data, field->data + offset, length);
	return CORE_OK;
}

//Writes data to a field. Used for remote field writes.
//Respects field permissions
uint8_t core_write_field_ext(uint16_t id, uint8_t* data, uint16_t offset, uint16_t length)
{
	if(id >= core_fieldnum)
		return CORE_ERROR_NO_SUCH_FIELD;
	ohc_core_field_t* field = core_fields[id];
	if(field == NULL)
		return CORE_ERROR_EMPTY_FIELD;
	if(!field->perms.w)
		return CORE_ERROR_PERMISSION_DENIED;
	if(length > field->length)
		return CORE_ERROR_OUT_OF_FIELD_BOUNDS;
	memcpy(field->data, data + offset, length);
	if(core_write_callback != NULL)
		core_write_callback(id);
	return CORE_OK;
}

//Reads data from a field. Used for remote field writes.
//Respects field permissions
uint8_t core_read_field_ext(uint16_t id, uint8_t* data, uint16_t offset, uint16_t length)
{
	if(id >= core_fieldnum)
		return CORE_ERROR_NO_SUCH_FIELD;
	ohc_core_field_t* field = core_fields[id];
	if(field == NULL)
		return CORE_ERROR_EMPTY_FIELD;
	if(!field->perms.r)
		return CORE_ERROR_PERMISSION_DENIED;
	if((length + offset) > field->length)
		return CORE_ERROR_OUT_OF_FIELD_BOUNDS;
	memcpy(data, field->data + offset, length);
	return CORE_OK;
}

//Sets a callback function to get called on field writes
uint8_t core_set_write_callback(void (*callback)(uint16_t))
{
	core_write_callback = callback;
	return CORE_OK;
}

//Removes the set callback function
uint8_t core_clear_write_callback(void)
{
	core_write_callback = NULL;
	return CORE_OK;
}