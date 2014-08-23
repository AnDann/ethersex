/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
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
#include <avr/interrupt.h>

#include "config.h"

#include "core/periodic.h"
#include "pwm_common.h"
#include "pwm.h"
#include "protocols/ecmd/ecmd-base.h"


#ifdef PWM_LED_SUPPORT
#include "cie1931.h"
uint16_t pwm_ticks=0;
#define FULL_OVERFLOWS ((F_CPU/8/4000/HZ)+1)
#endif

#ifdef CH_A_PWM_GENERAL_SUPPORT
  uint8_t channelAval=PWM_MIN_VALUE;
#ifdef PWM_LED_FADING_SUPPORT
  uint8_t channelAfade=PWM_MIN_VALUE;
#endif // PWM_LED_FADING_SUPPORT
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
  uint8_t channelBval=PWM_MIN_VALUE;
#ifdef PWM_LED_FADING_SUPPORT
  uint8_t channelBfade=PWM_MIN_VALUE;
#endif // PWM_LED_FADING_SUPPORT
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
  uint8_t channelCval=PWM_MIN_VALUE;
#ifdef PWM_LED_FADING_SUPPORT
  uint8_t channelCfade=PWM_MIN_VALUE;
#endif // PWM_LED_FADING_SUPPORT
#endif /* CH_C_PWM_GENERAL_SUPPORT */

#ifdef PWM_LED_FADING_SUPPORT
volatile uint8_t pwm_fade_counter = 0;
volatile uint8_t pwm_fade_step = 0;
#endif // PWM_LED_FADING_SUPPORT

#ifdef PWM_GENERAL_FADING_SUPPORT
  int8_t fadingAspeed=0;  // 0 = disable
  int8_t fadingBspeed=0;
  int8_t fadingCspeed=0;
#endif /* PWM_GENERAL_FADING_SUPPORT */

// init DDR, waveform and timer
void
pwm_init(){
//  TC1_COUNTER_CURRENT=0x0000; //set the timer counter
#ifdef CH_A_PWM_GENERAL_SUPPORT
  DDR_CONFIG_OUT(CHANNEL_A_PWM); 		// PWM OUTPUT
  TC1_COUNTER_COMPARE=pgm_read_word_near(cie_luminance_8_to_12bit+channelAval);
  TC1_OUTPUT_COMPARE_CLEAR; 		// Clear OCnA on compare match
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
  DDR_CONFIG_OUT(CHANNEL_B_PWM); 		// PWM OUTPUT
  TC1_COUNTER_COMPARE_B=pgm_read_word_near(cie_luminance_8_to_12bit+channelBval);
  TC1_OUTPUT_COMPARE_B_CLEAR; 		// Clear OCnB on compare match
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
  DDR_CONFIG_OUT(CHANNEL_C_PWM); 		// PWM OUTPUT
  OCR1C=pgm_read_word_near(cie_luminance_8_to_12bit+channelCval);
  TCCR1A|=_BV(COM1C1)|_BV(COM1C0); 		// Set OCnC on compare match
#endif /* CH_C_PWM_GENERAL_SUPPORT */

//  TCCR1A|=_BV(WGM10);  					// PWM, Phase Correct, 8-bit
//  TCCR1B|=_BV(WGM12); 					// waveform generation mode: CTC,
//  TCCR1B|=_BV(CS10); 					// clockselect: clkI/O/1 (No prescaler)
 
  TC1_INPUT_CAPTURE=3999;	 			// set the timer top value (PWM_Freq= F_CPU /(Prescaler * (ICR1 + 1)) )
  TC1_MODE_WGM14;						// Fast PWM, TOP ICR1, Pin low on OCR1A,OCR1B match
  TC1_PRESCALER_8;						// clockselect: clkI/O/8 (From prescaler)

  PWMDEBUG("PWM freq: %u Hz\n", F_CPU / (8 * (TC1_INPUT_CAPTURE + 1)));

  TC1_INT_OVERFLOW_ON;
  // activate PWM outports OC1C
/*
#ifdef CH_A_PWM_GENERAL_SUPPORT
  #if defined(_ATMEGA128)
  TCCR1C|=1<<FOC1A;					// with atmega128
  #else
  TCCR1A|=1<<FOC1A;  					// with atmega32
  #endif
#endif // CH_A_PWM_GENERAL_SUPPORT 
#ifdef CH_B_PWM_GENERAL_SUPPORT
  #if defined(_ATMEGA128)
  TCCR1C|=1<<FOC1B;					// with atmega128
  #else
  TCCR1A|=1<<FOC1B;						// with atmega 32
  #endif
#endif // CH_B_PWM_GENERAL_SUPPORT 
#ifdef CH_C_PWM_GENERAL_SUPPORT
  #if defined(_ATMEGA128)
  TCCR1C|=1<<FOC1C;  					// with atmega128
  #else
  TCCR1A|=1<<FOC1C; 					// with atmega 32
  #endif
#endif // CH_C_PWM_GENERAL_SUPPORT 
*/
}

