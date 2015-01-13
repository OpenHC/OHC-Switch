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

/************************************************************************/
/* HIGH functions                                                       */
/************************************************************************/

///Basic NRF24L01 initialization. Enables pipe 0 and pipe 1. Applies configuration from config/wireless.h
extern void NRF24L01_init(void)
{
	NRF24L01_LOW_init_IO();
	spi_init();
	NRF24L01_flush_rx();
	NRF24L01_flush_tx();
	NRF24L01_LOW_set_register(NRF24L01_REG_STATUS, NRF24L01_MASK_STATUS_MAX_RT | NRF24L01_MASK_STATUS_RX_DR | NRF24L01_MASK_STATUS_TX_DS);
	nrf24l01_config_t* config = malloc(sizeof(nrf24l01_config_t));
	config->value = 0;
	#if NRF24L01_PRESET_RX == TRUE
		config->prim_rx = 1;
	#endif
	config->pwr_up = 1;
	config->crco = 1;
	#if WIRELESS_EN_RT_IRQ != TRUE
		config->mask_max_rt = 1;
	#endif
	#if WIRELESS_EN_TX_IRQ != TRUE
		config->mask_tx_ds = 1;
	#endif
	#if WIRELESS_EN_RX_IRQ != TRUE
		config->mask_rx_dr = 1;
	#endif
	config->en_crc = 1;
	NRF24L01_LOW_set_register(NRF24L01_REG_CONFIG, config->value);
	free(config);
	nrf24l01_rf_ch_t* rf_ch = malloc(sizeof(nrf24l01_rf_ch_t));
	rf_ch->value = 0;
	rf_ch->rf_ch = WIRELESS_CHANNEL;
	NRF24L01_LOW_set_register(NRF24L01_REG_RF_CH, rf_ch->value);
	free(rf_ch);
	nrf24l01_rf_setup_t* rf_setup = malloc(sizeof(nrf24l01_rf_setup_t));
	rf_setup->value = 0;
	rf_setup->rf_pwr = NRF24L01_PRESET_TXPWR;
	rf_setup->rf_dr_high = NRF24L01_PRESET_BAUDRATE_HIGH;
	rf_setup->rf_dr_low = NRF24L01_PRESET_BAUDRATE_LOW;
	NRF24L01_LOW_set_register(NRF24L01_REG_RF_SETUP, rf_setup->value);
	free(rf_setup);
	nrf24l01_rx_pw_t* payload_width = malloc(sizeof(nrf24l01_rx_pw_t));
	payload_width->rx_pw = WIRELESS_PACK_LEN;
	NRF24L01_LOW_set_register(NRF24L01_REG_RX_PW_P0, payload_width->value);
	NRF24L01_LOW_set_register(NRF24L01_REG_RX_PW_P1, payload_width->value);
	free(payload_width);	
	NRF24L01_CE_HIGH;
}

///Initializes data transmission. Automatically sets the NRF24L01 to TX mode
extern void NRF24L01_send_data(uint8_t* data, uint8_t len)
{
	NRF24L01_CE_LOW;
	NRF24L01_set_tx();
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_FLUSH_TX);
	NRF24L01_CSN_HIGH;
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_W_TX_PAYLOAD);
	spi_transmit_sync(data, len);
	NRF24L01_CSN_HIGH;
	NRF24L01_CE_HIGH;
	_delay_us(10);
	NRF24L01_CE_LOW;
}

///Wakes the NRF24L01 from standby mode.
extern void NRF24L01_power_up(void)
{
	uint8_t config = NRF24L01_LOW_get_register(NRF24L01_REG_STATUS);
	config |= NRF24L01_MASK_CONFIG_PWR_UP;
	NRF24L01_LOW_set_register(NRF24L01_REG_STATUS, config);
}

///Puts the NRF24L01 into standby mode.
extern void NRF24L01_power_down(void)
{
	uint8_t config = NRF24L01_LOW_get_register(NRF24L01_REG_STATUS);
	config &= ~NRF24L01_MASK_CONFIG_PWR_UP;
	NRF24L01_LOW_set_register(NRF24L01_REG_STATUS, config);
}

