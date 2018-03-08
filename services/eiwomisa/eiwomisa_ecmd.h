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

#ifndef HAVE_EIWOMISA_ECMD_H
#define HAVE_EIWOMISA_ECMD_H

int16_t parse_cmd_eiwomisa_prog_speed(char *cmd, char *output,
                                      const uint16_t len);
int16_t parse_cmd_eiwomisa_prog(char *cmd, char *output, const uint16_t len);
int16_t parse_cmd_eiwomisa_action(char *cmd, char *output,
                                  const uint16_t len);

#ifndef TEENSY_SUPPORT
int16_t parse_cmd_eiwomisa_save(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_eiwomisa_load(char *cmd, char *output, uint16_t len);
#endif

int16_t parse_cmd_eiwomisa_pwm_fade_command(char *cmd, char *output,
                                            const uint16_t len);
int16_t parse_cmd_eiwomisa_pwm_command(char *cmd, char *output,
                                       const uint16_t len);
int16_t parse_cmd_eiwomisa_pwm_delay_command(char *cmd, char *output,
                                             const uint16_t len);
int16_t parse_cmd_eiwomisa_white_command(char *cmd, char *output,
                                         const uint16_t len);
int16_t parse_cmd_eiwomisa_white_rgb_command(char *cmd, char *output,
                                             const uint16_t len);
#endif /* HAVE_EIWOMISA_ECMD_H */
