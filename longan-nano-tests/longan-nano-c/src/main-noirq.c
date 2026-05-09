#include "gd32vf103.h"

void put_char(int ch)
{
	usart_data_transmit(USART0, (uint8_t)ch);

	while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
	{
	}
}

void delay(unsigned int cycles)
{

	while (cycles--) {
		__asm__ volatile ("nop");
	}
}

void main(void)
{
    /* enable clocks */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    /* connect LED ports */
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* connect USART ports */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* configure USART */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);

    put_char('R');
    put_char('S');
    put_char('T');
    put_char('\n');

    while (1) {
	    gpio_bit_set(GPIOA, GPIO_PIN_1);
	    gpio_bit_reset(GPIOA, GPIO_PIN_2);
	    put_char('A');
	    delay(200000);
	    gpio_bit_reset(GPIOA, GPIO_PIN_1);
	    gpio_bit_set(GPIOA, GPIO_PIN_2);
	    put_char('B');
	    delay(100000);
    }
}