///Sets the frequency channel the NRF24L01 operates on.
///Must be inside range [0 .. 125]
extern void NRF24L01_set_channel(uint8_t channel)
{
	uint8_t rf_ch = channel;
	rf_ch &= NRF24L01_MASK_RF_CH_RF_CH;
	NRF24L01_LOW_set_register(NRF24L01_REG_RF_CH, rf_ch);
}

///Sets the tx power of the NRF24L01
///Must be inside range [0 .. 3]
///0 = -18 dBm -> 0.016 mW
///1 = -12 dBm -> 0.063 mW
///2 = - 6 dBm -> 0.251 mW
///3 =   0 dBm -> 1.000 mW
extern void NRF24L01_set_tx_pwr(uint8_t tx_pwr)
{
	uint8_t rf_setup = NRF24L01_LOW_get_register(NRF24L01_REG_RF_SETUP);
	rf_setup &= ~NRF24L01_MASK_RF_SETUP_RF_PWR;
	rf_setup |= (tx_pwr << 1) & NRF24L01_MASK_RF_SETUP_RF_PWR;
	rf_setup &= NRF24L01_MASK_RF_SETUP;
	NRF24L01_LOW_set_register(NRF24L01_REG_RF_SETUP, rf_setup);
}

///Enables the dynamic payload feature.
extern void NRF24L01_enable_dyn_pld(uint8_t enable)
{
	uint8_t features = NRF24L01_LOW_get_register(NRF24L01_REG_FEATURE);
	features &= ~NRF24L01_MASK_FEATURE_EN_DPL;
	features |= NRF24L01_MASK_FEATURE_EN_DPL & ((enable ? TRUE : FALSE) << 2);
	NRF24L01_LOW_set_register(NRF24L01_REG_FEATURE, features);
}

///Enables the dynamic payload feature for the given pipe.
///Overrides the payload width setting for the pipe.
extern void NRF24L01_enable_dyn_pld_pipe(uint8_t pipe, uint8_t state)
{
	uint8_t dynpld_pipes = NRF24L01_LOW_get_register(NRF24L01_REG_DYNPD);
	if(state)
	dynpld_pipes |= (1 << pipe);
	else
	dynpld_pipes &= ~(1 << pipe);
	NRF24L01_LOW_set_register(NRF24L01_REG_DYNPD, dynpld_pipes);
}

///Enables the acknowledge payload feature. Should only be used with dynamic
///payload size. Otherwise ack payloads my be discarded by the receiver
extern void NRF24L01_enable_ack_pld(uint8_t enable)
{
	uint8_t features = NRF24L01_LOW_get_register(NRF24L01_REG_FEATURE);
	features &= ~NRF24L01_MASK_FEATURE_EN_ACK_PAY;
	features |= NRF24L01_MASK_FEATURE_EN_ACK_PAY & ((enable ? TRUE : FALSE) << 1);
	NRF24L01_LOW_set_register(NRF24L01_REG_FEATURE, features);
}

///Enables the dynamic acknowledge feature. This feature allows single packets to
///contain a 'do not acknowledge' flag. The receiver will then not send an automatic 
///acknowledgment for this packet.
extern void NRF24L01_enable_dyn_ack(uint8_t enable)
{
	uint8_t features = NRF24L01_LOW_get_register(NRF24L01_REG_FEATURE);
	features &= ~NRF24L01_MASK_FEATURE_EN_DYN_ACK;
	features |= NRF24L01_MASK_FEATURE_EN_DYN_ACK & (enable ? TRUE : FALSE);
	NRF24L01_LOW_set_register(NRF24L01_REG_FEATURE, features);
}

///Enables / Disables auctomatic packet acknowledgement for the givent pipe.
///By default all pipes have this feature turned on.
extern void	NRF24L01_set_autoack_pipe(uint8_t pipe, uint8_t state)
{
	uint8_t auto_ack_pipes = NRF24L01_LOW_get_register(NRF24L01_REG_EN_AA);
	auto_ack_pipes &= ~(1<<pipe);
	auto_ack_pipes |= (1<<pipe);
	NRF24L01_LOW_set_register(NRF24L01_REG_EN_AA, auto_ack_pipes);
}

