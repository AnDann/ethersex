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
#define EIWOMISA_DEBUG(a...)  debug_printf("eiwomisa: " a)
#else
#define EIWOMISA_DEBUG(a...)
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

  uint8_t fxoffset=0;
  uint8_t fxslot=EIWOMISA_FXSLOT;
  /* Init FX Slots */
#ifdef DMX_FX_RAINBOW
  fxslot[fxslot].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslot].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslot].devices = 1;
  fxslot[fxslot].margin = 0;
  fxslot[fxslot].effect = DMX_FXLIST_RAINBOW;
  fxoffset += 4;
  fxslot++;
#endif
#ifdef DMX_FX_RANDOM
  fxslot[fxslot].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslot].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslot].devices = 1;
  fxslot[fxslot].margin = 0;
  fxslot[fxslot].effect = DMX_FXLIST_RANDOM;
  fxoffset += 4;
  fxslot++;
#endif
#ifdef DMX_FX_FIRE
  fxslot[fxslot].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslot].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslot].devices = 1;
  fxslot[fxslot].margin = 0;
  fxslot[fxslot].effect = DMX_FXLIST_FIRE;
  fxoffset += 4;
  fxslot++;
#endif
#ifdef DMX_FX_WATER
  fxslot[fxslot].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslot].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslot].devices = 1;
  fxslot[fxslot].margin = 0;
  fxslot[fxslot].effect = DMX_FXLIST_WATER;
  fxoffset += 4;
  fxslot++;
#endif
#ifdef DMX_FX_RGB
  fxslot[fxslot].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslot].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslot].devices = 1;
  fxslot[fxslot].margin = 0;
  fxslot[fxslot].effect = DMX_FXLIST_RGB;
  fxoffset += 4;
  fxslot++;
#endif
  /* Setup DMX-Storage Connection */
  eiwomisa_dmx_conn_id = dmx_storage_connect(EIWOMISA_UNIVERSE);
  get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                                         EIWOMISA_UNIVERSE_OFFSET,
                                         eiwomisa_dmx_conn_id);
  EIWOMISA_DEBUG("Setup DMX id %i\n", eiwomisa_dmx_conn_id);
#endif
#ifndef TEENSY_SUPPORT
  eiwomisa_loadFromEEPROM();
#endif
}

void
eiwomisa_setProg(const e_programs newprog)
{
  if(newprog > WHITE || newprog == config.program)
    return;
  config.program = newprog;
}

e_programs
eiwomisa_getProg()
{
  return config.program;
}

uint16_t
eiwomisa_getProgSpeed()
{
  return fxslot[config.program].speed;
}

void
eiwomisa_setProgSpeed(uint16_t newspeed)
{
  fxslot[config.program].speed = newspeed;
}

e_whitedim
eiwomisa_getWhiteStatus()
{
  return config.whitedim[config.program];
}

uint8_t
eiwomisa_getProgActive()
{
  return fxslot[config.program].active;
}

void
eiwomisa_doAction(const e_actions action)
{
  uint8_t newprog = config.program;
  static uint8_t saveWhite;
#ifdef EIWOMISA_TTY_SUPPORT
  eiwomisa_tty_refresh();
#endif
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
      eiwomisa_setProg(newprog);
    case PROGSPEED_UP:
      if (fxslot[config.program].speed < 1000)
        fxslot[config.program].speed++;
      break;
    case PROGSPEED_DOWN:
      if (fxslot[config.program].speed > 0)
        fxslot[config.program].speed--;
      break;
    case PROG_PLAYPAUSE:
      fxslot[config.program].active = fxslot[config.program].active ? 0 : 1;
      break;
    case WHITE_UP:
      if(config.whitedim[config.program])
        config.whitedim[config.program] = config.whitedim[config.program] == ON ? UP : ON;
      break;
    case WHITE_DOWN:
      if(config.whitedim[config.program])
        config.whitedim[config.program] = config.whitedim[config.program] == ON ? DOWN : ON;
      break;
    case WHITE_TOGGLE:
      config.whitedim[config.program] = config.whitedim[config.program] ? ON : OFF;
      break;
    default:
      break;
  }
}

#ifndef TEENSY_SUPPORT
void
eiwomisa_loadFromEEPROM(void)
{
  eeprom_restore(eiwomisa_config, &config, sizeof(eiwomisa_config_t));
#ifdef EIWOMISA_DMX_SUPPORT
  dmx_fxslot_restore();
#endif
}

void
eiwomisa_storeToEEPROM(void)
{
  eeprom_save(eiwomisa_config, &config, sizeof(eiwomisa_config_t));
#ifdef EIWOMISA_DMX_SUPPORT
  dmx_fxslot_save();
#endif
  eeprom_update_chksum();
}
#endif

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
        for (uint8_t i = 0; i < LED_ALL; i++)
        {
          if(i == LED_W && config.whiteactive[config.program] == 0)
            eiwomisa_setpwm(i, 0);
          else
            eiwomisa_setpwmfade(i,
                          get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                                               (EIWOMISA_UNIVERSE_OFFSET + (config.program * 4)) + i,
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
          if(i == LED_W && config.whiteactive[config.program] == 0)
            eiwomisa_setpwm(i, 0);
          else
            eiwomisa_setpwm(i,
                          get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                                               (EIWOMISA_UNIVERSE_OFFSET + (config.program * 4)) + i,
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

void
eiwomisa_whitedim()
{
  if(config.whiteactive[config.program] && config.whitedim)
  {
    uint8_t channelVal = get_dmx_channel_raw(EIWOMISA_UNIVERSE, (EIWOMISA_UNIVERSE_OFFSET + (config.program * 4)) + LED_W);
    if(config.whitedim == UP)
      if(channelVal < 255)
        set_dmx_channel(EIWOMISA_UNIVERSE, (EIWOMISA_UNIVERSE_OFFSET + (config.program * 4)) + LED_W, channelVal + 1);
      else
        config.whitedim = HOLD;
    if(config.whitedim == DOWN)
      if(channelVal > 0)
        set_dmx_channel(EIWOMISA_UNIVERSE, (EIWOMISA_UNIVERSE_OFFSET + (config.program * 4)) + LED_W, channelVal - 1);
      else
        config.whitedim = HOLD;
  }
}


/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa.h)
  startup(eiwomisa_init)
  millitimer(40,eiwomisa_whitedim)
  mainloop(eiwomisa_periodic)
*/
