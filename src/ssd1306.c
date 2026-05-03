#include "ssd1306.h"

const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // (space) - 0x20
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0 - 0x30
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x00, 0x08, 0x14, 0x22, 0x41}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A - 0x41
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x00, 0x7F, 0x41, 0x41}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // backslash
    {0x41, 0x41, 0x7F, 0x00, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x38}, // a - 0x61
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x04, 0x04, 0x7E, 0x05, 0x05}, // f
    {0x08, 0x54, 0x54, 0x54, 0x3C}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x78, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x14, 0x7C}, // q
    {0x00, 0x7C, 0x08, 0x04, 0x04}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x04, 0x3F, 0x44, 0x44}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z - 0x7A
    {0x00, 0x08, 0x36, 0x41, 0x41}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x41, 0x41, 0x36, 0x08, 0x00}, // }
    {0x02, 0x01, 0x02, 0x04, 0x02}, // ~
    {0X7F, 0X7F, 0X7F, 0X7F, 0X7F}  // del
};

const uint8_t font5x7_RU[][5] = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // А (совпадает с лат. A)
    {0x7F, 0x49, 0x49, 0x49, 0x31}, // Б
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // В
    {0x7F, 0x01, 0x01, 0x01, 0x01}, // Г
    {0x70, 0x41, 0x41, 0x41, 0x7F}, // Д (упрощенная)
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // Е
    {0x77, 0x08, 0x7F, 0x08, 0x77}, // Ж
    {0x41, 0x49, 0x49, 0x49, 0x36}, // З
    {0x7F, 0x10, 0x08, 0x04, 0x7F}, // И
    {0x7F, 0x10, 0x09, 0x04, 0x7F}, // Й
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // К
    {0x40, 0x3E, 0x01, 0x01, 0x7F}, // Л
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // М
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // Н
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // О
    {0x7F, 0x01, 0x01, 0x01, 0x7F}, // П
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // Р
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // С
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // Т
    {0x27, 0x48, 0x48, 0x48, 0x3F}, // У
    {0x1C, 0x22, 0x7F, 0x22, 0x1C}, // Ф
    {0x63, 0x14, 0x08, 0x14, 0x63}, // Х
    {0x7F, 0x40, 0x40, 0x40, 0x7F}, // Ц
    {0x0F, 0x08, 0x08, 0x08, 0x7F}, // Ч
    {0x7F, 0x40, 0x7F, 0x40, 0x7F}, // Ш
    {0x7F, 0x40, 0x7F, 0x40, 0x7F}, // Щ (упрощенная)
    {0x01, 0x7F, 0x48, 0x48, 0x30}, // Ъ
    {0x7F, 0x48, 0x30, 0x00, 0x7F}, // Ы
    {0x7F, 0x48, 0x48, 0x30, 0x00}, // Ь
    {0x22, 0x41, 0x49, 0x49, 0x3E}, // Э
    {0x7F, 0x08, 0x3E, 0x41, 0x3E}, // Ю
    {0x46, 0x29, 0x19, 0x09, 0x7F}  // Я
};

/*
Clocking: Enable clocking for Port B and the I2C1 module, 
and assign the configured external clock source.
Pin Configuration: Alternate function, open drain.
Analog Filter: Enabled by default (ANFOFF = 0 in CR1), it removes noise shorter than 50ns. 
This is useful for OLEDs.
Timings (TIMINGR): Unlike older series (F1), this is a complex register. 
The value 0x00902025 is written to the register for a frequency of 400 kHz at a 48 MHz clock frequency. 
The value 0x10805E89 is for a frequency of 100 kHz at a 48 MHz clock frequency.
*/
void i2c1_oled_init(void)
{
    // 1. Clocking for GPIOB and I2C1
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->CFGR3 |= RCC_CFGR3_I2C1SW_SYSCLK; // I2C from 48МГц

    // 2. Configurate output PB6, PB7: Alternate Function, Open-Drain
    // Select alternate function AF1 for I2C1 on the STM32F0
    GPIOB->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7); // Bits reset
    GPIOB->MODER |= (GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1); // Bits set
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
    GPIOB->AFR[0] |= (1 << (6 * 4)) | (1 << (7 * 4)); // Alternate function AF1
    GPIOB->OTYPER |= (GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7); // 1: Output open-drain
    //GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR6_0 | GPIO_PUPDR_PUPDR7_0); // Pull-up

    // 3. Figure 203. I2C initialization flow pp.533 RM0360 - Reference manual
    I2C1->CR1 &= ~I2C_CR1_PE;
    
    // 4. Enable analog noise filter (ANFOFF=0)
    I2C1->CR1 &= ~I2C_CR1_ANFOFF;
    // Set the filter threshold (e.g., 4 I2C clock cycles).
    //I2C1->CR1 &= ~I2C_CR1_DNF; // Bits reset
    //I2C1->CR1 |= (4 << I2C_CR1_DNF_Pos);


    // 5. The set for NOSTRETCH
    // If need to DISABLE clock stretching:
    // I2C1->CR1 |= I2C_CR1_NOSTRETCH;
    
    // Or need for ENABLE (its recommended for Master):
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;

    // 6. Set the timings (for I2CCLK = 48 MHz)
    // The value for Fast Mode (400 kHz):
    // PRESC=0, SCLDEL=0x9, SDADEL=0x0, SCLH=0x20, SCLL=0x25
    // I2C1->TIMINGR = (uint32_t)0x00902025; 
    I2C1->TIMINGR = 0x10805E89; // 100kHz for 48MHz
    
    // 7. Enabling peripherals
    I2C1->CR1 |= I2C_CR1_PE;
}


