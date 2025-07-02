//Library for the LCD1602 display in 4-bit mode
//Writing this was largely assisted by AI tools, so generated code may be influenced by others.
//It was written to be compatible with the Arduino IDE and was done as a learning exercise.
//Current implementation was based off of the datasheet https://www.waveshare.com/datasheet/LCD_en_PDF/LCD1602.pdf

#include "LCD1602.h"

// Constructor for 4-bit mode (assume rw wired to GND)
LCD1602::LCD1602(uint8_t rs, uint8_t enable,
                 uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
                 bool debug = false)
  : _rs(rs), _rw(255), _enable(enable), _displayFunction(0x00),
    _entryMode(0x00), _debug(debug){
  // Initialize all data pins to zero
  for (int i = 0; i < 8; i++) _dataPins[i] = 0;
  _dataPins[4] = d4;
  _dataPins[5] = d5;
  _dataPins[6] = d6;
  _dataPins[7] = d7;
  _displayFunction |= BIT_MODE_FLAG_4; // Set DL (data lines) to 4-bit mode
  _displayFunction |= ROW_FLAG_2; // Assume 2-row display
  _displayFunction &= ~FONT_5x11_FLAG; // Use 5x8 font

  _entryMode |= ENTRY_RIGHT; // Default entry mode: cursor moves right
}


void LCD1602::begin(uint8_t cols, uint8_t rows) {
  _cols = cols; _rows = rows;
  pinMode(_rs, OUTPUT);
  if (_rw != 255) pinMode(_rw, OUTPUT);
  pinMode(_enable, OUTPUT);
  for (int i = 0; i < 8; i++)
    if (_dataPins[i]) pinMode(_dataPins[i], OUTPUT); //FIXME: This will break if using GPIO pin 0
  // Pull enable low to start
  digitalWrite(_enable, LOW);

  // initialization sequence
  //Delay time > 15ms after power up
  delay(20);
  // Need to send (Instruction Set)
  // RS | RW | D7 | D6 | D5 | D4
  // 0  | 0  | 0  | 0  | 1  | 1 
  if(_displayFunction & BIT_MODE_FLAG_8) {
    //TODO: Write the 8-bit initialization sequence
  } else {
    // 4-bit mode initialization
    init4bit();
  }
  // Set the display mode (display lines and character font)
  // if(_debug) {
  //   Serial.print("Setting display function: ");
  //   Serial.println(WRITE_DISPLAY_SETTINGS | _displayFunction, BIN);
  // }
  send(WRITE_DISPLAY_SETTINGS | _displayFunction, 0);
  delayMicroseconds(COMMAND_E_CYCLE);
  //Display off
  turnOff();
  //clear display
  // if(_debug) {
  //   Serial.print("Clearing display: ");
  //   Serial.println(CLEAR_DISPLAY, BIN);
  // }
  send(CLEAR_DISPLAY, 0);
  delayMicroseconds(COMMAND_E_CYCLE);
  //Entry mode set
  // if(_debug) {
  //   Serial.print("Setting entry mode: ");
  //   Serial.println(WRITE_ENTRY_MODE_SETTINGS | _entryMode, BIN);
  // }
  send(WRITE_ENTRY_MODE_SETTINGS | _entryMode, 0);
  delayMicroseconds(COMMAND_E_CYCLE);
}

void LCD1602::init4bit() {
  //Now we send the command during init, we are in 8-bit mode so 1 pulse
  if (_debug) {
    Serial.print("Initializing LCD in 4-bit mode... ");
    Serial.println(INIT_COMMAND >> 4, BIN);
  } 
  write4bits(INIT_COMMAND >> 4); // Send upper nibble
  delay(5); // Wait > 4.1ms
  write4bits(INIT_COMMAND >> 4); // Send 0x30 again
    if (_debug) {
    Serial.print("Initializing LCD in 4-bit mode... ");
    Serial.println(INIT_COMMAND >> 4, BIN);
  } 
  delayMicroseconds(100); // Wait > 100 us
  write4bits(INIT_COMMAND >> 4); // Send 0x30 again
    if (_debug) {
    Serial.print("Initializing LCD in 4-bit mode... ");
    Serial.println(INIT_COMMAND >> 4, BIN);
  } 
  delayMicroseconds(COMMAND_E_CYCLE); // This delay may not be necessary, none is specified in the datasheet
  // if(_debug) {
  //   Serial.print("Setting LCD to 4-bit mode... ");
  //   Serial.println((WRITE_DISPLAY_SETTINGS & ~BIT_MODE_FLAG_8) >> 4, BIN);
  // }
  write4bits((WRITE_DISPLAY_SETTINGS & ~BIT_MODE_FLAG_8) >> 4); // Send 0x20 to set 4-bit mode
  delayMicroseconds(COMMAND_E_CYCLE);
}

