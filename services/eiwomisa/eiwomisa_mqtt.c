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

#define EIWOMISA_PROG_PUBLISH_TOPIC     EIWOMISA_MQTT_TOPIC "/from/prog"
#define EIWOMISA_PROGDIM_PUBLISH_TOPIC     EIWOMISA_MQTT_TOPIC "/from/prog/dimmer"
#define EIWOMISA_PROGSPEED_PUBLISH_TOPIC     EIWOMISA_MQTT_TOPIC "/from/prog/speed"
#define EIWOMISA_ACTION_PUBLISH_TOPIC   EIWOMISA_MQTT_TOPIC "/from/action"
#define EIWOMISA_BUTTON_PUBLISH_TOPIC   EIWOMISA_MQTT_TOPIC "/from/button/%u"
#define EIWOMISA_IRMP_PUBLISH_TOPIC     EIWOMISA_MQTT_TOPIC "/irmp/%02" PRId8 "/%04" PRIX16
#define EIWOMISA_MQTT_RETAIN            false
#define DATA_LENGTH                     8
#define TOPIC_LENGTH                    sizeof(EIWOMISA_IRMP_PUBLISH_TOPIC) + 10

#ifdef DEBUG_EIWOMISA_MQTT
#include "core/debug.h"
#define EIWOMISA_MQTT_DEBUG(s, ...) debug_printf("[mqtt] " s "\n", ## __VA_ARGS__)
#else
#define EIWOMISA_MQTT_DEBUG(...) do { } while(0)
#endif

Queue mqtt_action_queue = { NULL, NULL };
Queue mqtt_irmp_queue = { NULL, NULL };
Queue mqtt_button_queue = { NULL, NULL };
static uint8_t last_prog = 255, last_progspeed = 255, last_progdim = 0;

void
eiwomisa_poll_cb(void)
{
  EIWOMISA_MQTT_DEBUG("MQTT Poll");
  uint8_t len;
  uint8_t topic_len;
  char buf[DATA_LENGTH];
  char topic[TOPIC_LENGTH];
  
  // Program publish
  if (last_prog != eiwomisa_getProg())
  {
    last_prog = eiwomisa_getProg();
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%u"), last_prog);
    mqtt_construct_publish_packet(EIWOMISA_PROG_PUBLISH_TOPIC, buf, len, EIWOMISA_MQTT_RETAIN);
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_PROG_PUBLISH_TOPIC, buf);
  }

  // Program dimmer publish
  if (last_progdim != eiwomisa_getProgDimmer())
  {
    last_progdim = eiwomisa_getProgDimmer();
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%u"), last_progdim);
    mqtt_construct_publish_packet(EIWOMISA_PROGDIM_PUBLISH_TOPIC, buf, len, EIWOMISA_MQTT_RETAIN);
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_PROGDIM_PUBLISH_TOPIC, buf);
  }

  // Program publish
  if (last_progspeed != eiwomisa_getProgSpeed())
  {
    last_progspeed = eiwomisa_getProgSpeed();
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%u"), last_progspeed);
    mqtt_construct_publish_packet(EIWOMISA_PROGSPEED_PUBLISH_TOPIC, buf, len, EIWOMISA_MQTT_RETAIN);
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_PROGSPEED_PUBLISH_TOPIC, buf);
  }
  
  // Action publish
  while (!isEmpty(&mqtt_action_queue))
  {
    char *data = pop(&mqtt_action_queue);
    if (!mqtt_construct_publish_packet(EIWOMISA_ACTION_PUBLISH_TOPIC, data, strlen(data), EIWOMISA_MQTT_RETAIN))
    {
      EIWOMISA_MQTT_DEBUG("MQTT send action failed!");
      free(data);
      return;
    }
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_ACTION_PUBLISH_TOPIC, buf);
    free(data);
  }
  
#ifdef  EIWOMISA_IRMP_SUPPORT
  // IRMP publish
  while (!isEmpty(&mqtt_irmp_queue))
  {
    irmp_data_t *data;
    data = (irmp_data_t*)pop(&mqtt_irmp_queue);
    topic_len = snprintf_P(topic, TOPIC_LENGTH, PSTR(EIWOMISA_IRMP_PUBLISH_TOPIC), data->protocol, data->address);
    len = snprintf_P(buf, DATA_LENGTH, PSTR("%02" PRIX16), data->command);
    if (!mqtt_construct_publish_packet(topic, buf, len, EIWOMISA_MQTT_RETAIN))
    {
      EIWOMISA_MQTT_DEBUG("MQTT send irmp failed!");
      free(data);
      return;
    }
    EIWOMISA_MQTT_DEBUG("%s=%s", EIWOMISA_IRMP_PUBLISH_TOPIC, buf);
    free(data);
  }
#endif
}

void
eiwomisa_publish_cb(char const *topic, uint16_t topic_length,
                   const void *payload, uint16_t payload_length)
{
  EIWOMISA_MQTT_DEBUG("MQTT Publish: %s", topic);
  if (topic_length < 20)
    return;

  if (topic[sizeof(EIWOMISA_MQTT_TOPIC)] == 's')
  {

  }
  else if (topic[sizeof(EIWOMISA_MQTT_TOPIC)] == 'q')
  {

  }
  else
    EIWOMISA_MQTT_DEBUG("MQTT parse error");
}

static void
eiwomisa_connack_cb(void)
{
//  EIWOMISA_MQTT_DEBUG("MQTT Sub: " EIWOMISA_SUBSCRIBE_SET_TOPIC);
//  mqtt_construct_subscribe_packet(EIWOMISA_SUBSCRIBE_SET_TOPIC);
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