int i2c1_oled_waittxis(void)
{
    uint32_t timeout = 100000;
    /*
    I2C_ISR_TXIS - The flag is raised when the TXDR register is empty and ready to receive a new byte.
    I2C_ISR_NACKF - If the display does not respond (e.g., a poor connection), the cycle is interrupted.
    */
    while (!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)) && --timeout);
    if (timeout == 0) return -1; // Error by timeout
	
	/*
    If a NACK is received, clear the flag (NACKCF) and generate a STOP to free the bus.
    Otherwise, the MCU will hang waiting for the next byte, which will not be received.
	*/
    if (I2C1->ISR & I2C_ISR_NACKF)
	{
        I2C1->ICR |= I2C_ICR_NACKCF;
        I2C1->CR2 |= I2C_CR2_STOP;
        return 0;
    }
    return 1;
}

/*
For the SSD1306, command is always a two-byte packet: Control byte (0x00) + Command byte.
*/
void oled_sendcommand(uint8_t cmd)
{
    I2C1->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RD_WRN); 
	// Set the display address. Specify that we will transfer 2 bytes. Generate a start condition.
    I2C1->CR2 |= (SSD1306_ADDRESS << 1) | (2 << I2C_CR2_NBYTES_Pos) | I2C_CR2_START | I2C_CR2_AUTOEND;

    // SSD1306_COMMAND_MODE (0x00) is sent first, then the command itself.
    if (I2C_OLED_WaitTXIS()) I2C1->TXDR = SSD1306_COMMAND_MODE;
    if (I2C_OLED_WaitTXIS()) I2C1->TXDR = cmd;
    
	/* At the end, wait for STOPF. 
    This is important: you cannot start a new transaction until 
    the previous one has physically completed on the line.
    */
    while (!(I2C1->ISR & I2C_ISR_STOPF));
    I2C1->ICR |= I2C_ICR_STOPCF;
}

/*
Chunking: The entire screen buffer is 1024 bytes.
The NBYTES register in the STM32 is only 8 bits in size,
meaning it can send a maximum of 255 bytes at a time.
*/
void oled_senddata(uint8_t* data, uint16_t size)
{
    uint16_t sent = 0;
    while (sent < size)
	{
	    /*
        We take 128 bytes of data + 1 mode byte (SSD1306_DATA_MODE = 0x40).
        The total chunk is 1 + 1 (maximum 129 bytes), which fits well within the hardware limits.
	    */
        uint8_t chunk = (size - sent > 128) ? 128 : (uint8_t)(size - sent);
        I2C1->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RD_WRN);
        I2C1->CR2 |= (SSD1306_ADDRESS << 1) | ((chunk + 1) << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
        
		/*
        The program opens an I2C session, sent 0x40, and sent 128 bytes from the buffer,
        closes the session (STOP), and 
        repeats this eight times until all 1024 bytes have been transferred. 
		*/
        if (I2C_OLED_WaitTXIS()) I2C1->TXDR = SSD1306_DATA_MODE;
        for (uint8_t i = 0; i < chunk; i++) {
            if (I2C_OLED_WaitTXIS()) I2C1->TXDR = data[sent++];
        }
		
		/*
        At the end, wait for STOPF. 
        This is important: you cannot start a new transaction until 
        the previous one has physically completed on the line.
        */
        while (!(I2C1->ISR & I2C_ISR_STOPF));
        I2C1->ICR |= I2C_ICR_STOPCF;
    }
}

