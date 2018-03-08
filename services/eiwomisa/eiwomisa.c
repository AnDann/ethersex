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
#include "core/util/fixedpoint.h"

#include "config.h"

#include "eiwomisa_pwm.h"
#include "eiwomisa_mqtt.h"

#ifdef DEBUG_EIWOMISA
#include "core/debug.h"
#define EIWOMISA_DEBUG(s, ...) debug_printf("[eiwomisa] " s "\n", ## __VA_ARGS__)
#else
#define EIWOMISA_DEBUG(...)  do { } while(0)
#endif

#ifdef EIWOMISA_DMX_SUPPORT
#include "services/dmx-storage/dmx_storage.h"
#include "services/dmx-fxslot/dmx-fxslot.h"
#include "services/clock/clock.h"
static uint8_t eiwomisa_dmx_conn_id;
static timestamp_t last_dmx_sync;
#endif

static eiwomisa_config_t config;

void
eiwomisa_init()
{
#ifndef TEENSY_SUPPORT
  eiwomisa_loadFromEEPROM();
#endif
#ifdef EIWOMISA_DMX_SUPPORT
  /* Init FX Slots */
  fxslot[EIWOMISA_FXSLOT].universe = EIWOMISA_UNIVERSE;
  fxslot[EIWOMISA_FXSLOT].startchannel = EIWOMISA_UNIVERSE_OFFSET;
  fxslot[EIWOMISA_FXSLOT].devices = 1;
  fxslot[EIWOMISA_FXSLOT].margin = 0;
  /* Setup DMX-Storage Connection */
  eiwomisa_dmx_conn_id = dmx_storage_connect(EIWOMISA_UNIVERSE);
  get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                       EIWOMISA_UNIVERSE_OFFSET, eiwomisa_dmx_conn_id);
  EIWOMISA_DEBUG("Setup DMX id %i", eiwomisa_dmx_conn_id);
#endif
}

void
eiwomisa_setProg(const e_programs newprog)
{
  if (newprog >= COUNT_PROGRAMS)
    return;
#ifdef EIWOMISA_DMX_SUPPORT
  if (newprog < DMX_RECEIVER)
  {
    fxslot[EIWOMISA_FXSLOT].active = 1;
    fxslot[EIWOMISA_FXSLOT].effect = newprog + 1;
    dmx_fxslot_init(EIWOMISA_FXSLOT);
  }
  else
  {
    fxslot[EIWOMISA_FXSLOT].active = 0;
    fxslot[EIWOMISA_FXSLOT].effect = 0;
  }
#endif
  config.program = newprog;
#ifdef EIWOMISA_TTY_SUPPORT
  eiwomisa_tty_refresh();
#endif
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
  return config.progspeed[config.program];
}

void
eiwomisa_setProgSpeed(uint16_t newspeed)
{
  config.progspeed[config.program] = newspeed;
}

uint8_t
eiwomisa_getProgActive()
{
  return fxslot[EIWOMISA_FXSLOT].active;
}

uint8_t
eiwomisa_getProgDimmer()
{
  return get_dmx_universe_dimmer(EIWOMISA_UNIVERSE);
}

void
eiwomisa_setProgDimmer(const uint8_t newdimmer)
{
  set_dmx_universe_dimmer(EIWOMISA_UNIVERSE, newdimmer);
}
#endif

e_whitedim
eiwomisa_getWhiteStatus()
{
  return config.whitedim[config.program];
}

uint8_t
eiwomisa_getWhite()
{
  return config.white_values[config.program];
}

void
eiwomisa_setWhite(uint8_t newWhite)
{
  if(newWhite) config.whitedim[config.program] = ON;
  config.white_values[config.program] = newWhite;
}

uint32_t eiwomisa_getWhiteRGB()
{
  return (uint32_t)config.white_rgb_values[LED_R] << 16 | (uint32_t)config.white_rgb_values[LED_G] << 8 | (uint32_t)config.white_rgb_values[LED_B];
}

void eiwomisa_setWhiteRGB(const int16_t R, const int16_t G, const int16_t B)
{
  if(R>=0)
    config.white_rgb_values[LED_R] = R;
  if(G>=0)
    config.white_rgb_values[LED_G] = G;
  if(B>=0)
    config.white_rgb_values[LED_B] = B;
}

