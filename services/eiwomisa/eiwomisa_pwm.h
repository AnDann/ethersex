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

#ifndef HAVE_EIWOMISA_PWM_H
#define HAVE_EIWOMISA_PWM_H

#include "eiwomisa.h"

void eiwomisa_pwm_init();

void eiwomisa_pwm_periodic();

uint8_t eiwomisa_getpwm(const e_leds channel);
void eiwomisa_setpwm(const e_leds channel,const uint8_t setval);
uint8_t eiwomisa_getpwmfade(const e_leds channel);
void eiwomisa_setpwmfade(const e_leds channel,const uint8_t setval);
uint8_t eiwomisa_getfadeDelay();
void eiwomisa_setfadeDelay(const uint8_t setval);

#endif /* HAVE_EIWOMISA_PWM_H */
