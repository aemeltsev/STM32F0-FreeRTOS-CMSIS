#include "usart.h"

/*   
    MCU                CP210x/FT232/CH340 Dongle
    -------            -----------
    PA9  TX  <-------> RX  (or TX)
    PA10 RX  <-------> TX  (or RX)
         GND  <-------> GND
         3.3  <-------> 3.3V
 
    to check out the UART output, run the following command in host:
    `$ picocom -b 115200 /dev/ttyUSB1`
    the USB-UART device path may be `/dev/ttyUSB0` or other else.
    press `Ctrl+a, Ctrl+x` to exit `picocom`.
*/
void usart1_gpio_init(void)
{

}

void configure_usart1(uint8_t word_len, uint8_t div_speed, uint8_t stop_bit, uint32_t usart_div, uint32_t clock)
{

}

void usart1_send_data(uint8_t *data, uint8_t len)
{

}

void usart1_send_byte(uint8_t data)
{

}

void usart1_receive_byte(uint8_t *data)
{

}

uint8_t usart1_getline(uint8_t **line)
{

}

tx_status usart1_send(const uint8_t *str, uint8_t len)
{

}

void usart1_send_hex(uint8_t num)
{
    
}