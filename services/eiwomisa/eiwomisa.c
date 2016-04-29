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
  uint8_t fxslotnum=EIWOMISA_FXSLOT;
  /* Init FX Slots */
#ifdef DMX_FX_RAINBOW
  fxslot[fxslotnum].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslotnum].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslotnum].devices = 1;
  fxslot[fxslotnum].margin = 0;
  fxslot[fxslotnum].active = 0;
  fxslot[fxslotnum].effect = DMX_FXLIST_RAINBOW;
  fxoffset += 4;
  fxslotnum++;
#endif
#ifdef DMX_FX_RANDOM
  fxslot[fxslotnum].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslotnum].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslotnum].devices = 1;
  fxslot[fxslotnum].margin = 0;
  fxslot[fxslotnum].active = 0;
  fxslot[fxslotnum].effect = DMX_FXLIST_RANDOM;
  fxoffset += 4;
  fxslotnum++;
#endif
#ifdef DMX_FX_FIRE
  fxslot[fxslotnum].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslotnum].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslotnum].devices = 1;
  fxslot[fxslotnum].margin = 0;
  fxslot[fxslotnum].active = 0;
  fxslot[fxslotnum].effect = DMX_FXLIST_FIRESIMULATION;
  fxoffset += 4;
  fxslotnum++;
#endif
#ifdef DMX_FX_WATER
  fxslot[fxslotnum].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslotnum].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslotnum].devices = 1;
  fxslot[fxslotnum].margin = 0;
  fxslot[fxslotnum].active = 0;
  fxslot[fxslotnum].effect = DMX_FXLIST_WATERSIMULATION;
  fxoffset += 4;
  fxslotnum++;
#endif
#ifdef DMX_FX_RGB
  fxslot[fxslotnum].universe = EIWOMISA_UNIVERSE;
  fxslot[fxslotnum].startchannel = EIWOMISA_UNIVERSE_OFFSET + fxoffset;
  fxslot[fxslotnum].devices = 1;
  fxslot[fxslotnum].margin = 0;
  fxslot[fxslotnum].active = 0;
  fxslot[fxslotnum].effect = DMX_FXLIST_RGB;
  fxoffset += 4;
  fxslotnum++;
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
  if(newprog >= COUNT_PROGRAMS || newprog == config.program)
    return;
#ifdef EIWOMISA_DMX_SUPPORT
  fxslot[config.program].active = 0;
  if(newprog<=RGB)
    fxslot[newprog].active = 1;
#endif
  config.program = newprog;
}

e_programs
eiwomisa_getProg()
{
  return config.program;
}

#ifdef EIWOMISA_DMX_SUPPORT
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

uint8_t
eiwomisa_getProgActive()
{
  return fxslot[config.program].active;
}
#endif

e_whitedim
eiwomisa_getWhiteStatus()
{
  return config.whitedim[config.program];
}


void
eiwomisa_doAction(const e_actions action)
{
  if(action==NONE)
    return;
  uint8_t newprog = config.program;
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
#ifdef EIWOMISA_DMX_SUPPORT
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
#endif
    case WHITE_UP:
      if(config.whitedim[config.program])
        config.whitedim[config.program] = (config.whitedim[config.program] == ON) ? UP : ON;
      break;
    case WHITE_DOWN:
      if(config.whitedim[config.program])
        config.whitedim[config.program] = (config.whitedim[config.program] == ON) ? DOWN : ON;
      break;
    case WHITE_TOGGLE:
      config.whitedim[config.program] = (config.whitedim[config.program] == OFF) ? ON : OFF;
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
//  dmx_fxslot_restore();
#endif
}

void
eiwomisa_storeToEEPROM(void)
{
  eeprom_save(eiwomisa_config, &config, sizeof(eiwomisa_config_t));
#ifdef EIWOMISA_DMX_SUPPORT
//  dmx_fxslot_save();
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
          if((i == LED_W) && (config.whitedim[config.program] == OFF))
            eiwomisa_setpwmfade(i, 0);
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
          if((i == LED_W) && (config.whitedim[config.program] == OFF))
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
        for (uint8_t i = 0; i < LED_ALL; i++)
        {
          if((i == LED_W) && (config.whitedim[config.program] == OFF))
          {
            eiwomisa_setpwmfade(i, 0);
          }
          else
          if(eiwomisa_getpwmfade(i)!=config.white_values[i])
            eiwomisa_setpwmfade(i, config.white_values[i]);
        }
      break;
    default:
      break;
  }
}

void
eiwomisa_whitedim()
{
  if(config.whitedim[config.program])
  {
#ifdef EIWOMISA_DMX_SUPPORT
    uint8_t channelVal = get_dmx_channel_raw(EIWOMISA_UNIVERSE, (EIWOMISA_UNIVERSE_OFFSET + (config.program * 4)) + LED_W);
    if(config.whitedim[config.program] == UP)
    {
      if(channelVal < 255)
        set_dmx_channel(EIWOMISA_UNIVERSE, (EIWOMISA_UNIVERSE_OFFSET + (config.program * 4)) + LED_W, channelVal + 1);
      else
        config.whitedim[config.program] = ON;
    }
    if(config.whitedim[config.program] == DOWN)
    {  
      if(channelVal > 0)
        set_dmx_channel(EIWOMISA_UNIVERSE, (EIWOMISA_UNIVERSE_OFFSET + (config.program * 4)) + LED_W, channelVal - 1);
      else
        config.whitedim[config.program] = ON;
    }
#else
    if(config.whitedim[config.program] == UP)
    {
      if(config.white_values[LED_W] < 255)
        config.white_values[LED_W]++;
      else
        config.whitedim[config.program] = ON;
    }
    if(config.whitedim[config.program] == DOWN)
    {  
      if(config.white_values[LED_W] > 0)
        config.white_values[LED_W]--;
      else
        config.whitedim[config.program] = ON;
    }
#endif
  }
}


/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa.h)
  startup(eiwomisa_init)
  millitimer(20,eiwomisa_whitedim)
  mainloop(eiwomisa_periodic)
*/
