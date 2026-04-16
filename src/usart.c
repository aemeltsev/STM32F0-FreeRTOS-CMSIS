#include "usart.h"

// Буферы для приема и передачи данных
volatile uint8_t tx_buffer[UART_BUF_SIZE];
volatile uint8_t tx_ready = 1; // Флаг готовности передачи
volatile uint32_t tx_data_len = 0; // Длина передаваемых данных
volatile uint8_t tx_index = 0; // Индекс текущего передаваемого байта

uint8_t rx_buffer1[UART_BUF_SIZE];
uint8_t rx_buffer2[UART_BUF_SIZE];
uint8_t *current_rx_buffer = rx_buffer1;
uint8_t *processing_rx_buffer = rx_buffer2;
volatile uint32_t rx_index = 0;
volatile uint8_t buffer_switch_flag = 0;
volatile uint8_t data_ready = 0; // Флаг, указывающий на готовность данных для обработки


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

    // Настройка PA9, PA10 на AF1 (USART1)
    GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);
    GPIOA->AFR[1] |= (0x01 << GPIO_AFRH_AFSEL9_Pos) | (0x01 << GPIO_AFRH_AFSEL10_Pos);

    USART1->BRR = clock / baudrate;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    USART1->CR1 |= USART_CR1_RXNEIE; // Включить прерывание по приему байта
    NVIC_SetPriority(USART1_IRQn, 0); // Максимальный приоритет
    NVIC_EnableIRQ(USART1_IRQn);      // Разрешить в NVIC
}

void usart1_send_data(uint8_t *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        tx_buffer[i] = data[i]; // Выгружаем данные в промежуточный буффер
    }
    tx_index = 0;
    USART1->CR1 |= USART_CR1_TXEIE; // Включение прерывания по передаче данных
}

void usart1_send_byte(uint8_t data)
{
    while (!(USART1->ISR & USART_ISR_TXE)); // Ожидание освобождения регистра передачи
    USART1->TDR = data;
    while (!(USART1->ISR & USART_ISR_TC)); // Ожидание успешной передачи
}

void usart1_receive_byte(uint8_t *data)
{
    while (!(USART1->ISR & USART_ISR_RXNE));
    *data = USART1->RDR; // Чтение данных из регистра приема
}

uint8_t usart1_getline(uint8_t **line)
{
    // Проверка на готовность данных
    if (!data_ready) {
        *line = 0;
        return 0; // Нет данных для обработки
    }

    // Установка указателя на буфер с данными для обработки
    *line = processing_rx_buffer;

    // Получение длины данных в буфере
    uint32_t length = rx_index;

    // Сброс флага готовности данных
    data_ready = 0;

    // Возврат длины данных
    return length;
}

tx_status usart1_send(const uint8_t *str, uint8_t len)
{
    if (len > UART_BUF_SIZE) return STR_TOO_LONG;
    if (!tx_ready) return LINE_BUSY; // Проверка, что прошлая отправка завершена

    tx_ready = 0;
    tx_data_len = len;
    for (uint8_t i = 0; i < len; i++) {
        tx_buffer[i] = str[i];
    }
    tx_index = 0;
    USART1->CR1 |= USART_CR1_TXEIE; // Включение прерывания по передаче данных

    return ALL_OK;
}

void usart1_send_hex(uint8_t num)
{
    char hex_chars[] = "0123456789ABCDEF";
    usart1_send_byte(hex_chars[(num >> 4) & 0x0F]); // Старшая тетрада
    usart1_send_byte(hex_chars[num & 0x0F]);        // Младшая тетрада
    usart1_send_byte(' ');                          // Пробел для читаемости
}

void USART1_IRQHandler(void)
{
    uint32_t isr_reg = USART1->ISR;

    // --- ПРИЕМ ДАННЫХ ---
    if (isr_reg & USART_ISR_RXNE) {
        uint8_t received_byte = (uint8_t)USART1->RDR;
        
        if (rx_index < UART_BUF_SIZE) {
            current_rx_buffer[rx_index++] = received_byte;
            
            // Условие конца строки (например, '\n')
            if (received_byte == '\n' || rx_index == UART_BUF_SIZE) {
                // Переключаем буферы
                uint8_t *temp = current_rx_buffer;
                current_rx_buffer = processing_rx_buffer;
                processing_rx_buffer = temp;
                
                data_ready = 1;
                // В реальной задаче rx_index нужно сохранять отдельно для каждого буфера
                // но для упрощения сбрасываем:
                rx_index = 0; 
            }
        }
    }

    // --- ПЕРЕДАЧА ДАННЫХ ---
    if ((isr_reg & USART_ISR_TXE) && (USART1->CR1 & USART_CR1_TXEIE)) {
        if (tx_index < tx_data_len) {
            USART1->TDR = tx_buffer[tx_index++];
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE; // Данные закончились, выключаем прерывание
            tx_ready = 1;
        }
    }
}


/*
int main(void)
{
    // 1. Инициализация UART (115200 бод при частоте 48МГц)
    // Частота зависит от настроек тактирования в system_stm32f0xx.c
    usart1_gpio_init(115200, 48000000UL);

    // 2. Приветственное сообщение с метаданными из Makefile
    char startup_msg[64];
    usart1_send((uint8_t*)"\r\n--- SYSTEM START ---\r\n", 25);

    while (1) {
        uint8_t *rx_line = NULL;
        
        // 3. Проверка входящих данных (ваша двойная буферизация)
        uint8_t len = usart1_getline(&rx_line);
        
        if (len > 0 && rx_line != NULL) {
            // Эхо-ответ: отправляем обратно полученную строку
            usart1_send((uint8_t*)"Received command: ", 18);
            usart1_send(rx_line, len);
            
            // Простейший парсер команд
            if (strncmp((char*)rx_line, "ver", 3) == 0) {
                usart1_send((uint8_t*)"\r\nFirmware Build: ", 18);
                usart1_send_hex(BUILD_NUMBER); // Используем вашу функцию вывода HEX
            }
            
            usart1_send((uint8_t*)"\r\n> ", 4);
        }

        // Фоновая индикация (например, для проверки, что МК не завис)
        delay_ms(10); 
    }
}
*/