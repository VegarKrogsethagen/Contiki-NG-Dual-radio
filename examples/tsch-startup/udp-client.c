#include "contiki.h"
#include "os/net/routing/nullrouting/nullrouting.c"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/packetbuf.c"
#include "os/net/queuebuf.c"
#include "os/sys/clock.h"
#include "os/sys/log.h"
#include "os/sys/log.c"
#include "os/net/mac/tsch/tsch.h"
#include "os/net/mac/tsch/tsch-slot-operation.c"
#include "os/net/mac/tsch/tsch-queue.h"

#define LOG_MODULE "Test"
#define LOG_LEVEL LOG_LEVEL_DBG

#define StartUpPause 1
#define pause (120 * CLOCK_SECOND)
#define PAYLOAD_LEN	16
#define bootup (60 * CLOCK_SECOND)


/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  PROCESS_BEGIN();
  NETSTACK_MAC.on();

  etimer_set(&periodic_timer,StartUpPause);
  while(1) 
  {
    
    
    if(tsch_is_associated==1)
     {
       
       printf("Time = %d \n", clock_seconds());
       
      } 
         PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
         etimer_reset(&periodic_timer);

  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