///Sets the data rate used on the RF side
///Must be inside range [0 .. 2]
///0 = 0.25 Mbps
///1 = 1.00 Mbps
///2 = 2.00 Mbps
///0.25 Mbps mode works only with NRF24L01+
extern void NRF24L01_set_rf_dr(uint8_t data_rate)
{
	uint8_t rf_setup = NRF24L01_LOW_get_register(NRF24L01_REG_RF_SETUP);
	switch(data_rate)
	{
		case 0: rf_setup &=	~NRF24L01_MASK_RF_SETUP_RF_DR_HIGH; rf_setup |=	NRF24L01_MASK_RF_SETUP_RF_DR_LOW; break; //250 kbps
		case 1: rf_setup &=	~NRF24L01_MASK_RF_SETUP_RF_DR_HIGH; rf_setup &=	~NRF24L01_MASK_RF_SETUP_RF_DR_LOW; break; //1 Mbps
		case 2: rf_setup |=	NRF24L01_MASK_RF_SETUP_RF_DR_HIGH; rf_setup &= ~NRF24L01_MASK_RF_SETUP_RF_DR_LOW; break; //2 Mbps
	}
	rf_setup &= NRF24L01_MASK_RF_SETUP;
	NRF24L01_LOW_set_register(NRF24L01_REG_RF_SETUP, rf_setup);
}

///Reads fifo status and returns if rx fifo is not empty
extern uint8_t NRF24L01_data_ready(void)
{
	uint8_t fifo_status = NRF24L01_LOW_get_register(NRF24L01_REG_FIFO_STATUS);
	if((fifo_status & NRF24L01_MASK_FIFO_STATUS_RX_EMPTY) == 0)
		return TRUE;
	else
		return FALSE;
}

///Reads data received by the NRF24L01
extern void NRF24L01_get_received_data(uint8_t* data, uint8_t len)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_R_RX_PAYLOAD);
	spi_transfer_sync(data, data, len);
	NRF24L01_CSN_HIGH;
	NRF24L01_LOW_set_register(NRF24L01_REG_STATUS, NRF24L01_MASK_STATUS_RX_DR);
}

///Reads the NRF24L01's actual status
extern uint8_t NRF24L01_get_status(void)
{
	return NRF24L01_LOW_read_byte(NRF24L01_CMD_NOP);
}

///Reads the pipe data was received on from the NRF24L01's status
uint8_t	NRF24L01_get_pipe_from_status(uint8_t status)
{
	return (status & NRF24L01_MASK_STATUS_RX_P_NO) << 1;
}

extern uint8_t NRF24L01_get_payload_len()
{
	NRF24L01_CSN_LOW;
	return spi_fast_shift(NRF24L01_CMD_R_RX_PL_WID);
	NRF24L01_CSN_HIGH;
}

///Writes the additional payload to be transmitted together with the auto 
///acknowledgment data on the given pipe. Works only when dynamic payload 
///and acknowledgment payload feature are turned on
void NRF24L01_write_ack_payload(uint8_t pipe, uint8_t* data, uint8_t len)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_W_ACK_PAYLOAD | pipe);
	spi_transmit_sync(data, len);
	NRF24L01_CSN_HIGH;
}


///NRF24L01 ONLY
///Calling this function toggles the availability of the acknowledgment payload, dynamic 
///payload and dynamic acknowledgment bits int he feature register. Disabled by default.
void NRF24L01_activate(void)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_ACTIVATE);
	spi_fast_shift(0x73);
	NRF24L01_CSN_HIGH;
}

///Enables / Disables single data pipes. By default pipe 0 and pipe 1 are enabled
void NRF24L01_enable_pipe(uint8_t pipe, uint8_t state)
{
	uint8_t pipes = NRF24L01_LOW_get_register(NRF24L01_REG_EN_RXADDR);
	if(state)
		pipes |= (1<<pipe);
	else
		pipes &= ~(1<<pipe);
	NRF24L01_LOW_set_register(NRF24L01_REG_EN_RXADDR, pipes);
}

