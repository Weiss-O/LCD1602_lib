#ifndef LCD1602_H
#define LCD1602_H

#include <Arduino.h>

//Flags for display function
const uint8_t BIT_MODE_FLAG_8 = 0x10; // 8-bit mode flag
const uint8_t BIT_MODE_FLAG_4 = 0x00;
const uint8_t INIT_COMMAND = 0x30; // Initial command for LCD1602
const uint8_t ROW_FLAG_2 = 0x08; // 2-row display flag
const uint8_t FONT_5x11_FLAG = 0x04; // 5x11 font flag (0x00 for 5x8)

//Flags for entry mode
const uint8_t ENTRY_RIGHT = 0x02; // Entry mode: cursor moves right
const uint8_t ENTRY_SHIFT = 0x01; // Entry mode: display shifts to the right

//Commands
const uint8_t WRITE_DISPLAY_SETTINGS = 0x20; // Command to set display settings
const uint8_t DISPLAY_OFF = 0x08; // Command to turn off display
const uint8_t DISPLAY_ON = 0x0F; // Command to turn on display
const uint8_t CLEAR_DISPLAY = 0x01; // Command to clear display
const uint8_t WRITE_ENTRY_MODE_SETTINGS = 0x04; // Command to set entry mode

// Map for ascii characters.
const uint8_t FIRST_CHAR_ASCII = 0x20; // First character in the LCD character set
const uint8_t LAST_CHAR_ASCII = 0x7F; // Last character in the LCD character set
inline uint8_t mapCharToLCD(uint8_t c);

//Other constants
const uint16_t COMMAND_E_CYCLE = 1700; // Command cycle time in microseconds

class LCD1602 {
public:
	//constructor for 4-bit mode:
	LCD1602(uint8_t rs, uint8_t enable,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
		bool debug = false);
	
	void begin(uint8_t cols = 16, uint8_t rows = 2);
	void clear();
	void home();
	void setCursor(uint8_t col, uint8_t row);
	size_t write(uint8_t);	//for Print compatibility
	void print(const char *s, uint8_t row = 0, uint8_t col = 0);
	void turnOff();
	void turnOn();
private:
	void init4bit();
	void send(uint8_t value, uint8_t mode);
	void write4bits(uint8_t nibble);
	void pulseEnable();

	uint8_t _rs, _rw, _enable;
	uint8_t _dataPins[8];
	uint8_t _displayFunction, _entryMode;
	uint8_t _cols, _rows;
	bool _debug;
};

#endif