void
eiwomisa_doAction(const e_actions action)
{
  EIWOMISA_DEBUG("Action=%u", action);
  if (action == NONE)
    return;
  uint8_t newprog = config.program;
  uint8_t newvalue;
#ifdef EIWOMISA_TTY_SUPPORT
  eiwomisa_tty_refresh();
#endif
  switch (action)
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
      break;
#ifdef EIWOMISA_DMX_SUPPORT
    case PROGSPEED_UP:
      if (config.progspeed[config.program] < 255)
        config.progspeed[config.program]++;
      break;
    case PROGSPEED_DOWN:
      if (config.progspeed[config.program] > 0)
        config.progspeed[config.program]--;
      break;
    case PROGDIM_UP:
      newvalue=get_dmx_universe_dimmer(EIWOMISA_UNIVERSE);
      if (newvalue < 255)
        set_dmx_universe_dimmer(EIWOMISA_UNIVERSE, ++newvalue);
      break;
    case PROGDIM_DOWN:
      newvalue=get_dmx_universe_dimmer(EIWOMISA_UNIVERSE);
      if (newvalue > 0)
        set_dmx_universe_dimmer(EIWOMISA_UNIVERSE, --newvalue);
      break;
    case PROG_PLAYPAUSE:
      fxslot[EIWOMISA_FXSLOT].active =
        (fxslot[EIWOMISA_FXSLOT].active == 0) ? 1 : 0;
      break;
#endif
    case WHITE_UP:
      if (config.whitedim[config.program])
        config.whitedim[config.program] =
          (config.whitedim[config.program] == ON) ? UP : ON;
      break;
    case WHITE_DOWN:
      if (config.whitedim[config.program])
        config.whitedim[config.program] =
          (config.whitedim[config.program] == ON) ? DOWN : ON;
      break;
    case WHITE_TOGGLE:
      config.whitedim[config.program] =
        (config.whitedim[config.program] == OFF) ? ON : OFF;
      break;
    default:
      break;
  }
#ifdef EIWOMISA_MQTT_SUPPORT
  char *data = malloc(4);
  utoa(action, data, 10);
  push(data, &mqtt_action_queue);
#endif
}

#ifndef TEENSY_SUPPORT
void
eiwomisa_loadFromEEPROM(void)
{
  eeprom_restore(eiwomisa_config, &config, sizeof(eiwomisa_config_t));
#ifdef EIWOMISA_DMX_SUPPORT
  dmx_fxslot_restore();
#endif
  eiwomisa_setProg(config.program);
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
  switch (config.program)
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
                                                   EIWOMISA_UNIVERSE_OFFSET +
                                                   i, eiwomisa_dmx_conn_id));
        }
      }
      break;
    case DMX_RECEIVER:
    case AMBILIGHT:
      if (get_dmx_slot_state(EIWOMISA_UNIVERSE, eiwomisa_dmx_conn_id) ==
          DMX_NEWVALUES)
      {
        last_dmx_sync = clock_get_time();
        for (uint8_t i = 0; i < LED_ALL; i++)
        {
          if (i != LED_W || config.program == DMX_RECEIVER)
            eiwomisa_setpwm(i,
                          get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                                               EIWOMISA_UNIVERSE_OFFSET + i,
                                               eiwomisa_dmx_conn_id));
        }
      }
      else if (EIWOMISA_DMX_TIMEOUT
               && (clock_get_time() - last_dmx_sync) > EIWOMISA_DMX_TIMEOUT)
      {
        last_dmx_sync = clock_get_time();
        EIWOMISA_DEBUG("DMX Timeout!");
        for (uint8_t i = 0; i < LED_ALL; i++)
        {
          eiwomisa_setpwm(i, 0);
        }
      }
      break;
#endif
    case WHITE:
      for (uint8_t i = 0; i < LED_W; i++)
      {
        if (eiwomisa_getpwmfade(i) != config.white_rgb_values[i])
          eiwomisa_setpwmfade(i, config.white_rgb_values[i]);
      }
      break;
    default:
      break;
  }
  
  //Dont update white channel if dmx is activated
  if (config.program == DMX_RECEIVER)
    return;
  
  //Update white channel
  if (config.whitedim[config.program] == OFF)
  {
    if (eiwomisa_getpwmfade(LED_W))
      eiwomisa_setpwmfade(LED_W, 0);
  }
  else if (eiwomisa_getpwmfade(LED_W) != config.white_values[config.program])
    eiwomisa_setpwmfade(LED_W, config.white_values[config.program]);

  //Update progspeed
  if (fxslot[EIWOMISA_FXSLOT].speed != config.progspeed[config.program])
    fxslot[EIWOMISA_FXSLOT].speed = config.progspeed[config.program];
}

void
eiwomisa_whitedim()
{
  if (config.whitedim[config.program])
  {
    if (config.whitedim[config.program] == UP)
    {
      if (config.white_values[config.program] < 255)
        config.white_values[config.program]++;
      else
      {
        config.whitedim[config.program] = ON;
#ifdef EIWOMISA_TTY_SUPPORT
        eiwomisa_tty_refresh();
#endif
      }
    }
    if (config.whitedim[config.program] == DOWN)
    {
      if (config.white_values[config.program] > 0)
        config.white_values[config.program]--;
      else
      {
        config.whitedim[config.program] = ON;
#ifdef EIWOMISA_TTY_SUPPORT
        eiwomisa_tty_refresh();
#endif
      }
    }
  }
}


/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa.h)
  startup(eiwomisa_init)
  millitimer(20,eiwomisa_whitedim)
  mainloop(eiwomisa_periodic)
*/
