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
#include "core/eeprom.h"

#include "config.h"

#include "eiwomisa_pwm.h"

#ifdef DEBUG_EIWOMISA
#include "core/debug.h"
#define EIWOMISADEBUG(a...)  debug_printf("eiwomisa: " a)
#else
#define EIWOMISADEBUG(a...)
#endif

#ifdef EIWOMISA_DMX_SUPPORT
#include "services/dmx-storage/dmx_storage.h"
#include "services/dmx-fxslot/dmx-fxslot.h"
uint8_t eiwomisa_dmx_conn_id;
#endif

static eiwomisa_config_t config; 

void
eiwomisa_init()
{
#ifdef EIWOMISA_DMX_SUPPORT
  /* Init FX Slot */
  fxslot[EIWOMISA_FXSLOT].universe = EIWOMISA_UNIVERSE;
  fxslot[EIWOMISA_FXSLOT].startchannel = EIWOMISA_UNIVERSE_OFFSET;
  fxslot[EIWOMISA_FXSLOT].devices = 1;
  fxslot[EIWOMISA_FXSLOT].margin = 0;
  /* Setup DMX-Storage Connection */
  eiwomisa_dmx_conn_id = dmx_storage_connect(EIWOMISA_UNIVERSE);
  get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                                         EIWOMISA_UNIVERSE_OFFSET,
                                         eiwomisa_dmx_conn_id);
  EIWOMISADEBUG("Setup DMX id %i\n", eiwomisa_dmx_conn_id);
#endif
#ifndef TEENSY_SUPPORT
  eiwomisa_loadFromEEPROM();
#endif
}

void
eiwomisa_periodic()
{
  switch(config.program)
  {
#ifdef EIWOMISA_DMX_SUPPORT
    case RAINBOW:
    case RANDOM:
    case FIRE:
    case WATER:
    case RGB:
      if (get_dmx_slot_state(EIWOMISA_UNIVERSE, eiwomisa_dmx_conn_id) ==
          DMX_NEWVALUES)
      {
        for (uint8_t i = 0; i < LED_W; i++)
        {
          eiwomisa_setpwmfade(i,
                          get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                                               EIWOMISA_UNIVERSE_OFFSET + i,
                                               eiwomisa_dmx_conn_id));
        }
      }
      break;
    case DMX_RECEIVER:
    case AMBILIGHT:
      if (get_dmx_slot_state(EIWOMISA_UNIVERSE, eiwomisa_dmx_conn_id) ==
          DMX_NEWVALUES)
      {
        for (uint8_t i = 0; i < LED_ALL; i++)
        {
          eiwomisa_setpwm(i,
                          get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                                               EIWOMISA_UNIVERSE_OFFSET + i,
                                               eiwomisa_dmx_conn_id));
        }
      }
      break;
#endif
    case WHITE:
      break;
    default:
      break;
  }
}

static void 
eiwomisa_changeProg(const e_programs newprog)
{
  if(newprog > WHITE || newprog == config.program)
    return;
  
  switch(newprog)
  {
#ifdef EIWOMISA_DMX_SUPPORT
    case RAINBOW:
    case RANDOM:
    case FIRE:
    case WATER:
    case RGB:
      fxslot[EIWOMISA_FXSLOT].active = 1;
      fxslot[EIWOMISA_FXSLOT].effect = newprog;
      dmx_fxslot_init(EIWOMISA_FXSLOT);
      break;
    case DMX_RECEIVER:
    case AMBILIGHT:
      fxslot[EIWOMISA_FXSLOT].active = 0;
      break;
#endif
    case WHITE:
      break;
    default:
      break;
  }
  config.program = newprog;
}

void 
eiwomisa_doAction(const e_actions action)
{
  uint8_t newprog = config.program;
  switch(action)
  {
#ifndef TEENSY_SUPPORT
    case SAVE:
      eiwomisa_storeToEEPROM();
      break;
    case LOAD:
      eiwomisa_loadFromEEPROM();
      break;
#endif
    case PROG_UP:
      newprog += 2;
    case PROG_DOWN:
      newprog--;
      eiwomisa_changeProg(newprog);
    case PROGSPEED_UP:
      fxslot[EIWOMISA_FXSLOT].speed++;
      break;
    case PROGSPEED_DOWN:
      fxslot[EIWOMISA_FXSLOT].speed--;
      break;
    default:
      break;
  }
  if (config.program > WHITE)
    config.program = WHITE;
}

#ifndef TEENSY_SUPPORT
void
eiwomisa_loadFromEEPROM(void)
{
  eiwomisa_config_t temp;
  eeprom_restore(eiwomisa_config, &temp, sizeof(eiwomisa_config_t));
  EIWOMISADEBUG("Load eeprom %i,%i,%i,%i\n", temp.values[LED_R],temp.values[LED_G],temp.values[LED_B],temp.values[LED_W]);
  eiwomisa_setpwmfade(LED_R, temp.values[LED_R]);
  eiwomisa_setpwmfade(LED_G, temp.values[LED_G]);
  eiwomisa_setpwmfade(LED_B, temp.values[LED_B]);
  eiwomisa_setpwmfade(LED_W, temp.values[LED_W]);
  eiwomisa_changeProg(temp.program);
}

void
eiwomisa_storeToEEPROM(void)
{
  config.values[LED_R] = eiwomisa_getpwmfade(LED_R);
  config.values[LED_G] = eiwomisa_getpwmfade(LED_G);
  config.values[LED_B] = eiwomisa_getpwmfade(LED_B);
  config.values[LED_W] = eiwomisa_getpwmfade(LED_W);
  eeprom_save(eiwomisa_config, &config, sizeof(eiwomisa_config_t));
  eeprom_update_chksum();
}
#endif


/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa.h)
  init(eiwomisa_init)
  mainloop(eiwomisa_periodic)
*/
