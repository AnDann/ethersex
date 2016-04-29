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
#include <string.h>
#include <avr/pgmspace.h>

#include "config.h"

#include "eiwomisa_pwm.h"
#include "eiwomisa_ecmd.h"
#include "protocols/ecmd/ecmd-base.h"

#ifndef TEENSY_SUPPORT
int16_t
parse_cmd_eiwomisa_save(char *cmd, char *output, uint16_t len)
{
  eiwomisa_storeToEEPROM();
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_eiwomisa_load(char *cmd, char *output, uint16_t len)
{
  eiwomisa_loadFromEEPROM();
  return ECMD_FINAL_OK;
}
#endif

int16_t
parse_cmd_eiwomisa_prog(char *cmd, char *output, const uint16_t len)
{
  if (cmd[0])
  {
    eiwomisa_setProg (atoi(cmd));
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(eiwomisa_getProg(), output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

#ifdef EIWOMISA_DMX_SUPPORT
int16_t
parse_cmd_eiwomisa_prog_speed(char *cmd, char *output, const uint16_t len)
{
  if (cmd[0])
  {
    eiwomisa_setProgSpeed(atoi(cmd));
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(eiwomisa_getProgSpeed(), output, 10);
    return ECMD_FINAL(strlen(output));
  }
}
#endif

// ECMD:  set/get fading for channel
int16_t
parse_cmd_eiwomisa_pwm_fade_command(char *cmd, char *output, const uint16_t len)
{
  if (cmd[0] == '\0')
  {
    e_leds i;
    for (i = 0; i < LED_ALL; i++)
    {
      output += snprintf_P(output, len, PSTR("%i "), eiwomisa_getpwmfade(i));
    }
    return ECMD_FINAL(strlen(output));
  }
  e_leds channel = atoi(cmd + 1);
  if (cmd[2] == '\0')
  {
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("%i"), eiwomisa_getpwmfade(channel)));
  }
  uint8_t value = atoi(cmd + 3);
  eiwomisa_setpwmfade(channel, value);

  return ECMD_FINAL_OK;
}

// ECMD:  set/get channel
int16_t
parse_cmd_eiwomisa_pwm_command(char *cmd, char *output, const uint16_t len)
{

  if (cmd[0] == '\0')
  {
    e_leds i;
    for (i = 0; i < LED_ALL; i++)
    {
      output += snprintf_P(output, len, PSTR("%i "), eiwomisa_getpwm(i));
    }
    return ECMD_FINAL(strlen(output));
  }
  e_leds channel = atoi(cmd + 1);
  if (cmd[2] == '\0')
  {
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("%i"), eiwomisa_getpwm(channel)));
  }
  uint8_t value = atoi(cmd + 3);
  eiwomisa_setpwm(channel, value);

  return ECMD_FINAL_OK;
}

// ECMD:  set/get fading delay
int16_t
parse_cmd_eiwomisa_pwm_delay_command(char *cmd, char *output, const uint16_t len)
{
  if (cmd[0])
  {
    eiwomisa_setfadeDelay(atoi(cmd));
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(eiwomisa_getfadeDelay(), output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa_ecmd.h)
  block([[EIWOMISA]])
  ecmd_ifdef(EIWOMISA_DMX_SUPPORT)
    ecmd_feature(eiwomisa_prog_speed, "eiwomisa prog speed", , get/set actual programspeed)
  ecmd_endif()
  ecmd_feature(eiwomisa_prog, "eiwomisa prog", , get/set actual program)
  ecmd_ifndef(TEENSY_SUPPORT)
    ecmd_feature(eiwomisa_save, "eiwomisa save", , write channels to EEPROM)
    ecmd_feature(eiwomisa_load, "eiwomisa load", , write channels to EEPROM)
  ecmd_endif()
  block([[EIWOMISA_PWM]])
  ecmd_feature(eiwomisa_pwm_fade_command, "eiwomisa pwm fade", [channel value], Set/Get fade channel value)  
  ecmd_feature(eiwomisa_pwm_delay_command, "eiwomisa pwm delay", [value], Set/Get delay value)
  ecmd_feature(eiwomisa_pwm_command, "eiwomisa pwm", [channel value], Set/Get channel value)
*/