#ifdef PWM_LED_SUPPORT
ISR(TC1_VECTOR_OVERFLOW)
{
	/*	Tick the clock from here because periodic.h has no access to Timer1*/
	if(++pwm_ticks >= FULL_OVERFLOWS)
	{
		pwm_ticks = 0;

        // call the regular ISR for timer expired condition (every 20ms)
        timer_expired();
	}
	if (pwm_fade_counter)
		pwm_fade_counter--;
}
#endif

// set pwm to hardware value use setpwm or setpwmfade to set from extern
void
setpwm_hardware(char channel, uint8_t setval){
  PWMDEBUG ("set hw %c, values: %i\n",channel, setval);
#ifdef PWM_GENERAL_INVERT_SUPPORT
  setval=255-setval;
#endif /* PWM_GENERAL_INVERT_SUPPORT */
	uint16_t temp = pgm_read_word_near(cie_luminance_8_to_12bit+setval);
  switch (channel){
#ifdef CH_A_PWM_GENERAL_SUPPORT
    case 'a': 
		if(temp > 0)
		{	
			TC1_COUNTER_COMPARE = temp;
			TC1_OUTPUT_COMPARE_CLEAR;
		}
		else
		{
			TC1_OUTPUT_COMPARE_NONE;
			PIN_CLEAR(CHANNEL_A_PWM);
		}
		channelAval = setval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    case 'b': 
		if(temp > 0)
		{	
			TC1_COUNTER_COMPARE_B = temp;
			TC1_OUTPUT_COMPARE_B_CLEAR;
		}
		else
		{
			TC1_OUTPUT_COMPARE_B_NONE;
			PIN_CLEAR(CHANNEL_B_PWM);
		}
		channelBval = setval;
	  break;
#endif /* CH_C_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    case 'c': 
      OCR1C=pgm_read_word_near(cie_luminance_8_to_12bit+setval);
		channelCval = setval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
    default:
    PWMDEBUG ("channel %c unsupported\n",channel);
  }
}

// return current pwm value
uint8_t
getpwm(char channel){
  uint8_t ret=0;
  switch (channel){
#ifdef CH_A_PWM_GENERAL_SUPPORT
    case 'a': ret=channelAval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    case 'b': ret=channelBval;
	  break;
#endif /* CH_C_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    case 'c': ret=channelCval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
    default:
    PWMDEBUG ("channel %c unsupported\n",channel);
  }
#ifdef PWM_GENERAL_INVERT_SUPPORT
  return 255-ret;
#else
  return ret;
#endif /* PWM_GENERAL_INVERT_SUPPORT */
}

#ifdef PWM_LED_FADING_SUPPORT
// return target pwm value
uint8_t
getpwmfade(char channel){
  uint8_t ret=0;
  switch (channel){
#ifdef CH_A_PWM_GENERAL_SUPPORT
    case 'a': ret=channelAfade;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    case 'b': ret=channelBfade;
	  break;
#endif /* CH_C_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    case 'c': ret=channelCfade;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
    default:
    PWMDEBUG ("channel %c unsupported\n",channel);
  }
#ifdef PWM_GENERAL_INVERT_SUPPORT
  return 255-ret;
#else
  return ret;
#endif /* PWM_GENERAL_INVERT_SUPPORT */
}

