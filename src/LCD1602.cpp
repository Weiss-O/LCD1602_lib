#include "LCD1602.h"

// Constructor for 4-bit mode (assume rw wired to GND)
LCD1602::LCD1602(uint8_t rs, uint8_t enable,
                 uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
  : _rs(rs), _rw(255), _enable(enable), _displayFunction(0x00),
    _entryMode(0x00){
  _dataPins[4] = d4;
  _dataPins[5] = d5;
  _dataPins[6] = d6;
  _dataPins[7] = d7;
  _displayFunction |= 4_BIT_MODE_FLAG; // Set DL (data lines) to 4-bit mode
  _displayFunction |= 2_ROW_FLAG; // Assume 2-row display
  _displayFunction |= FONT_5x10_FLAG; // Use 5x10 font

  _entryMode |= ENTRY_RIGHT | ENTRY_SHIFT; // Default entry mode: cursor moves right
}


void LCD1602::begin(uint8_t cols, uint8_t rows) {
  _cols = cols; _rows = rows;
  pinMode(_rs, OUTPUT);
  if (_rw != 255) pinMode(_rw, OUTPUT);
  pinMode(_enable, OUTPUT);
  for (int i = 0; i < 8; i++)
    if (_dataPins[i]) pinMode(_dataPins[i], OUTPUT);
  // Pull enable low to start
  digitalWrite(_enable, LOW);

  // initialization sequence
  //Delay time > 15ms after power up
  delay(20);
  // Need to send (Instruction Set)
  // RS | RW | D7 | D6 | D5 | D4
  // 0  | 0  | 0  | 0  | 1  | 1 
  if(_displayFunction & 8_BIT_MODE_FLAG) {
    //TODO: Write the 8-bit initialization sequence
  } else {
    // 4-bit mode initialization
    init4bit();
  }
  // Set the display mode (display lines and character font)
  send(WRITE_DISPLAY_SETTINGS | _displayFunction, 0);
  //Display off
  send(DISPLAY_OFF, 0);
  //clear display
  send(CLEAR_DISPLAY, 0);
  //Entry mode set
  send(WRITE_ENTRY_MODE_SETTINGS | _entryMode, 0);
}

void LCD1602::init4bit() {
  //Now we send the command during init, we are in 8-bit mode so 1 pulse
  write4bits(INIT_COMMAND >> 4, 0); // Send upper nibble
  delay(5); // Wait > 4.1ms
  write4bits(INIT_COMMAND >> 4, 0); // Send 0x30 again
  delay(1); // Wait > 100 us
  write4bits(INIT_COMMAND >> 4, 0); // Send 0x30 again
  delay(1); // This delay may not be necessary, none is specified in the datasheet
  write4bits((WRITE_DISPLAY_SETTINGS & ~8_BIT_MODE_FLAG) >> 4, 0); // Send 0x20 to set 4-bit mode
}

void LCD1602::clear() {
  send(0x01, 0);
  delayMicroseconds(2000);
}

//Cursor Return function on datasheet
void LCD1602::home() {
  send(0x02, 0);
  delayMicroseconds(2000);
}

void LCD1602::setCursor(uint8_t col, uint8_t row) {
  static uint8_t rowOffsets[] = { 0x00, 0x40, 0x14, 0x54 };
  send(0x80 | (col + rowOffsets[row]), 0);
}

//When RS is set to 1, writes from DDRAM or CGRAM.
//for upper nibble 0000 it will read custom characters from CGRAM
//for upper nibble 0010 and higher it will write to DDRAM
// I think that upper nibble 0001 is undefined?
size_t LCD1602::write(uint8_t c) {
  uint8_t value = mapCharToLCD(c);
  send(value, 1);
  return 1;
}

void LCD1602::print(const char *s) {
  while (*s != NULL) write(*s++);
}

//Follows the order of operations for the write protocol
void LCD1602::send(uint8_t value, uint8_t mode) {
  digitalWrite(_rs, mode);
  if (_rw != 255) digitalWrite(_rw, LOW);

  if (_displayFunction & 0x10) {
    // 4-bit mode: high nibble, then low nibble
    write4bits(value >> 4); //right-shift four to get higher nibble
    write4bits(value & 0x0F); // & with 0x0F to remove high nibble
  } else {
    // 8-bit mode: all eight bits at once
    for (int i = 0; i < 8; i++){
      digitalWrite(_dataPins[i], (value >> i) & 0x01);
    }
    pulseEnable();
  }
  //Delay for command cycle time
  delayMicroseconds(COMMAND_E_CYCLE);
}

void LCD1602::write4bits(uint8_t nibble) {
  for (int i = 0; i < 4; i++) {
    //Right shift based on current bit then mask with 0x01
    digitalWrite(_dataPins[i+4], (nibble >> i) & 0x01); 
  }
  pulseEnable();
}

void LCD1602::pulseEnable() {
  //Since a digitalWrite takes 3.4us >> t_pw = 0.45us, we don't need any delays
  digitalWrite(_enable, LOW);
  digitalWrite(_enable, HIGH);
  digitalWrite(_enable, LOW);
}
