/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
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
#include <avr/pgmspace.h>
#include <avr/io.h>

#include "config.h"

#include "eiwomisa.h"
#include "eiwomisa_pwm.h"
#include "core/periodic.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef EIWOMISA_USE_CIE1931
#include "cie1931.h"
#endif

#ifdef DEBUG_EIWOMISA_PWM
#include "core/debug.h"
#define PWMDEBUG(a...)  debug_printf("pwm: " a)
#else
#define PWMDEBUG(a...)
#endif

static uint8_t channelVal[LED_ALL];
static uint8_t channelFade[LED_ALL];

// init DDR, waveform and timer
void
eiwomisa_pwm_init()
{
  TC1_INPUT_CAPTURE = PERIODIC_TOP;     // set the timer top value (PWM_Freq= F_CPU /(Prescaler * (ICR1 + 1)) )
  TC1_MODE_PWMFAST_ICR;               // Fast PWM, TOP ICR1, Pin low on OCR1A,OCR1B match
  TC1_PRESCALER_8;              // clockselect: clkI/O/8 (From prescaler)

  TC3_INPUT_CAPTURE = PERIODIC_TOP;     // set the timer top value (PWM_Freq= F_CPU /(Prescaler * (ICR1 + 1)) )
  TC3_MODE_PWMFAST_ICR;               // Fast PWM, TOP ICR1, Pin low on OCR1A,OCR1B match
  TC3_PRESCALER_8;              // clockselect: clkI/O/8 (From prescaler)

  PWMDEBUG("PWM1 freq: %u Hz\n", F_CPU / (8 * (TC1_INPUT_CAPTURE + 1)));
  PWMDEBUG("PWM2 freq: %u Hz\n", F_CPU / (8 * (TC3_INPUT_CAPTURE + 1)));
  
  //Set all channels to Min Value
  e_leds i;
  for (i = 0; i < LED_ALL; i++)
  {
    eiwomisa_setpwm(i, 0);
  }

  // Activate PWM Outputs
  TC1_OUTPUT_COMPARE_CLEAR;     // Clear OCnA on compare match
  TC1_OUTPUT_COMPARE_B_CLEAR;   // Clear OCnB on compare match
  TC3_OUTPUT_COMPARE_CLEAR;     // Clear OCnA on compare match
  TC3_OUTPUT_COMPARE_B_CLEAR;   // Clear OCnB on compare match
}

// set pwm to hardware value use setpwm or setpwmfade to set from extern
static void
eiwomisa_setpwm_hardware(const e_leds channel, const uint8_t setval)
{
  PWMDEBUG("set hw %c, values: %i\n", channel, setval);
  uint16_t temp = pgm_read_word_near(cie_luminance_8bit_to_top + setval);
  switch (channel)
  {
    case LED_B:
      if (temp > 0)
      {
        TC1_COUNTER_COMPARE = temp;
        TC1_OUTPUT_COMPARE_CLEAR;
      }
      else
      {
        TC1_OUTPUT_COMPARE_NONE;
        PIN_CLEAR(CHANNEL_B_PWM);
      }
      break;
    case LED_W:
      if (temp > 0)
      {
        TC1_COUNTER_COMPARE_B = temp;
        TC1_OUTPUT_COMPARE_B_CLEAR;
      }
      else
      {
        TC1_OUTPUT_COMPARE_B_NONE;
        PIN_CLEAR(CHANNEL_W_PWM);
      }
      break;
    case LED_R:
      if (temp > 0)
      {
        TC3_COUNTER_COMPARE = temp;
        TC3_OUTPUT_COMPARE_CLEAR;
      }
      else
      {
        TC3_OUTPUT_COMPARE_NONE;
        PIN_CLEAR(CHANNEL_R_PWM);
      }
      break;
    case LED_G:
      if (temp > 0)
      {
        TC3_COUNTER_COMPARE_B = temp;
        TC3_OUTPUT_COMPARE_B_CLEAR;
      }
      else
      {
        TC3_OUTPUT_COMPARE_B_NONE;
        PIN_CLEAR(CHANNEL_G_PWM);
      }
      break;
    default:
      PWMDEBUG("channel %c unsupported\n", channel);
  }
}

// return current pwm value
uint8_t
eiwomisa_getpwm(const e_leds channel)
{
  return channelVal[channel];
}

// return target pwm value
uint8_t
eiwomisa_getpwmfade(const e_leds channel)
{
  return channelFade[channel];
}

void 
eiwomisa_setpwmfade(const e_leds channel, const uint8_t setval)
{
  channelFade[channel] = setval;
}

// set pwm value
void
eiwomisa_setpwm(const e_leds channel, const uint8_t setval)
{
  PWMDEBUG("set %c, values: %i\n", channel, setval);
  channelVal[channel]=setval;
  eiwomisa_setpwmfade(channel, setval);
  eiwomisa_setpwm_hardware(channel, setval);
}

// ECMD:  set/get fading for channel
int16_t
parse_cmd_eiwomisa_pwm_fade_command(char *cmd, char *output, const uint16_t len)
{
  if (cmd[0] == '\0')
  {
    e_leds i;
    for (i = 0; i < LED_ALL; i++)
    {
      PWMDEBUG("%i: %i\n", i, eiwomisa_getpwmfade(i));
    }

    return ECMD_FINAL_OK;
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
      PWMDEBUG("%i: %i\n", i, eiwomisa_getpwm(i));
    }

    return ECMD_FINAL_OK;
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

void
eiwomisa_pwm_periodic()
{
  /* Fade channels. */
  for (e_leds i = 0; i < LED_ALL; i++)
  {
    if (channelVal[i] < channelFade[i])
    {
      eiwomisa_setpwm(i, channelVal[i] + 1);
    }
    else if (channelVal[i] > channelFade[i])
    {
      eiwomisa_setpwm(i, channelVal[i] - 1);
    }
  }
}



/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa_pwm.h)
  init(eiwomisa_pwm_init)
  timer(1, eiwomisa_pwm_periodic())
  block([[EIWOMISA_PWM]])
  ecmd_feature(eiwomisa_pwm_fade_command, "eiwomisa_pwm fade", [channel value], Set/Get fade channel value)  
  ecmd_feature(eiwomisa_pwm_command, "eiwomisa_pwm", [channel value], Set/Get channel value)
*/
