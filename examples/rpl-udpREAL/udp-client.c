#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/energest.h"
#include "net/packetbuf.c"
#include "os/net/queuebuf.c"
#include "os/sys/clock.h"
#include "os/sys/log.h"
#include "os/sys/log.c"
#include "os/net/mac/tsch/tsch.h"



#define LOG_MODULE "Test"
#define LOG_LEVEL LOG_LEVEL_DBG
#define ENERGEST_CONF_CURRENT_TIME clock_time
#define ENERGEST_CONF_TIME_T clock_time_t
#define ENERGEST_CONF_SECOND CLOCK_SECOND
#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn;

#define START_INTERVAL		(1 * CLOCK_SECOND)
#define SEND_INTERVAL (1 * CLOCK_SECOND)
#define TOTAL_PACKAGES 10000
#define Energest_interval 50

#define pause (120 * CLOCK_SECOND)
#define PAYLOAD_LEN	16

static struct simple_udp_connection udp_conn;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
PROCESS(Energest, "Energest");
AUTOSTART_PROCESSES(&udp_client_process,&Energest);
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
  unsigned count = *(unsigned *)data;
  /* If tagging of traffic class is enabled tc will print number of
     transmission - otherwise it will be 0 */
    LOG_INFO("Received response %u (Max MAC Tx: %d) from ", count,
           uipbuf_get_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS));
  LOG_INFO_6ADDR(sender_addr);
LOG_INFO_("\n");
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count=0;
  static int packages_sent =-1;
  //static int payload = 64;
  static uint32_t payload[PAYLOAD_LEN] = {0};
  uip_ipaddr_t dest_ipaddr;
  PROCESS_BEGIN();
 // uint32_t asn_lsb4b =  tsch_current_asn.ls4b;
  //process_start(&energest,NULL);
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer,START_INTERVAL);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  etimer_set(&periodic_timer,SEND_INTERVAL);
  
  
  while(count<TOTAL_PACKAGES) 
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
     {
       if(packages_sent==-1)
         {
		printf("time");
         } 
        /* Set the number of transmissions to use for this packet -
           this can be used to create more reliable transmissions or
           less reliable than the default. Works end-to-end if
           UIP_CONF_TAG_TC_WITH_VARIABLE_RETRANSMISSIONS is set to 1.
         */
           queuebuf_debug_print();
           //uipbuf_set_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS, 0);
           LOG_INFO("Sending request %u (Max MAC Tx: %d) to ", count,uipbuf_get_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS));
           LOG_INFO_6ADDR(&dest_ipaddr);
           LOG_INFO_("\n");
           payload[0]=tsch_current_asn.ls4b;
          simple_udp_sendto(&udp_conn, payload, sizeof(payload), &dest_ipaddr);
         
        
        count++;
        packages_sent++;

      } 
     else 
     {
     //LOG_INFO("Not reachable yet %p\n", rpl_get_any_dag());
     }

    etimer_reset(&periodic_timer);
  }
  etimer_set(&periodic_timer,pause);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  printf("SENDT = %i ",packages_sent);
  LOG_INFO("test");
  LOG_INFO("\n");
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
            printf("Client Energest CPU: %lu LPM: %lu Deep LPM: %lu Total time: %lu seconds\n",
           (unsigned long)(energest_type_time(ENERGEST_TYPE_CPU) / ENERGEST_SECOND),
           (unsigned long)(energest_type_time(ENERGEST_TYPE_LPM) / ENERGEST_SECOND),
           (unsigned long)(energest_type_time(ENERGEST_TYPE_DEEP_LPM) / ENERGEST_SECOND),
           (unsigned long)(ENERGEST_GET_TOTAL_TIME() / ENERGEST_SECOND));
         etimer_reset(&periodic_timer);
 }
 PROCESS_END();
}
/*---------------------------------------------------------------------------*/
