
#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8

/* Energy log */
#define RPL_ICMP_ENERGY_LOG		0

#define TRAFFIC_MODEL 0 // 0: Periodic, 1: Poisson
#if TRAFFIC_MODEL == 0
#define PERIOD 20
#elif TRAFFIC_MODEL == 1
#define ARRIVAL_RATE 0 // Mean value, 1/lambda
#endif

uint8_t dead;

/*-----------------------------------------------------------------------------------------------*/
#define DETERMINED_ROUTING_TREE	0

#if DETERMINED_ROUTING_TREE
#define MAX_NODE_NUMBER 30

#endif /* ROUTING_TREE */
