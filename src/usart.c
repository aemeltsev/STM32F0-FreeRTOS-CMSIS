#include "usart.h"

// Buffers for receiving and transmitting data
volatile uint8_t tx_buffer[UART_BUF_SIZE];
volatile uint8_t tx_ready = 1; // A flag that transmit data ready
volatile uint32_t tx_data_len = 0; // A length of transmit data
volatile uint8_t tx_index = 0; // Indx of current tansmit byte

uint8_t rx_buffer1[UART_BUF_SIZE];
uint8_t rx_buffer2[UART_BUF_SIZE];
uint8_t *current_rx_buffer = rx_buffer1;
uint8_t *processing_rx_buffer = rx_buffer2;
volatile uint32_t rx_index = 0;
volatile uint8_t buffer_switch_flag = 0;
volatile uint8_t data_ready = 0; // A flag indicating that the data is ready for processing

volatile uint8_t last_rx_len;


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
void usart1_gpio_init(uint32_t baudrate, uint32_t clock)
{
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // Configure PA9, PA10 on the AF1 (USART1)
    GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);
    GPIOA->AFR[1] |= (0x01 << GPIO_AFRH_AFSEL9_Pos) | (0x01 << GPIO_AFRH_AFSEL10_Pos);

    USART1->BRR = clock / baudrate;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    USART1->CR1 |= USART_CR1_RXNEIE; // Enable interrupt by receiving byte
    NVIC_SetPriority(USART1_IRQn, 0); // Hight priority
    NVIC_EnableIRQ(USART1_IRQn);      // Enable in NVIC
}

void usart1_send_data(uint8_t *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        tx_buffer[i] = data[i]; // Loading data into an intermediate buffer
    }
    tx_index = 0;
    USART1->CR1 |= USART_CR1_TXEIE; // Enabling data transfer interrupt
}

void usart1_send_byte(uint8_t data)
{
    while (!(USART1->ISR & USART_ISR_TXE)); // Waiting for the transfer register to be released
    USART1->TDR = data;
    while (!(USART1->ISR & USART_ISR_TC)); // Waiting for transmission complete
}

void usart1_receive_byte(uint8_t *data)
{
    while (!(USART1->ISR & USART_ISR_RXNE));
    *data = USART1->RDR; // Read data from receive register
}

uint8_t usart1_getline(uint8_t **line)
{
    // Check if data ready
    if (!data_ready) {
        *line = 0;
        return 0; // There is not data for processing
    }

    // Get pointer to the buffer with data for processing
    *line = processing_rx_buffer;

    // Get data length in the buffer
    uint32_t length = rx_index;

    // Reset flag of data ready
    data_ready = 0;

    // Return data length
    return length;
}

tx_status usart1_send(const uint8_t *str, uint8_t len)
{
    if (len > UART_BUF_SIZE) return STR_TOO_LONG;
    if (!tx_ready) return LINE_BUSY; // Checking that the previous submission has completed

    tx_ready = 0;
    tx_data_len = len;
    for (uint8_t i = 0; i < len; i++) {
        tx_buffer[i] = str[i];
    }
    tx_index = 0;
    USART1->CR1 |= USART_CR1_TXEIE; // Enabling data transfer interrupt

    return ALL_OK;
}

void usart1_send_hex(uint8_t num)
{
    char hex_chars[] = "0123456789ABCDEF";
    usart1_send_byte(hex_chars[(num >> 4) & 0x0F]); // High tetrad
    usart1_send_byte(hex_chars[num & 0x0F]);        // Low tetrad
    usart1_send_byte(' ');                          // Space for readability
}

void USART1_IRQHandler(void)
{
    uint32_t isr_reg = USART1->ISR;

    // --- IF TRANSMIT COMPLETE ---
    if (USART1->ISR & USART_ISR_TC)
    {
        USART1->ISR |= USART_ICR_TCCF; // Clean flag of complete transmit
        //txrdy = 1;
    }

    // --- RECEIVE DATA ---
    if (isr_reg & USART_ISR_RXNE) // Interrupt by RX data not empty - received is the uint8_t value
    {
        uint8_t received_byte = (uint8_t)USART1->RDR; // Read data into the intermediate buffer
        
        if (rx_index < UART_BUF_SIZE) {
            current_rx_buffer[rx_index++] = received_byte;
            
            // End-of-line condition (e.g. '\n')
            if (received_byte == '\n' || rx_index == UART_BUF_SIZE) {
                last_rx_len = rx_index; // Save the length of this particular string
                
                // Buffers switching
                uint8_t *temp = current_rx_buffer;
                current_rx_buffer = processing_rx_buffer;
                processing_rx_buffer = temp;
                
                data_ready = 1;
                rx_index = 0; 
            }
        }
    }

    // --- TRANSMIT DATA ---
    if ((isr_reg & USART_ISR_TXE) && (USART1->CR1 & USART_CR1_TXEIE)) {
        if (tx_index < tx_data_len) {
            USART1->TDR = tx_buffer[tx_index++];
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE; // If data is ending, the interrupt on
            tx_ready = 1;
        }
    }

    if (USART1->ISR & USART_ISR_ORE) { // Overflow error
        USART1->ICR |= USART_ICR_ORECF;
        // Handling overflow error
    }

    if (USART1->ISR & USART_ISR_FE) { // Frame error
        USART1->ICR |= USART_ICR_FECF;
        // Handling frame error
    }

    if (USART1->ISR & USART_ISR_PE) { // Even error
        USART1->ICR |= USART_ICR_PECF;
        // Handling even error
    }
}


/*
int main(void)
{
    // 1. UART initialization (115200 baud at 48 MHz)
    // The frequency depends on the clock settings in system_stm32f0xx.c
    usart1_gpio_init(115200, 48000000UL);

    // 2. Welcome message with metadata from the Makefile
    char startup_msg[64];
    usart1_send((uint8_t*)"\r\n--- SYSTEM START ---\r\n", 25);

    while (1) {
        uint8_t *rx_line = NULL;
        
        // 3. Check of input data
        uint8_t len = usart1_getline(&rx_line);
        
        if (len > 0 && rx_line != NULL) {
            // Echo-answer: send back receiving line
            usart1_send((uint8_t*)"Received command: ", 18);
            usart1_send(rx_line, len);
            
            // Simply command parser
            if (strncmp((char*)rx_line, "ver", 3) == 0) {
                usart1_send((uint8_t*)"\r\nFirmware Build: ", 18);
                usart1_send_hex(BUILD_NUMBER); // Using your HEX output function
            }
            
            usart1_send((uint8_t*)"\r\n> ", 4);
        }

        // Background indication (for example, to check that the MC has not frozen)
        delay_ms(10); 
    }
}
*/