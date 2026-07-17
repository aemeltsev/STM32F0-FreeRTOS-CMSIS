#include "uart_drv.h"
#include "modbus_slave.h"

/**
 * Критически важный нюанс чистой работы с RS-485 на CMSIS: нельзя переключать микросхему
 * в режим приема сразу после записи байта в регистр TDR. 
 * Флаг TXE (Transmit Data Register Empty) означает лишь то, 
 * что данные ушли из буфера в сдвиговый регистр. Переключать направление 
 * можно только после того, как взведется флаг TC (Transmission Complete), 
 * сигнализирующий, что последний бит физически покинул ножку процессора.
 */

 /*   
    MCU                CP210x/FT232/CH340 Dongle
    -------            -----------
    PA2  TX  <-------> RX  (or TX)
    PA3  RX  <-------> TX  (or RX)
         GND  <-------> GND
         3.3  <-------> 3.3V
 
    to check out the UART output, run the following command in host:
    `$ picocom -b 115200 /dev/ttyUSB1`
    the USB-UART device path may be `/dev/ttyUSB0` or other else.
    press `Ctrl+a, Ctrl+x` to exit `picocom`.
*/
 void uart2_init(uint32_t baudrate, uint32_t clock)
 {
    // 1. Enable peripheral system clocks for USART2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // Настройка скорости
    // 2. Задаем точный коэффициент деления бодрейта в регистр BRR с математическим округлением
    USART2->BRR = clock / baudrate;
    //USART2->BRR = (clock + (baudrate / 2U)) / baudrate;

    // 3. Активируем встроенный аппаратный автомат управления линией Driver Enable (DE)
    // CR3_DEM = Режим Driver Enable включен. 
    // CR3_DEP = Полярность сигнала DE (0 = Активный HIGH, стандарт для чипов MAX485/ST485)
    USART2->CR3 |= USART_CR3_DEM;
    USART2->CR3 &= ~USART_CR3_DEP;

    // 4. Задаем защитные времена упреждения и удержания пина трансивера (в 1/16 долях длительности бита)
    USART2->CR1 |= (1U << USART_CR1_DEAT_Pos) | (1U << USART_CR1_DEDT_Pos);

    // 5. Frame configurations: 8-N-1 format, activate Transmitter, Receiver and RX-Register-Not-Empty Interrupt
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART2->CR1 |= USART_CR1_RXNEIE; // Enable interrupt by receiving byte

    // 6. Запускаем модуль USART2 в работу
    USART2->CR1 |= USART_CR1_UE;

    // RS485_RX_ENABLE();

    // 7. Разрешаем и настраиваем векторы прерываний в контроллере NVIC ядра
    NVIC_SetPriority(USART2_IRQn, 1); // Средний уровень приоритета обработки
    NVIC_EnableIRQ(USART2_IRQn);      // Enable in NVIC
 }


void uart2_send_buffer(const uint8_t *p_data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++){
        // Блокирующий цикл: ждем, пока освободится внутренний буфер передатчика (TXE)
        while (!(USART2->ISR & USART_ISR_TXE));
        
        // Передаем байт. Процессор сам аппаратно поднимет пин PA1 (DE) в состояние логической «1»!
        USART2->TDR = p_data[i];
    }
    // Безопасность: жестко ждем, пока последний стоп-бит физически покинет ножку процессора (TC)
    while (!(USART2->ISR & USART_ISR_TC));
    // В эту же микросекунду кремниевый автомат аппаратно опустит PA1 (DE) обратно в логический «0».
}

/**
 * @brief  Аппаратный обработчик прерывания для транзакций USART2.
 */
void USART2_IRQHandler(void)
{
        // Проверяем, вызвано ли прерывание приходом нового сетевого байта в регистр данных
    if (USART2->ISR & USART_ISR_RXNE)
    {
        uint8_t received_byte = (uint8_t)(USART2->RDR);

        // Мгновенно сбрасываем внешний таймер межсимвольной тишины Modbus T3.5 (TIM3)
        TIM3->CNT = 0;
        TIM3->CR1 |= TIM_CR1_CEN;

        // Передаем принятый байт напрямую в абстрагированные хуки протокольного стека Modbus
        Modbus_RxISR(received_byte);
    }

    // Аппаратная защита от переполнения (сбрасывает флаг, если байты летят быстрее, чем main успевает их разобрать)
    if (USART2->ISR & USART_ISR_ORE)
    {
        USART2->ICR |= USART_ICR_ORECF;
    }
}