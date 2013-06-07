/**
 * @file CosaHD44780.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2013, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * @section Description
 * Demonstration of the HD44780 (aka 1602) device driver with mapping 
 * to IOStream::Device.
 * 
 * @section Circuit
 * Use default pin configuration (Arduino/Standard, Mighty, ATtiny ==> LCD); 
 * D4 ==> D4, D5 ==> D5, D6 ==> D6, D7 ==> D7, D8 ==> RS, and D9 ==> EN.
 * For Arduino/Mega: D8 ==> RS, D9 ==> EN, D10 ==> D4, D11 => D5, 
 * D12 ==> D6, and D13 ==> D7.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Types.h"
#include "Cosa/Trace.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/IOStream/Driver/HD44780.hh"

HD44780 lcd;
#undef putchar

const uint8_t bitmaps[] PROGMEM = {
  // Bar(1)
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  0b10000,
  // Bar(2)
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  // Bar(3)
  0b11100,
  0b11100,
  0b11100,
  0b11100,
  0b11100,
  0b11100,
  0b11100,
  0b11100,
  // Bar(4)
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  // Bar(5)
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
};

void setup()
{
  // Initiate Watchdog for low power delay
  Watchdog::begin();

  // Initate LCD and load custom character bitmaps
  lcd.begin();
  lcd.set_tab_step(2);
  lcd.cursor_underline_off();
  lcd.cursor_blink_off();
  uint8_t n = membersof(bitmaps)/8;
  for (char c = 0; c < n; c++) 
    lcd.set_custom_char_P(c, &bitmaps[c*8]);

  // Bind LCD to trace output and print the custom characters
  trace.begin(&lcd, PSTR("\fCosaHD44780"));
  for (uint8_t i = 0; i < lcd.WIDTH; i++) {
    uint8_t x, y;
    lcd.get_cursor(x, y);
    for (char c = 0; c < n; c++) {
      lcd.set_cursor(x, y);
      trace << c;
      Watchdog::delay(64);
    }
  }
  trace << clear;

  // Simple scrolling text
  static const char msg[] PROGMEM = 
    "The quick brown fox jumps over the lazy dog. ";
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t len = strlen_P(msg);
    for (uint8_t j = 0; j < len; j++) {
      trace << (char) pgm_read_byte(msg + j);
      Watchdog::delay(64);
    }
  }
  SLEEP(2);
}

void loop()
{
  // Step through the LCD font. Use tab steps between characters
  static char c = 0;
  trace << c++ << '\t';
  Watchdog::delay(64);
}