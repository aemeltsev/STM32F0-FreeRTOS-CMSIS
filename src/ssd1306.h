#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f0xx.h"  // For access to data type

/* Common parameters */
#define SSD1306_ADDRESS             0x3C    // Standart adress (0x3C or 0x3D)
#define SSD1306_COMMAND_MODE        0x00    // Control Byte: next message is command
#define SSD1306_DATA_MODE           0x40    // Control Byte: next message is data from RAM

/* The commands for control power and display */
#define SSD1306_DISPLAYOFF          0xAE    // Turn off screen
#define SSD1306_DISPLAYON           0xAF    // Turn on screen
#define SSD1306_CHARGEPUMP          0x8D    // Charge pump control
#define SSD1306_PUMP_ON             0x14    // Turn on pump (3.3V)
#define SSD1306_PUMP_OFF            0x10    // Turn off the pump (external power supply)

/* Configure addressing */
#define SSD1306_MEMORYMODE          0x20    // Seting address mode
#define SSD1306_ADDR_HORIZ          0x00    // Horisontal mode (auto-line break)
#define SSD1306_ADDR_VERT           0x01    // Vertical mode
#define SSD1306_ADDR_PAGE           0x02    // Pages mode (by default)

#define SSD1306_COLUMNADDR          0x21    // Set range of column (0-127)
#define SSD1306_PAGEADDR            0x22    // Set range of pages (0-7)

/* Hardware Configuration */
#define SSD1306_SETSTARTLINE        0x40    // Beginner line (0x40 - 0x7F)
#define SSD1306_SETCONTRAST         0x81    // Brightness (for it byte 0–255)
#define SSD1306_SEGREMAP_0          0xA0    // Segments 0 to 127
#define SSD1306_SEGREMAP_127        0xA1    // Horizontal rotation (mirror)
#define SSD1306_DISPLAYALLON_RESUME 0xA4    // Output from RAM
#define SSD1306_DISPLAYALLON        0xA5    // Ignore RAM (all pixels are lit)
#define SSD1306_NORMALDISPLAY       0xA6    // Normal mode
#define SSD1306_INVERTDISPLAY       0xA7    // Inversion (black -> white)

#define SSD1306_SETMULTIPLEX        0xA8    // Multiplexer (screen height - 1)
#define SSD1306_COMSCANINC          0xC0    // Scanning from bottom to top
#define SSD1306_COMSCANDEC          0xC8    // Scanning from top to bottom (vertical rotation)
#define SSD1306_SETDISPLAYOFFSET    0xD3    // Vertical offset (followed by 0x00-0x3F)
#define SSD1306_SETCOMPINS          0xDA    // COM pin configuration (depends on height)

/* Timings and frequency */
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5    // Refresh rate
#define SSD1306_SETPRECHARGE        0xD9    // Precharge phases
#define SSD1306_SETVCOMDETECT       0xDB    // VCOMH level

// Font 5x7 (every symbol - 5 vertical byte)
extern const uint8_t font5x7[][5];
extern const uint8_t font5x7_RU[][5];

void i2c1_oled_init(void);
int i2c1_oled_waittxis(void);
void oled_sendcommand(uint8_t cmd);
void oled_senddata(uint8_t* data, uint16_t size);
void oled_init(void);
void oled_clear(void);
void oled_setcursor(uint8_t column, uint8_t page);
void oled_drawchar(uint8_t *font_char);
void oled_putc(char c);
void oled_puts(char* str);

#endif // SSD1306_H