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

#include "hardware/input/buttons/buttons.h"
#include "eiwomisa.h"
#include "eiwomisa_button.h"

void
eiwomisa_button_init()
{
  hook_buttons_input_register(eiwomisa_button_handler);
}

void
eiwomisa_button_handler(buttons_ButtonsType button, uint8_t status)
{
  if(status == BUTTON_RELEASE)
    return;
  e_actions action;
  switch(button)
  {
    case BTN_UP:
      action = WHITE_UP;
      break;
    case BTN_DOWN:
      action = WHITE_DOWN;
      break;
    case BTN_LEFT:
      action = PROG_DOWN;
      break;
    case BTN_RIGHT:
      action = PROG_UP;
      break;
    case BTN_FIRE:
      action = WHITE_TOGGLE;
      break;
    case BTN_FIRE2:
      action = SAVE;
      break;
  }
  eiwomisa_doAction(action);
}


/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa_button.h)
  init(eiwomisa_button_init)
*/