void LCD1602::clear() {
  send(CLEAR_DISPLAY, 0);
  delayMicroseconds(COMMAND_E_CYCLE);
}

//Cursor Return function on datasheet
void LCD1602::home() {
  send(0x02, 0);
  delayMicroseconds(COMMAND_E_CYCLE);
}

void LCD1602::setCursor(uint8_t col, uint8_t row) {
  static uint8_t rowOffsets[] = { 0x00, 0x28};
  if (row >= 2) row = 1; // Clamp to max row index
  send(0x80 | (col + rowOffsets[row]), 0);
}

//When RS is set to 1, writes from DDRAM or CGRAM.
//for upper nibble 0000 it will read custom characters from CGRAM
//for upper nibble 0010 and higher it will write to DDRAM
// I think that upper nibble 0001 is undefined?
size_t LCD1602::write(uint8_t c) {
  uint8_t value = mapCharToLCD(c);
  // if(_debug){
  //   Serial.print("Sending to LCD: ");
  //   Serial.print(value, BIN);  // print binary format
  //   Serial.print(" (0x");
  //   Serial.print(value, HEX);  // print hex format too
  //   Serial.print(") ");
  //   Serial.println((char)value); // print character representation
  // }
  send(value, 1);
  return 1;
}

void LCD1602::print(const char *s, uint8_t row, uint8_t col) {
  while (*s && row < _rows) {
    int startCol = (row == 0) ? col : 0;
    int spaceRemaining = _cols - startCol;
    int charsToPrint = 0;
    int lastSpaceIdx = -1;

    // Measure how many characters fit on this line
    while (s[charsToPrint] && charsToPrint < spaceRemaining) {
      if (s[charsToPrint] == ' ') {
        lastSpaceIdx = charsToPrint;
      }
      charsToPrint++;
    }

    // Wrap at last space if possible
    if (charsToPrint == spaceRemaining && lastSpaceIdx != -1) {
      charsToPrint = lastSpaceIdx;
    }

    // Print the chunk
    setCursor(startCol, row);
    for (int i = 0; i < charsToPrint; i++) {
      write(*s++);
    }

    // Skip trailing space (if any)
    if (*s == ' ') s++;

    row++;
  }
}


//Follows the order of operations for the write protocol
void LCD1602::send(uint8_t value, uint8_t mode) {
  digitalWrite(_rs, mode);
  if (_rw != 255) digitalWrite(_rw, LOW);

  if (_displayFunction & 0x10) {
    // 8-bit mode: all eight bits at once
    for (int i = 0; i < 8; i++){
      digitalWrite(_dataPins[i], (value >> i) & 0x01);
    }
    pulseEnable();
  } else {
    // 4-bit mode: high nibble, then low nibble
    write4bits(value >> 4); //right-shift four to get higher nibble
    write4bits(value & 0x0F); // & with 0x0F to remove high nibble
  }
  //Delay for command cycle time
  delayMicroseconds(COMMAND_E_CYCLE);
}

void LCD1602::turnOff() {
  // if(_debug) {
  //   Serial.print("Turning display off: ");
  //   Serial.println(DISPLAY_OFF, BIN);
  // }
  send(DISPLAY_OFF, 0);
  delayMicroseconds(COMMAND_E_CYCLE); // Wait for the display to turn off
}

// Turn on the display
void LCD1602::turnOn() {
  // if(_debug) {
  //   Serial.print("Turning display on: ");
  //   Serial.println(DISPLAY_ON, BIN);
  // }
  send(DISPLAY_ON, 0); // Turn on the display
  delayMicroseconds(COMMAND_E_CYCLE); // Wait for the display to turn on
}

// Writes a 4-bit nibble to the LCD, mapping bits 0-3 to D4-D7 (not D0-D3)
void LCD1602::write4bits(uint8_t nibble) {
  for (int i = 0; i < 4; i++) {
    // Right shift based on current bit then mask with 0x01; maps to D4-D7
    digitalWrite(_dataPins[i+4], (nibble >> i) & 0x01); 
  }
  pulseEnable();
}

void LCD1602::pulseEnable() {
  // According to the LCD1602 datasheet, the minimum enable pulse width (t_pw) is 450ns.
  // Since digitalWrite takes approximately 3.4us, this satisfies the datasheet timing requirements.
  digitalWrite(_enable, LOW);
  digitalWrite(_enable, HIGH);
  digitalWrite(_enable, LOW);
}

inline uint8_t mapCharToLCD(uint8_t c) {
	if (c < FIRST_CHAR_ASCII || c > LAST_CHAR_ASCII) {
		return 0xFF; // Return 0xFF for characters outside the LCD character set (displays full block)
	}
	return c; // Map character to LCD character set
}