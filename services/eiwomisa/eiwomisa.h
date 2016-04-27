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

typedef enum e_leds
{ LED_R=0, LED_G, LED_B, LED_W, LED_ALL } e_leds;

typedef enum e_actions
{ 
  SAVE,
  LOAD, 
  PROG_UP, 
  PROG_DOWN, 
  PROGSPEED_UP, 
  PROGSPEED_DOWN, 
  WHITE_UP, 
  WHITE_DOWN, 
  WHITE_TOOGLE, 
  NONE 
} e_actions;

typedef enum e_programs
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
} e_programs;

typedef struct eiwomisa_config_t
{
  uint8_t values[LED_ALL];
  e_programs program;
} eiwomisa_config_t;

void eiwomisa_init();
void eiwomisa_periodic();
void eiwomisa_doAction(const e_actions action);

int16_t parse_cmd_eiwomisa_prog_speed(char *cmd, char *output, const uint16_t len);
int16_t parse_cmd_eiwomisa_prog(char *cmd, char *output, const uint16_t len);

#ifndef TEENSY_SUPPORT
void eiwomisa_loadFromEEPROM(void);
void eiwomisa_storeToEEPROM(void);
int16_t parse_cmd_eiwomisa_save(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_eiwomisa_load(char *cmd, char *output, uint16_t len);
#endif

#endif /* HAVE_EIWOMISA_H */