// set pwm fade value
void
setpwmfade(char channel, uint8_t setval){
  switch (channel){
#ifdef CH_A_PWM_GENERAL_SUPPORT
    case 'a': 
	  channelAfade=setval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    case 'b': 
	  channelBfade=setval;
	  break;
#endif /* CH_C_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    case 'c': 
	  channelCfade=setval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
    default:
    PWMDEBUG ("channel %c unsupported\n",channel);
  }
}

// Set pwm fade delay
void
setpwmfadestep(uint8_t setval)
{
	pwm_fade_step = setval;
}

// Set pwm fade delay
uint8_t
getpwmfadestep()
{
	return pwm_fade_step;
}

#endif

// set pwm value
void
setpwm(char channel, uint8_t setval){
  PWMDEBUG ("set %c, values: %i\n",channel, setval);
  setpwmfade(channel,setval);
  setpwm_hardware(channel,setval);
}

// set pwm via ecmdA
int16_t parse_cmd_pwm_command(char *cmd, char *output, uint16_t len) 
{
  uint8_t channel=cmd[1];
  uint8_t value=atoi(cmd+3);

  if (cmd[0]=='\0') {
#ifdef CH_A_PWM_GENERAL_SUPPORT
    PWMDEBUG ("a: %i\n",getpwm('a'));
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    PWMDEBUG ("b: %i\n",getpwm('b'));
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    PWMDEBUG ("c: %i\n",getpwm('c'));
#endif /* CH_C_PWM_GENERAL_SUPPORT */
    return ECMD_FINAL_OK;
  }
  if (cmd[2]=='\0') {
      return ECMD_FINAL(snprintf_P(output, len, PSTR("%i"), getpwm(channel)));
  }
  setpwm(channel,value);

  return ECMD_FINAL_OK;
}

#ifdef PWM_GENERAL_FADING_SUPPORT
// set fading for channel
int16_t parse_cmd_pwm_fade_command(char *cmd, char *output, uint16_t len) 
{
  uint8_t channel=cmd[1];
  int8_t diff=atoi(cmd+3);
  uint8_t startvalue=atoi(cmd+8);

  PWMDEBUG ("set ch: %c, diff %i, start %i\n",channel, diff,startvalue);
  setpwm(channel,startvalue);

  switch (channel){
#ifdef CH_A_PWM_GENERAL_SUPPORT
	case 'a': fadingAspeed=diff; break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
	case 'b': fadingBspeed=diff; break;
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
	case 'c': fadingCspeed=diff; break;
#endif /* CH_C_PWM_GENERAL_SUPPORT */
  }

  return ECMD_FINAL_OK;
}
#endif /* PWM_GENERAL_FADING_SUPPORT */

#ifdef PWM_LED_FADING_SUPPORT
// set fading for channel
int16_t parse_cmd_pwm_led_fade_command(char *cmd, char *output, uint16_t len) 
{
  uint8_t channel=cmd[1];
  uint8_t value=atoi(cmd+3);

  if (cmd[0]=='\0') {
#ifdef CH_A_PWM_GENERAL_SUPPORT
    PWMDEBUG ("a: %i\n",getpwmfade('a'));
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    PWMDEBUG ("b: %i\n",getpwmfade('b'));
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    PWMDEBUG ("c: %i\n",getpwmfade('c'));
#endif /* CH_C_PWM_GENERAL_SUPPORT */
    return ECMD_FINAL_OK;
  }
  if (cmd[2]=='\0') {
      return ECMD_FINAL(snprintf_P(output, len, PSTR("%i"), getpwmfade(channel)));
  }
  setpwmfade(channel,value);

  return ECMD_FINAL_OK;
}
#endif /* PWM_LED_FADING_SUPPORT */

