#ifdef NETSTACK_CONF_MAC
#undef NETSTACK_CONF_MAC
#endif
#define NETSTACK_CONF_MAC csma_driver 
//#define NETSTACK_CONF_MAC nullmac_driver

#ifdef NETSTACK_CONF_RDC
#undef NETSTACK_CONF_RDC
#endif
#define NETSTACK_CONF_RDC nullrdc_driver
//#define NETSTACK_CONF_RDC contikimac_driver

#ifdef NETSTACK_CONF_FRAMER
#undef NETSTACK_CONF_FRAMER
#endif
#define NETSTACK_CONF_FRAMER framer_802154
//#define NETSTACK_CONF_FRAMER framer_nullmac

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 2
