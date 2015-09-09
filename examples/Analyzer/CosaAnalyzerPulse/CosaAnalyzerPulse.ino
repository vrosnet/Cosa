/**
 * @file CosaAnalyzerPulse.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2015, Mikael Patel
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
 * Logic Analyzer based analysis of Job Scheduler; generate 160 ms
 * pulse width [10..90%]. Demonstrate scheduling relative to scheduler
 * dispatch timestamp (expire_period).
 *
 * @section Circuit
 * Trigger on CHAN0/D8 rising.
 *
 * +-------+
 * | CHAN0 |-------------------------------> D8
 * | CHAN1 |-------------------------------> D9
 * | CHAN2 |-------------------------------> D10
 * |       |
 * | GND   |-------------------------------> GND
 * +-------+
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Job.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/OutputPin.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"

// Use the RTC or Watchdog Job Scheduler
#define USE_RTC
// #define USE_WATCHDOG

// Call directly from interrupt service routine removing the event
// handler overhead but also making time update and other interrupt
// vunerable to drops
#define USE_ISR_DISPATCH

// Use dispatch timestamp as reference. The pulses will be kept in sync
// otherwise they will drift towards a 1 ms spread. Note that sync
// will also compensate for the event handler overhead
#define expire_after expire_period

#if defined(USE_RTC)
#define TIMER RTC
#define SCALE(x) (x) * 1000UL
#endif

#if defined(USE_WATCHDOG)
#define TIMER Watchdog
#define SCALE(x) (x)
#endif

class Pulse : public Job {
public:
  static const uint32_t START = SCALE(1000);
  static const uint32_t WIDTH = SCALE(160);

  Pulse(Job::Scheduler* scheduler, uint16_t percent,
	Board::DigitalPin pin) :
    Job(scheduler),
    m_pin(pin)
  {
    expire_at(START);
    width(percent);
  }

#if defined(USE_ISR_DISPATCH)
  virtual void on_expired()
  {
    run();
  }
#endif

  virtual void run()
  {
    m_pin.toggle();
    if (m_pin.is_set())
      expire_after(m_pulse);
    else
      expire_after(WIDTH - m_pulse);
    start();
  }

  void width(uint16_t percent)
  {
    if (percent < 10) percent = 10;
    else if (percent > 90) percent = 90;
    m_pulse = (WIDTH * percent) / 100UL;
  }

private:
  OutputPin m_pin;
  uint32_t m_pulse;
};

// The job scheduler
TIMER::Scheduler scheduler;

// Pulse generators with 20, 50 and 20% pulse width (32, 80, 32 ms)
Pulse p1(&scheduler, 20, Board::D8);
Pulse p2(&scheduler, 50, Board::D9);
Pulse p3(&scheduler, 20, Board::D10);

void setup()
{
  // Print Info about the logic analyser probe channels
  uart.begin(9600);
  trace.begin(&uart, PSTR("CosaAnalyzerPulse: started"));
  trace << PSTR("CHAN0 - D8 [^]") << endl;
  trace << PSTR("CHAN1 - D9") << endl;
  trace << PSTR("CHAN2 - D10") << endl;
  trace.flush();

  // Start the real-time clock and scheduler
  TIMER::begin();
  TIMER::job(&scheduler);

  // Start the pulse width generators
  p1.start();
  p2.start();
  p3.start();
}

void loop()
{
  Event::service();
}