void
pwm_periodic()
{
#ifdef PWM_GENERAL_FADING_SUPPORT

 #ifdef CH_A_PWM_GENERAL_SUPPORT
  if (fadingAspeed!=0){
    int16_t chAdiff = getpwm('a')+fadingAspeed;
    if (chAdiff >= PWM_MIN_VALUE) {
      fadingAspeed=0;
      setpwm('a',PWM_MIN_VALUE);
    } else if (chAdiff<=0) {
      fadingAspeed=0;
      setpwm('a',PWM_MAX_VALUE);
    } else
      setpwm('a',chAdiff);
  }
 #endif /* CH_A_PWM_GENERAL_SUPPORT */
 #ifdef CH_B_PWM_GENERAL_SUPPORT
  if (fadingBspeed!=0){
    int16_t chBdiff = getpwm('b')+fadingBspeed;
    if (chBdiff >= PWM_MIN_VALUE) {
      fadingBspeed=0;
      setpwm('b',PWM_MIN_VALUE);
    } else if (chBdiff<=0) {
      fadingBspeed=0;
      setpwm('b',PWM_MAX_VALUE);
    } else
      setpwm('b',chBdiff);
  }
 #endif /* CH_B_PWM_GENERAL_SUPPORT */
 #ifdef CH_C_PWM_GENERAL_SUPPORT
  if (fadingCspeed!=0){
    int16_t chCdiff = getpwm('c')+fadingCspeed;
    if (chCdiff >= PWM_MIN_VALUE) {
      fadingCspeed=0;
      setpwm('c',PWM_MIN_VALUE);
    } else if (chCdiff<=0) {
      fadingCspeed=0;
      setpwm('c',PWM_MAX_VALUE);
    } else
      setpwm('c',chCdiff);
  }
 #endif /* CH_C_PWM_GENERAL_SUPPORT */

#endif /* PWM_GENERAL_FADING_SUPPORT */

#ifdef PWM_LED_FADING_SUPPORT
  /* the main loop is too fast, slow down */
  PWMDEBUG ("Main Loop\n");
  if (pwm_fade_counter == 0)
  {
    /* Fade channels. */
    PWMDEBUG ("Fade channels\n");
#ifdef CH_A_PWM_GENERAL_SUPPORT
      if (channelAval < channelAfade)
	  {
		setpwm_hardware('a',channelAval+1);
	  }
      else if (channelAval > channelAfade)
	  {
		setpwm_hardware('a',channelAval-1);
	  }
#endif /* CH_A_PWM_GENERAL_SUPPORT */

#ifdef CH_B_PWM_GENERAL_SUPPORT
      if (channelBval < channelBfade)
	  {
		setpwm_hardware('b',channelBval+1);
	  }
      else if (channelBval > channelBfade)
	  {
		setpwm_hardware('b',channelBval-1);
	  }
#endif /* CH_B_PWM_GENERAL_SUPPORT */

#ifdef CH_C_PWM_GENERAL_SUPPORT
      if (getpwm('c') < getpwmfade('c'))
	  {
		setpwm_hardware('c',getpwm('c')+1);
	  }
      else if (getpwm('c') > getpwmfade('c'))
	  {
		setpwm_hardware('c',getpwm('c')-1);
	  }
#endif /* CH_C_PWM_GENERAL_SUPPORT */

    /* reset counter */
    pwm_fade_counter = pwm_fade_step;
  }

#endif
}



/*
  -- Ethersex META --
  header(hardware/pwm/pwm.h)
  init(pwm_init)
  mainloop(pwm_periodic)
dnl timer(1, `pwm_periodic()')
  block([[PWM]])
  ecmd_ifdef(PWM_GENERAL_FADING_SUPPORT)
    ecmd_feature(pwm_fade_command, "pwm fade", [channel +-diff startvalue], Set fading at channel with startvalue and change each stepp to diff (must be signed 3 digit))
  ecmd_endif()
  ecmd_feature(pwm_command, "pwm set", [channel value], Set channel to value)
  ecmd_ifdef(PWM_LED_FADING_SUPPORT)
    ecmd_feature(pwm_led_fade_command, "pwm led fade", [channel value], Fade led channel from actual to value)  
  ecmd_endif()
*/
