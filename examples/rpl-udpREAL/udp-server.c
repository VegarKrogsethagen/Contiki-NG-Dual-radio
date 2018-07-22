/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/energest.h"
#include "sys/log.h"
#include "os/net/mac/tsch/tsch.h"
#include "os/services/orchestra/orchestra.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define ENERGEST_CONF_CURRENT_TIME clock_time
#define ENERGEST_CONF_TIME_T clock_time_t
#define ENERGEST_CONF_SECOND CLOCK_SECOND


#define Energest_interval 50
#define WITH_SERVER_REPLY 0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define PAYLOAD_LEN	10000
static struct simple_udp_connection udp_conn;
static int teller=0;
static uint32_t delay=0;
static uint64_t totaldelay=0;
static uint8_t lavest=100;
static uint8_t hoyest=0;
PROCESS(udp_server_process, "UDP server");
PROCESS(Energest, "Energest");
AUTOSTART_PROCESSES(&udp_server_process,&Energest);

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
 uint32_t Current_ASN =  tsch_current_asn.ls4b;
 //unsigned count = *(unsigned *)data;
 delay = Current_ASN - *(unsigned *)data;
 totaldelay = delay+totaldelay;
 if(delay>hoyest)
 {
   hoyest = delay;
 }
 else if(delay<lavest)
 {
   lavest = delay;
 }
 teller++;
 printf("Reciving package = %i total delay = %d highest = %i lowest = %i\n", teller,totaldelay,hoyest,lavest);
#if WITH_SERVER_REPLY
 uipbuf_set_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS, 0);
 LOG_INFO("Sending response %u (Max MAC Tx: %d) to ", count,
    uipbuf_get_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS));
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
simple_udp_sendto(&udp_conn, &count, sizeof(count), sender_addr);
#endif /* WITH_SERVER_REPLY */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();
 
  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}
PROCESS_THREAD(Energest, ev, data)
{
  
  PROCESS_BEGIN();
 static struct etimer periodic_timer;
 etimer_set(&periodic_timer,Energest_interval);
 energest_init();
 while(1)
 {
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
 	     energest_flush();
            printf("Server Energest CPU: %lu LPM: %lu Deep LPM: %lu Total time: %lu seconds\n",
           (unsigned long)(energest_type_time(ENERGEST_TYPE_CPU) / ENERGEST_SECOND),
           (unsigned long)(energest_type_time(ENERGEST_TYPE_LPM) / ENERGEST_SECOND),
           (unsigned long)(energest_type_time(ENERGEST_TYPE_DEEP_LPM) / ENERGEST_SECOND),
           (unsigned long)(ENERGEST_GET_TOTAL_TIME() / ENERGEST_SECOND));
         etimer_reset(&periodic_timer);
 }
 PROCESS_END();
}


/*---------------------------------------------------------------------------*/

