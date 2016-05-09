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

#include "protocols/mqtt/mqtt.h"

#include "eiwomisa.h"
#include "eiwomisa_mqtt.h"

#define EIWOMISA_MQTT_RETAIN             false

#ifdef DEBUG_EIWOMISA_MQTT
#include "core/debug.h"
#define EIWOMISA_MQTT_DEBUG(s, ...) debug_printf("[mqtt] " s "\n", ## __VA_ARGS__)
#else
#define EIWOMISA_MQTT_DEBUG(...) do { } while(0)
#endif

void
eiwomisa_poll_cb(void)
{
  EIWOMISA_MQTT_DEBUG("MQTT Poll");

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
  EIWOMISA_MQTT_DEBUG("MQTT Sub: " EIWOMISA_SUBSCRIBE_SET_TOPIC);
  mqtt_construct_subscribe_packet(EIWOMISA_SUBSCRIBE_SET_TOPIC);
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