/*
Initialization sequence for an OLED display with 
an SSD1306 controller (128x64 resolution).
  * First, the display is blanked (DISPLAYOFF) to ensure proper setup. 
    The refresh rate is set (SETDISPLAYCLOCKDIV).
  * SETMULTIPLEX with the parameter 0x3F (63) tells the controller to use all 64 lines.
  * The CHARGEPUMP and 0x14 commands (usually hidden behind PUMP_ON) enable 
    the internal voltage multiplier. Without this, the display simply won't light up from 3.3V.
  * SEGREMAP_127 and COMSCANDEC together rotate the image 180 degrees. 
    This is convenient if the display connector is on the top rather than the bottom.
  * ADDR_HORIZ enables horizontal addressing. This means that when you send data bytes, 
    the cursor automatically moves to the next line after the current line is 
    filled—perfect for quickly filling a frame. 
  * SETCONTRAST with a value of 0xCF sets a fairly high brightness.
*/
void oled_init(void)
{
    oled_sendcommand(SSD1306_DISPLAYOFF);
    
    oled_sendcommand(SSD1306_SETDISPLAYCLOCKDIV);
    oled_sendcommand(0x80);
    
    oled_sendcommand(SSD1306_SETMULTIPLEX);
    oled_sendcommand(0x3F); // For 128x64

    oled_sendcommand(SSD1306_SETDISPLAYOFFSET);
    oled_sendcommand(0x00);
    
    oled_sendcommand(SSD1306_SETSTARTLINE | 0x00);
    
    oled_sendcommand(SSD1306_CHARGEPUMP);
    oled_sendcommand(SSD1306_PUMP_ON);
    
    oled_sendcommand(SSD1306_MEMORYMODE);
    oled_sendcommand(SSD1306_ADDR_HORIZ);
    
    oled_sendcommand(SSD1306_SEGREMAP_127); // Flip horizontally
    oled_sendcommand(SSD1306_COMSCANDEC);   // Flip vertically
    
    oled_sendcommand(SSD1306_SETCOMPINS);
    oled_sendcommand(0x12);
    
    oled_sendcommand(SSD1306_SETCONTRAST);
    oled_sendcommand(0xCF);
    
    oled_sendcommand(SSD1306_SETPRECHARGE);
    oled_sendcommand(0xF1);
    
    oled_sendcommand(SSD1306_SETVCOMDETECT);
    oled_sendcommand(0x40);
    
    oled_sendcommand(SSD1306_DISPLAYALLON_RESUME);
    oled_sendcommand(SSD1306_NORMALDISPLAY);
    oled_sendcommand(SSD1306_DISPLAYON);
}

/*
Given the NBYTES (255 bytes) limitation we discussed earlier, 
the easiest way to clear is in cycles of 128 bytes (one page).
*/
void oled_clear(void)
{
    oled_setcursor(0, 0); // First, let's go back to the beginning
    uint8_t zero_page[128] = {0}; // Empty string
    
    // In Horizontal Addressing mode, the cursor will automatically move to a new line
    // We need to fill 8 lines (pages) of 128 pixels each
    for (uint8_t i = 0; i < 8; i++) {
        oled_senddata(zero_page, 128);
    }
}

/*
To write in a specific location on the screen, 
you need to send commands to set the "window" or position.
*/
void oled_setcursor(uint8_t column, uint8_t page)
{
    // Specify the column range (from column to 127)
    oled_sendcommand(SSD1306_COLUMNADDR);
    oled_sendcommand(column);
    oled_sendcommand(127);

    // Specify the range of pages (lines) (from page to 7)
    oled_sendcommand(SSD1306_PAGEADDR);
    oled_sendcommand(page);
    oled_sendcommand(7);
}

/*
Example array for letter 'A' (5 bytes) - 0x7E, 0x11, 0x11, 0x11, 0x7E
*/
void oled_drawchar(uint8_t *font_char)
{
    /* 
    Just send 5 data bytes to the display
    If Horizontal Mode is enabled, the cursor will move to the right
    */
    oled_senddata(font_char, 5);
    
    // Add the empty column (1 pixel) between letters for much better reading
    uint8_t space = 0x00;
    oled_senddata(&space, 1);
}

/*
Call OLED_Init().
Call OLED_Clear() to clear out any garbage.
Call OLED_SetCursor(0, 0), starting writing from the upper-left corner.
Call OLED_DrawChar(...) as many times as you need characters.
*/
void oled_putc(char c)
{
    if (c < 32 || c > 126) c = ' '; // Replace unknown characters with spaces
    
    /*
    Take 5 bytes from the font array for a specific character
    And send them to the display
    */
    oled_senddata((uint8_t*)font5x7[c - 32], 5);
    
    // Add 1 empty column (character spacing)
    uint8_t space = 0x00;
    oled_senddata(&space, 1);
}

void oled_puts(char* str)
{
    while (*str) {
        oled_putc(*str++);
    }
}

/*
int main(void)
{
    OLED_Init();
    OLED_Clear();

    OLED_SetCursor(0, 0); // Upper left corner
    OLED_PutS("Hello STM32!");
    
    OLED_SetCursor(0, 2); // Third line (page)
    OLED_PutS("Temp: 25.5 C");
}
*/