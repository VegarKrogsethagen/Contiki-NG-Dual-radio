#include "contiki.h"
#include "rpl.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/energest.h"



#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#include "os/sys/log.h"
#include "os/sys/log.c"
#define LOG_MODULE "Test"
#define LOG_LEVEL LOG_LEVEL_DBG


#define ENERGEST_CONF_CURRENT_TIME clock_time
#define ENERGEST_CONF_TIME_T clock_time_t
#define ENERGEST_CONF_SECOND CLOCK_SECOND


static struct simple_udp_connection udp_conn;

#define START_INTERVAL		(1 * CLOCK_SECOND)
#define SEND_INTERVAL 200
#define TOTAL_PACKAGES 3000
#define Energest_interval 500

#define pause (120 * CLOCK_SECOND)


static struct simple_udp_connection udp_conn;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
PROCESS(Energest, "Energest");
AUTOSTART_PROCESSES(&udp_client_process);
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
  LOG_INFO("Received response %u (tc:%d) from ", count,
           uipbuf_get_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS));
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count;
  static int packages_sent =-1;

  PROCESS_BEGIN();
  //process_start(&energest,NULL);
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer,START_INTERVAL);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  etimer_set(&periodic_timer,SEND_INTERVAL);
  while(count<=TOTAL_PACKAGES) 
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(rpl_is_reachable())
     {
      /* Send to DAG root */
      rpl_dag_t *dag = rpl_get_any_dag();
      if(dag != NULL) 
        { /* Only a sanity check. Should never be NULL
                          as rpl_is_reachable() is true */
       // LOG_INFO("Sending request %u to ", count);
        //LOG_INFO_6ADDR(&dag->dag_id);
        //LOG_INFO_("\n");
        /* Set the number of transmissions to use for this packet -
           this can be used to create more reliable transmissions or
           less reliable than the default. Works end-to-end if
           UIP_CONF_TAG_TC_WITH_VARIABLE_RETRANSMISSIONS is set to 1.
         */
        
        simple_udp_sendto(&udp_conn, &count, sizeof(count), &dag->dag_id);
        count++;
        packages_sent++;
	
         
      }
    } 
     else {
     //LOG_INFO("Not reachable yet %p\n", rpl_get_any_dag());
    }

    etimer_reset(&periodic_timer);
  }
  etimer_set(&periodic_timer,pause);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
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