///Sets the tx address of the NRF24L01
void NRF24L01_set_tx_addr(uint8_t* addr, uint8_t len)
{
	NRF24L01_LOW_write_register(NRF24L01_REG_TX_ADDR, addr, len);
}

///Sets the rx address for the specified pipe
void NRF24L01_set_rx_addr(uint8_t pipe, uint8_t* addr, uint8_t len)
{
	NRF24L01_LOW_write_register(NRF24L01_REG_RX_ADDR_P0 + pipe, addr, len);
}

///Sets the payload length for the specified pipe
void NRF24L01_set_payload_width(uint8_t pipe, uint8_t width)
{
	NRF24L01_LOW_set_register(NRF24L01_REG_RX_PW_P0 + pipe, width & NRF24L01_MASK_RX_PW_P0);
}

///Puts the NRF24L01 into receive mode
void NRF24L01_set_rx(void)
{
	uint8_t config = NRF24L01_LOW_get_register(NRF24L01_REG_CONFIG);
	config |= NRF24L01_MASK_CONFIG_PRIM_RX;
	config |= NRF24L01_MASK_CONFIG_PWR_UP;
	config &= NRF24L01_MASK_CONFIG;
	NRF24L01_LOW_set_register(NRF24L01_REG_CONFIG, config);
	NRF24L01_CE_HIGH;
}

///Puts the NRF24L01 into transmit mode
void NRF24L01_set_tx(void)
{
	uint8_t config = NRF24L01_LOW_get_register(NRF24L01_REG_CONFIG);
	config &= ~NRF24L01_MASK_CONFIG_PRIM_RX;
	config |= NRF24L01_MASK_CONFIG_PWR_UP;
	config &= NRF24L01_MASK_CONFIG;
	NRF24L01_LOW_set_register(NRF24L01_REG_CONFIG, config);
}

///Flushes the NRF's receive packet buffer
extern void NRF24L01_flush_rx()
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_FLUSH_RX);
	NRF24L01_CSN_HIGH;
}

///Flushes the NRF's transmit packet buffer
extern void NRF24L01_flush_tx()
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_FLUSH_TX);
	NRF24L01_CSN_HIGH;
}

/************************************************************************/
/* LOW functions                                                        */
/************************************************************************/

///Initializes the CS and CSN outputs
void NRF24L01_LOW_init_IO(void)
{
	MODULE_CE_DDR	|= (1<<MODULE_CE_PIN);
	MODULE_CSN_DDR	|= (1<<MODULE_CSN_PIN);
	NRF24L01_CE_LOW;
	NRF24L01_CSN_HIGH;
}

///Sets one byte in the NRF24L01's config registers
void NRF24L01_LOW_set_register(uint8_t regaddr, uint8_t val)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_W_REGISTER | regaddr);
	spi_fast_shift(val);
	NRF24L01_CSN_HIGH;
}

///Gets one byte from the NRF24L01's config registers
uint8_t NRF24L01_LOW_get_register(uint8_t regaddr)
{
	uint8_t byte;
	NRF24L01_LOW_read_register(regaddr, &byte, 1);
	return byte;
}

///Sets multi byte registers in the NRF24L01
void NRF24L01_LOW_write_register(uint8_t regaddr, uint8_t* data, uint8_t len)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_W_REGISTER | regaddr);
	spi_transmit_sync(data, len);
	NRF24L01_CSN_HIGH;
}

///Reads multi byte registers from the NRF24L01
void NRF24L01_LOW_read_register(uint8_t regaddr, uint8_t* data, uint8_t len)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_R_REGISTER | regaddr);
	spi_transfer_sync(data, data, len);
	NRF24L01_CSN_HIGH;
}

///Sends a single one byte command to the NRF24L01 and gets the single byte response
uint8_t NRF24L01_LOW_read_byte(uint8_t cmd)
{
	NRF24L01_CSN_LOW;
	uint8_t data = spi_fast_shift(cmd);
	NRF24L01_CSN_HIGH;
	return data;
}