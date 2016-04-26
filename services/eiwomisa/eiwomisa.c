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

#include "eiwomisa.h"
#include "eiwomisa_pwm.h"

#ifdef EIWOMISA_DMX_SUPPORT
#include "services/dmx-storage/dmx_storage.h"
uint8_t eiwomisa_dmx_conn_id;
#endif

void
eiwomisa_init()
{
#ifdef EIWOMISA_DMX_SUPPORT
  /* Setup DMX-Storage Connection */
  eiwomisa_dmx_conn_id = dmx_storage_connect(EIWOMISA_UNIVERSE);
  get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                                         EIWOMISA_UNIVERSE_OFFSET,
                                         eiwomisa_dmx_conn_id);
#endif
#ifndef TEENSY_SUPPORT
  eiwomisa_loadFromEEPROM();
#endif
}

void
eiwomisa_periodic()
{
#ifdef EIWOMISA_DMX_SUPPORT
  if (get_dmx_slot_state(EIWOMISA_UNIVERSE, eiwomisa_dmx_conn_id) ==
      DMX_NEWVALUES)
  {
    for (uint8_t i = 0; i < LED_ALL; i++)
    {
      eiwomisa_setpwmfade(i,
                      get_dmx_channel_slot(EIWOMISA_UNIVERSE,
                                           EIWOMISA_UNIVERSE_OFFSET + i,
                                           eiwomisa_dmx_conn_id));
    }
  }
#endif
}

void 
eiwomisa_doAction(const e_actions action)
{
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
    default:
  }
}

#ifndef TEENSY_SUPPORT
void
eiwomisa_loadFromEEPROM(void)
{
  uint8_t values[LED_ALL];
  eeprom_restore(eiwomisa_channel_values, values, LED_ALL);
  eiwomisa_setpwmfade(LED_R, values[LED_ALL]);
  eiwomisa_setpwmfade(LED_G, values[LED_ALL]);
  eiwomisa_setpwmfade(LED_B, values[LED_ALL]);
  eiwomisa_setpwmfade(LED_W, values[LED_ALL]);
}

void
eiwomisa_storeToEEPROM(void)
{
  uint8_t values[LED_ALL] = {eiwomisa_getpwmfade[LED_R], eiwomisa_getpwmfade[LED_G], eiwomisa_getpwmfade[LED_B], eiwomisa_getpwmfade[LED_W]};
  eeprom_save(eiwomisa_channel_values, values, LED_ALL);
  eeprom_update_chksum();
}
#endif

/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa.h)
  init(eiwomisa_init)
  mainloop(eiwomisa_periodic())
*/
