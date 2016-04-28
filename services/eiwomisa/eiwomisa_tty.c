/*
 * Copyright (c) 2016 by Daniel Lindner <daniel.lindner@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "config.h"

#include "eiwomisa.h"
#include "eiwomisa_tty.h"
#include "core/tty/tty.h"

#ifdef EIWOMISA_HD44780_BACKLIGHT
#include "hardware/lcd/hd44780.h"
static uint16_t blcounter;
#endif

WINDOW *wmain, *wvalue;
static uint8_t refresh;

static const char str1[] PROGMEM = "Rainbow";
static const char str2[] PROGMEM = "Random";
static const char str3[] PROGMEM = "Fire";
static const char str4[] PROGMEM = "Water";
static const char str5[] PROGMEM = "RGB";
static const char str6[] PROGMEM = "DMX-Receiver";
static const char str7[] PROGMEM = "Ambilight";
static const char str8[] PROGMEM = "White";

static const char * const program_names[] PROGMEM = {str1,str2,str3,str4,str5,str6,str7,str8};

#ifdef EIWOMISA_HD44780_BACKLIGHT
void eiwomisa_backlight_periodic()
{
  if(blcounter)
  {
    if(--blcounter == 0)
    hd44780_backlight(0);
  }
}
#endif


void eiwomisa_tty_refresh()
{
#ifdef EIWOMISA_HD44780_BACKLIGHT
  hd44780_backlight(1);
  blcounter = EIWOMISA_BACKLIGHT_TIMEOUT;
#endif
  refresh=1;
}


void eiwomisa_tty_init()
{
#ifdef EIWOMISA_HD44780_BACKLIGHT
  //Switch on backlight
  eiwomisa_tty_refresh();
#endif
  initscr();
  wmain = subwin(curscr, 1, 16,0 ,0);
  wvalue = subwin(curscr, 1, 16,1 ,0);
}

void eiwomisa_tty_periodic()
{
  if(refresh)
  {
    wclear(wmain);
    waddstr_P(wmain, (const char*)pgm_read_word(&(program_names[eiwomisa_getProg()])));
  }
  wclear(wvalue);
  wprintw_P(wvalue, PSTR("%u %u %u %u"), eiwomisa_getpwmfade(LED_R), eiwomisa_getpwmfade(LED_G), eiwomisa_getpwmfade(LED_B), eiwomisa_getpwmfade(LED_W));
  refresh = 0;
}

/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa_tty.h)
  init(eiwomisa_tty_init)
  millitimer(1000, eiwomisa_backlight_periodic)
  millitimer(100, eiwomisa_tty_periodic)
*/
