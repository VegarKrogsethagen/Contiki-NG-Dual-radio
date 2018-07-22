/*
 * Copyright (c) 2017, Yasuyuki Tanaka
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <contiki.h>
#include <lib/assert.h>
#include <sys/node-id.h>
#include <net/mac/tsch/tsch.h>
#include <net/mac/tsch/tsch-queue.h>
#include <net/mac/tsch/sixtop/sixtop.h>

extern const sixtop_sf_t test_sf;
extern int test_sf_start(const linkaddr_t *addr);

PROCESS(sixp_node_process, "6P node");
AUTOSTART_PROCESSES(&sixp_node_process);

#define COORDINATOR_NODE_ID 1
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_COAP                        LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_LWM2M                       LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_6TOP                        LOG_LEVEL_DBG

PROCESS_THREAD(sixp_node_process, ev, data)
{
  PROCESS_BEGIN();

  sixtop_add_sf(&test_sf);

  if(node_id == COORDINATOR_NODE_ID) {
    tsch_set_coordinator(1);
    assert(test_sf_start(NULL) == 0);
  } else {
    static struct etimer et;
    struct tsch_neighbor *peer;
    etimer_set(&et, CLOCK_SECOND);
    while(tsch_is_associated == 0) {
      PROCESS_YIELD_UNTIL(etimer_expired(&et));
      etimer_reset(&et);
    }
    peer = tsch_queue_get_time_source();
    assert(test_sf_start((const linkaddr_t *)&peer->addr) == 0);
  }

  PROCESS_END();
}
