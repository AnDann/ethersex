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

#include "config.h"

#include "hardware/ir/irmp/irmp.h"
#include "eiwomisa.h"
#include "eiwomisa_irmp.h"

#ifdef DEBUG_EIWOMISA_IRMP
#include "core/debug.h"
#define EIWOMISA_IRMP_DEBUG(a...)  debug_printf("[irmp] " a)
#else
#define EIWOMISA_IRMP_DEBUG(a...)
#endif

#define EIWOMISA_IR_PROTOCOL  IRMP_PROTO_RC5
#define EIWOMISA_IR_ADDRESS   0

void
eiwomisa_irmp_periodic()
{
  irmp_data_t *irmp_data;

  if ((irmp_data = irmp_read()))
  {
    EIWOMISA_IRMP_DEBUG("Protocol=%u Address=%u Command=%u Flags=%u\n",
                        irmp_data->protocol, irmp_data->address,
                        irmp_data->command, irmp_data->flags);
    if (irmp_data->protocol == EIWOMISA_IR_PROTOCOL &&
        irmp_data->address == EIWOMISA_IR_ADDRESS)
    {
      e_actions action = NONE;

      // Not repeated commands
      if (!irmp_data->flags)
      {
        switch (irmp_data->command)
        {
          case 1 ... 12:
            eiwomisa_setProg(irmp_data->command - 1);
            break;
          case 17:
            action = SAVE;
            break;
          case 18:
            action = LOAD;
            break;
          case 23:
            action = WHITE_UP;
            break;
          case 24:
            action = WHITE_DOWN;
            break;
          case 14:
            action = PROG_DOWN;
            break;
          case 15:
            action = PROG_UP;
            break;
          case 38:
            action = WHITE_TOGGLE;
            break;
          case 34:
          case 39:
            action = PROG_PLAYPAUSE;
            break;
        }
        if (action != NONE)
        {
          eiwomisa_doAction(action);
          return;
        }
      }
      switch (irmp_data->command)
      {
        case 25:
          action = MENU_UP;
          break;
        case 26:
          action = MENU_RIGHT;
          break;
        case 27:
          action = MENU_DOWN;
          break;
        case 28:
          action = MENU_LEFT;
          break;
        case 29:
          action = MENU_OK;
          break;
        case 13:
          action = MENU_HOME;
          break;
        case 20:
          action = PROGDIM_UP;
          break;
        case 19:
          action = PROGDIM_DOWN;
          break;
        case 30:
          action = PROGSPEED_UP;
          break;
        case 31:
          action = PROGSPEED_DOWN;
          break;
      }
      eiwomisa_doAction(action);
      return;
    }
  }
}


/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa_irmp.h)
  millitimer(20, eiwomisa_irmp_periodic)
*/
