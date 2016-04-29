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

#define ARROW_UP 0
#define ARROW_DOWN 1

#ifdef EIWOMISA_HD44780_BACKLIGHT
#include "hardware/lcd/hd44780.h"
#endif

#ifdef EIWOMISA_STELLA_BACKLIGHT
#include "services/stella/stella.h"
#endif

WINDOW *wprog, *wwhite, *wstatus, *wrgb;
static uint8_t refresh;

#ifdef EIWOMISA_DMX_SUPPORT
static const char str1[] PROGMEM = "Rainbow";
static const char str2[] PROGMEM = "Random";
static const char str3[] PROGMEM = "Fire";
static const char str4[] PROGMEM = "Water";
static const char str5[] PROGMEM = "RGB";
static const char str6[] PROGMEM = "DMX-Receiver";
static const char str7[] PROGMEM = "Ambilight";
#endif
static const char str8[] PROGMEM = "White";

static const char * const program_names[] PROGMEM = {
#ifdef EIWOMISA_DMX_SUPPORT
  str1,
  str2,
  str3,
  str4,
  str5,
  str6,
  str7,
#endif
  str8
  };

#if defined (EIWOMISA_HD44780_BACKLIGHT) || defined (EIWOMISA_STELLA_BACKLIGHT)
static uint16_t blcounter;

void eiwomisa_backlight_periodic()
{
  if(blcounter)
  {
    if(--blcounter == 0)
    {
#ifdef EIWOMISA_HD44780_BACKLIGHT
    hd44780_backlight(0);
#endif
#ifdef EIWOMISA_STELLA_BACKLIGHT
    stella_setValue(STELLA_SET_FADE, EIWOMISA_STELLA_CHANNEL, 0);
#endif
    }
  }
}
#endif /* EIWOMISA_HD44780_BACKLIGHT or EIWOMISA_STELLA_BACKLIGHT */


void eiwomisa_tty_refresh()
{
#if defined (EIWOMISA_HD44780_BACKLIGHT) || defined (EIWOMISA_STELLA_BACKLIGHT)
#ifdef EIWOMISA_HD44780_BACKLIGHT
  hd44780_backlight(1);
#endif
#ifdef EIWOMISA_STELLA_BACKLIGHT
  stella_setValue(STELLA_SET_FADE, EIWOMISA_STELLA_CHANNEL, 255);
#endif
  blcounter = EIWOMISA_BACKLIGHT_TIMEOUT;
#endif /* EIWOMISA_HD44780_BACKLIGHT or EIWOMISA_STELLA_BACKLIGHT */
  refresh=1;
}


void eiwomisa_tty_init()
{
  initscr();
  wprog = subwin(curscr, 1, 12,0 ,0);
  wwhite = subwin(curscr, 1, 2,0 ,14);
  wstatus = subwin(curscr, 1, 2,0 ,12);
  wrgb = subwin(curscr, 1, 16,1 ,0);
 
#ifdef HD44780_SUPPORT
  uint8_t arrow_up[] = {0x4,0xa,0x1f,0x0,0x0,0x0,0x0,0x0};
  uint8_t arrow_down[] = {0x0,0x0,0x0,0x0,0x1f,0xa,0x4,0x0};
  hd44780_define_char(ARROW_UP, arrow_up,1);
  hd44780_define_char(ARROW_DOWN, arrow_down,1);
#ifdef HD44780_MULTIENSUPPORT
  hd44780_define_char(ARROW_UP, arrow_up,2);
  hd44780_define_char(ARROW_DOWN, arrow_down,2);
#endif
#endif  /*  HD44780_SUPPORT */

  eiwomisa_tty_refresh();
}

void eiwomisa_tty_periodic()
{
  if(refresh)
  {
    wclear(wprog);
    waddstr_P(wprog, (const char*)pgm_read_word(&(program_names[eiwomisa_getProg()])));
    wclear(wwhite);
    uint8_t whitestatus = eiwomisa_getWhiteStatus();
    if(whitestatus)
      waddch(wwhite, 'W');
    switch(whitestatus)
    {
      case UP:
        waddch(wwhite, ARROW_UP);
        break;
      case DOWN:
        waddch(wwhite, ARROW_DOWN);
        break;
    }
  }
  wclear(wrgb);
  wprintw_P(wrgb, PSTR("%3u %3u %3u %3u"), eiwomisa_getpwmfade(LED_R), eiwomisa_getpwmfade(LED_G), eiwomisa_getpwmfade(LED_B), eiwomisa_getpwmfade(LED_W));
  refresh = 0;
}

/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa_tty.h)
  init(eiwomisa_tty_init)
  millitimer(1000, eiwomisa_backlight_periodic)
  millitimer(100, eiwomisa_tty_periodic)
*/
