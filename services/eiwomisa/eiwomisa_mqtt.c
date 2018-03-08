/*
 *
 * Copyright (c) 2016 by Daniel Lindner <daniel.lindner@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
#include "core/util/string_parsing.h"

#include "hardware/ir/irmp/irmp.h"
#include "protocols/mqtt/mqtt.h"

#include "eiwomisa.h"
#include "eiwomisa_mqtt.h"

#define EIWOMISA_PROG_PUBLISH_TOPIC         EIWOMISA_MQTT_TOPIC "/from/prog"
#define EIWOMISA_PROGDIM_PUBLISH_TOPIC      EIWOMISA_MQTT_TOPIC "/from/prog/dimmer"
#define EIWOMISA_PROGSPEED_PUBLISH_TOPIC    EIWOMISA_MQTT_TOPIC "/from/prog/speed"
#define EIWOMISA_WHITE_PUBLISH_TOPIC        EIWOMISA_MQTT_TOPIC "/from/white"
#define EIWOMISA_RGB_PUBLISH_TOPIC          EIWOMISA_MQTT_TOPIC "/from/rgb"
#define EIWOMISA_ACTION_PUBLISH_TOPIC       EIWOMISA_MQTT_TOPIC "/from/action"
#define EIWOMISA_BUTTON_PUBLISH_TOPIC       EIWOMISA_MQTT_TOPIC "/from/button/%u"
#define EIWOMISA_SUBSCRIBE_TOPIC            EIWOMISA_MQTT_TOPIC "/to/#"
#define EIWOMISA_PROG_SUBSCRIBE_TOPIC       EIWOMISA_MQTT_TOPIC "/to/prog"
#define EIWOMISA_PROGDIM_SUBSCRIBE_TOPIC    EIWOMISA_MQTT_TOPIC "/to/prog/dimmer"
#define EIWOMISA_PROGSPEED_SUBSCRIBE_TOPIC  EIWOMISA_MQTT_TOPIC "/to/prog/speed"
#define EIWOMISA_WHITE_SUBSCRIBE_TOPIC      EIWOMISA_MQTT_TOPIC "/to/white"
#define EIWOMISA_RGB_SUBSCRIBE_TOPIC        EIWOMISA_MQTT_TOPIC "/to/rgb"
#define EIWOMISA_ACTION_SUBSCRIBE_TOPIC     EIWOMISA_MQTT_TOPIC "/to/action"
#define EIWOMISA_MQTT_RETAIN              false
#define DATA_LENGTH                       10
#define TOPIC_LENGTH                      sizeof(EIWOMISA_PROGDIM_PUBLISH_TOPIC) + 10

#ifdef DEBUG_EIWOMISA_MQTT
#include "core/debug.h"
#define EIWOMISA_MQTT_DEBUG(s, ...) debug_printf("[mqtt] " s "\n", ## __VA_ARGS__)
#else
#define EIWOMISA_MQTT_DEBUG(...) do { } while(0)
#endif

Queue mqtt_action_queue = { NULL, NULL };
Queue mqtt_button_queue = { NULL, NULL };

static uint8_t last_prog = 255, last_progspeed = 255, last_progdim =
  0, last_white = 0;
static uint32_t last_rgb = 0;

void
eiwomisa_poll_cb(void)
{
  uint8_t len;
  char buf[DATA_LENGTH];
  char topic[TOPIC_LENGTH];

  // Program publish
  if (last_prog != eiwomisa_getProg())
  {
    last_prog = eiwomisa_getProg();
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%u"), last_prog);
    mqtt_construct_publish_packet(EIWOMISA_PROG_PUBLISH_TOPIC, buf, len,
                                  EIWOMISA_MQTT_RETAIN);
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_PROG_PUBLISH_TOPIC, buf);
  }

  // Program dimmer publish
  if (last_progdim != eiwomisa_getProgDimmer())
  {
    last_progdim = eiwomisa_getProgDimmer();
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%u"), last_progdim);
    mqtt_construct_publish_packet(EIWOMISA_PROGDIM_PUBLISH_TOPIC, buf, len,
                                  EIWOMISA_MQTT_RETAIN);
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_PROGDIM_PUBLISH_TOPIC, buf);
  }

  // Program speed publish
  if (last_progspeed != eiwomisa_getProgSpeed())
  {
    last_progspeed = eiwomisa_getProgSpeed();
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%u"), last_progspeed);
    mqtt_construct_publish_packet(EIWOMISA_PROGSPEED_PUBLISH_TOPIC, buf, len,
                                  EIWOMISA_MQTT_RETAIN);
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_PROGSPEED_PUBLISH_TOPIC, buf);
  }

  // White value publish
  if (last_white != eiwomisa_getWhite())
  {
    last_white = eiwomisa_getWhite();
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%u"), last_white);
    mqtt_construct_publish_packet(EIWOMISA_WHITE_PUBLISH_TOPIC, buf, len,
                                  EIWOMISA_MQTT_RETAIN);
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_WHITE_PUBLISH_TOPIC, buf);
  }

  // White rgb values publish
  if (last_rgb != eiwomisa_getWhiteRGB())
  {
    last_rgb = eiwomisa_getWhiteRGB();
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%06" PRIX32), last_rgb);
    mqtt_construct_publish_packet(EIWOMISA_RGB_PUBLISH_TOPIC, buf, len,
                                  EIWOMISA_MQTT_RETAIN);
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_RGB_PUBLISH_TOPIC, buf);
  }

  // Action publish
  while (!isEmpty(&mqtt_action_queue))
  {
    char *data = pop(&mqtt_action_queue);
    if (!mqtt_construct_publish_packet
        (EIWOMISA_ACTION_PUBLISH_TOPIC, data, strlen(data),
         EIWOMISA_MQTT_RETAIN))
    {
      EIWOMISA_MQTT_DEBUG("MQTT send action failed!");
      free(data);
      return;
    }
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_ACTION_PUBLISH_TOPIC, data);
    free(data);
  }

#ifdef  EIWOMISA_BUTTON_SUPPORT
  // Button publish
  while (!isEmpty(&mqtt_button_queue))
  {
    button_data_t *data;
    data = (button_data_t *) pop(&mqtt_button_queue);
    snprintf_P(topic, TOPIC_LENGTH, PSTR(EIWOMISA_BUTTON_PUBLISH_TOPIC),
               data->button);
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%02" PRId8), data->status);
    if (!mqtt_construct_publish_packet(topic, buf, len, EIWOMISA_MQTT_RETAIN))
    {
      EIWOMISA_MQTT_DEBUG("MQTT send button failed!");
      free(data);
      return;
    }
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_BUTTON_PUBLISH_TOPIC, buf);
    free(data);
  }
#endif
}

void
eiwomisa_publish_cb(char const *topic, uint16_t topic_length,
                    const void *payload, uint16_t payload_length)
{

  EIWOMISA_MQTT_DEBUG("MQTT Publish: %s", topic);
  if (topic_length < sizeof(EIWOMISA_SUBSCRIBE_TOPIC))
    return;
  if (payload_length >= DATA_LENGTH)
    return;

  char strpayload[DATA_LENGTH];
  int16_t raw_val;

  memcpy(strpayload, payload, payload_length);
  strpayload[payload_length] = 0;
  sscanf_P(strpayload, PSTR("%i"), &raw_val);

  if (strncmp_P(topic, PSTR(EIWOMISA_PROG_SUBSCRIBE_TOPIC), topic_length) ==
      0)
  {
    eiwomisa_setProg(raw_val);
  }
  else
    if (strncmp_P(topic, PSTR(EIWOMISA_PROGDIM_SUBSCRIBE_TOPIC), topic_length)
        == 0)
  {
    eiwomisa_setProgDimmer(raw_val);
  }
  else
    if (strncmp_P
        (topic, PSTR(EIWOMISA_PROGSPEED_SUBSCRIBE_TOPIC), topic_length) == 0)
  {
    eiwomisa_setProgSpeed(raw_val);
  }
  else
    if (strncmp_P(topic, PSTR(EIWOMISA_WHITE_SUBSCRIBE_TOPIC), topic_length)
        == 0)
  {
    eiwomisa_setWhite(raw_val);
  }
  else if (strncmp_P(topic, PSTR(EIWOMISA_RGB_SUBSCRIBE_TOPIC), topic_length)
           == 0)
  {
    if (payload_length > 5)
    {
      uint8_t red, green, blue, len = 0;
      len += next_hexbyte(strpayload, &red);
      len += next_hexbyte(strpayload + len, &green);
      len += next_hexbyte(strpayload + len, &blue);
      eiwomisa_setWhiteRGB(red, green, blue);
    }
  }
  else
    if (strncmp_P(topic, PSTR(EIWOMISA_ACTION_SUBSCRIBE_TOPIC), topic_length)
        == 0)
  {
    eiwomisa_doAction(raw_val);
  }
  else
    EIWOMISA_MQTT_DEBUG("MQTT parse error");

}

static void
eiwomisa_connack_cb(void)
{
  EIWOMISA_MQTT_DEBUG("MQTT Sub: " EIWOMISA_SUBSCRIBE_TOPIC);
  mqtt_construct_subscribe_packet(EIWOMISA_SUBSCRIBE_TOPIC);
  mqtt_construct_publish_packet(MQTT_STATIC_CONF_WILL_TOPIC, "1", 1, true);
  last_prog = 255;
  last_progspeed = 255;
  last_progdim = 0;
  last_white = 0;
}

static const mqtt_callback_config_t mqtt_callback_config PROGMEM = {
  .connack_callback = eiwomisa_connack_cb,
  .poll_callback = eiwomisa_poll_cb,
  .close_callback = NULL,
  .publish_callback = eiwomisa_publish_cb,
};

void
eiwomisa_mqtt_init()
{
  EIWOMISA_MQTT_DEBUG("MQTT Init");
  mqtt_register_callback(&mqtt_callback_config);
}

/*
  -- Ethersex META --
  header(services/eiwomisa/eiwomisa_mqtt.h)
  net_init(eiwomisa_mqtt_init)
*/
