/**
 * @file Cosa/IOStream.cpp
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2012-2013, Mikael Patel
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * @section Description
 * Basic in-/output stream support class. Requires implementation of
 * Stream::Device. See UART for an example.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/IOStream.hh"
#include <stdarg.h>

IOStream::Device IOStream::Device::null;

int 
IOStream::Device::putchar(char c) 
{ 
  return (-1); 
}
    
int 
IOStream::Device::puts(char* s) 
{ 
  char c;
  while ((c = *s++) != 0) 
    if (putchar(c) < 0) return (-1);
  return (0); 
}

int 
IOStream::Device::puts_P(const char* s)
{ 
  char c;
  while ((c = pgm_read_byte(s++)) != 0)
    if (putchar(c) < 0) return (-1);
  return (0); 
}

int 
IOStream::Device::write(void* buf, size_t size) 
{ 
  char* ptr = (char*) buf;
  size_t n = 0;
  for(; n < size; n++)
    if (putchar(*ptr++) < 0)
      return (-1);
  return (n);
}

int 
IOStream::Device::getchar() 
{ 
  return (-1); 
}

char* 
IOStream::Device::gets(char *s, size_t count) 
{ 
  char* res = s;
  while (count--) {
    int c = getchar();
    if (c < 0) {
      *s = 0;
      return (0);
    }
    if (c == '\n') {
      *s = 0;
      return (res);
    }
    *s++ = c;
  }
  *--s = 0;
  return (res);
}

int 
IOStream::Device::read(void* buf, size_t size) 
{ 
  char* ptr = (char*) buf;
  size_t n = 0; 
  for (; n < size; n++)
    if ((*ptr++ = getchar()) < 0)
      return (-1);
  return (n);
}

int 
IOStream::Device::flush() 
{ 
  return (-1); 
}

void 
IOStream::print(int n, uint8_t base) 
{
  if (base != 10) print_prefix(base);
  char buf[sizeof(int) * CHARBITS + 1];
  print(itoa(n, buf, base));
}

void 
IOStream::print(long int n, uint8_t base)
{
  if (base != 10) print_prefix(base);
  char buf[sizeof(long int) * CHARBITS + 1];
  print(ltoa(n, buf, base));
}

void 
IOStream::print(unsigned int n, uint8_t base) 
{
  if (base != 10) print_prefix(base);
  char buf[sizeof(int) * CHARBITS + 1];
  print(utoa(n, buf, base));
}

void 
IOStream::print(unsigned long int n, uint8_t base)
{
  if (base != 10) print_prefix(base);
  char buf[sizeof(long int) * CHARBITS + 1];
  print(ultoa(n, buf, base));
}

void 
IOStream::print_prefix(uint8_t base)
{
  if (base == 2)
    print_P(PSTR("0b"));
  else if (base == 8)
    print_P(PSTR("0"));
  else if (base == 16)
    print_P(PSTR("0x"));
}

void 
IOStream::print(void *ptr, size_t size, uint8_t base, uint8_t max)
{
  uint8_t* p = (uint8_t*) ptr;
  unsigned int v_adj = (base == 10 ? 0 : (base == 8 ? 01000 : 0x100));
  uint8_t adj = (v_adj != 0);
  uint8_t n = 0;
  while (size--) {
    if (n == 0) {
      print(p);
      print_P(PSTR(": "));
    }
    char buf[sizeof(int) * CHARBITS + 1];
    unsigned int v = (*p++) + v_adj;
    print(utoa(v, buf, base) + adj);
    if (++n < max) {
      print_P(PSTR(" "));
    }
    else {
      println();
      n = 0;
    }
  }
  if (n != 0) println();
}

void 
IOStream::vprintf_P(const char* format, va_list args)
{
  const char* s = format;
  uint8_t is_signed;
  uint8_t base;
  char c;
  while ((c = pgm_read_byte(s++)) != 0) {
    if (c == '%') {
      is_signed = 1;
      base = 10;
    next:
      c = pgm_read_byte(s++);
      if (c == 0) s--;
      switch (c) {
      case 'b': 
	base = 2; 
	goto next;
      case 'o': 
	base = 8; 
	goto next;
      case 'h':
      case 'x': 
	base = 16; 
	goto next;
      case 'u': 
	is_signed = 0; 
	goto next;
      case 'c': 
	print((char) va_arg(args, int)); 
	continue;
      case 'p': 
	print(va_arg(args, void*)); 
	continue;
      case 's': 
	print(va_arg(args, char*)); 
	continue;
      case 'S': 
	print_P(va_arg(args, const char*)); 
	continue;
      case 'd': 
	if (is_signed) 
	  print(va_arg(args, int), base); 
	else
	  print(va_arg(args, unsigned int), base); 
	continue;
      case 'l': 
	if (is_signed) 
	  print(va_arg(args, long int), base);
	else
	  print(va_arg(args, unsigned long int), base);
	continue;
      };
    }
    print(c);
  }
}

IOStream& 
endl(IOStream& outs)
{
  outs.print('\n');
  return (outs);
}

IOStream& 
bin(IOStream& outs)
{
  outs.m_base = 2;
  return (outs);
}

IOStream& 
oct(IOStream& outs)
{
  outs.m_base = 8;
  return (outs);
}

IOStream& 
dec(IOStream& outs)
{
  outs.m_base = 10;
  return (outs);
}

IOStream& 
hex(IOStream& outs)
{
  outs.m_base = 16;
  return (outs);
}

