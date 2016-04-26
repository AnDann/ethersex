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

#ifndef HAVE_EIWOMISA_H
#define HAVE_EIWOMISA_H

typedef enum 
{ LED_R=0, LED_G, LED_B, LED_W, LED_ALL } e_leds;

typedef enum
{ SAVE, LOAD, PROG_UP, PROG_DOWN, WHITE_UP, WHITE_DOWN, WHITE_TOOGLE } e_actions;

typedef enum
{ 
#ifdef EIWOMISA_DMX_SUPPORT
#ifdef DMX_FX_RAINBOW
  RAINBOW,
#endif
#ifdef DMX_FX_RANDOM
  RANDOM,
#endif
#ifdef DMX_FX_FIRE
  FIRE,
#endif
#ifdef DMX_FX_WATER
  WATER,
#endif
#ifdef DMX_FX_RGB
  RGB,
#endif
  DMX_RECEIVER,
  AMBILIGHT,
#endif
  WHITE
} e_program;

typedef struct eiwomisa_config_t
{
  uint8_t values[LED_ALL];
  e_program program;
} eiwomisa_config_t;

void eiwomisa_init();
void eiwomisa_periodic();
void eiwomisa_doAction(const e_actions action);

#ifndef TEENSY_SUPPORT
void eiwomisa_loadFromEEPROM(void);
void eiwomisa_storeToEEPROM(void);
#endif

#endif /* HAVE_EIWOMISA_H */
