/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"

#if 1 /* don't build, this is only a skeleton, see previous comment */

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
// #include "netif/ppp_oe.h"
#include "lpc177x_8x_emac.h"
#include "stdint.h"
#include "string.h"
#include "ucos_ii.h"
#include <lwip/sys.h>
#include <phylan_dm9161a.h>

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

static sys_sem_t sem_recv = {NULL};

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

/* Forward declarations. */
static void  ethernetif_input(struct netif *netif);

static OS_STK App_TaskEthernetInputStk[APP_CFG_TASK_ETHERNET_INPUT_STK_SIZE];
/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  struct ethernetif *ethernetif = netif->state;
  
  (void)ethernetif;
  
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] = 0x11;
  netif->hwaddr[1] = 0x22;
  netif->hwaddr[2] = 0x33;
  netif->hwaddr[3] = 0x44;
  netif->hwaddr[4] = 0x55;
  netif->hwaddr[5] = 0x66;

  /* maximum transfer unit */
  netif->mtu = 1500;
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
 
  /* Do whatever else is needed to initialize interface. */

  sys_sem_new(&sem_recv, 0);
  
  OSTaskCreateExt((void (*)(void *))ethernetif_input,            /* Create task                                          */
                  (void          *)netif,
                  (OS_STK        *)&App_TaskEthernetInputStk[APP_CFG_TASK_ETHERNET_INPUT_STK_SIZE - 1],
                  (INT8U          )APP_CFG_TASK_ETHERNET_INPUT_PRIO,
                  (INT16U         )APP_CFG_TASK_ETHERNET_INPUT_PRIO,
                  (OS_STK        *)&App_TaskEthernetInputStk[0],
                  (INT32U         )APP_CFG_TASK_ETHERNET_INPUT_STK_SIZE,
                  (void          *)0,
                  (INT16U         )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q;
  uint32_t Index, IndexNext;
  uint8_t *ptr;

  /* calculate next index */
  IndexNext = LPC_EMAC->TxProduceIndex + 1;
  if(IndexNext > LPC_EMAC->TxDescriptorNumber)
    IndexNext = 0;
#if 0
	/* check whether block is full */
	while (IndexNext == LPC_EMAC->TxConsumeIndex)
	{
		err_t result;
		uint32_t recved;

		/* there is no block yet, wait a flag */
		result = rt_event_recv(&tx_event, 0x01,
			RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &recved);
	}
#endif
  
	Index = LPC_EMAC->TxProduceIndex;

	/* calculate next index */
	IndexNext = LPC_EMAC->TxProduceIndex + 1;
	if(IndexNext > LPC_EMAC->TxDescriptorNumber)
		IndexNext = 0;

#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

	/* copy data to tx buffer */
	ptr = (uint8_t*)TX_BUF(Index);
  for(q = p; q != NULL; q = q->next) {
		MEMCPY(ptr, q->payload, q->len);
		ptr += q->len;
  }

	TX_DESC_CTRL(Index) = (p->tot_len - 1) & 0x7ff | EMAC_TCTRL_INT | EMAC_TCTRL_LAST;

	/* change index to the next */
	LPC_EMAC->TxProduceIndex = IndexNext;

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
  
  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
  struct pbuf *p, *q;
  u16_t len;

  if(EMAC_GetBufferSts(EMAC_RX_BUFF) == EMAC_BUFF_EMPTY)
    return NULL;
    
  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = EMAC_GetRxFrameSize();

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
  if (p != NULL) {

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
    uint8_t *ptr = (uint8_t *)EMAC_GetRxBuffer();
    for(q = p; q != NULL; q = q->next) {
      MEMCPY(q->payload, ptr, q->len);
      ptr += q->len;
    }
    EMAC_UpdateRxConsumeIndex();

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
    
  } else {
//    drop packet();
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
  }
  
  return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input(struct netif *netif)
{
  struct eth_hdr *ethhdr;
  struct pbuf *p;

	EMAC_IntCmd(EMAC_INT_RX_DONE, ENABLE);

  for (;;)
  {
    sys_sem_wait(&sem_recv);

    /* no packet could be read, silently ignore this */
    while ((p = low_level_input(netif)) != NULL)
    {
      /* points to packet payload, which starts with an Ethernet header */
      ethhdr = p->payload;

      switch (htons(ethhdr->type)) {
      /* IP or ARP packet? */
      case ETHTYPE_IP:
      case ETHTYPE_ARP:
    #if PPPOE_SUPPORT
      /* PPPoE packet? */
      case ETHTYPE_PPPOEDISC:
      case ETHTYPE_PPPOE:
    #endif /* PPPOE_SUPPORT */
        /* full packet send to tcpip_thread to process */
        if (netif->input(p, netif)!=ERR_OK)
         { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
           pbuf_free(p);
           p = NULL;
         }
        break;

      default:
        pbuf_free(p);
        p = NULL;
        break;
      }
    }
    
    EMAC_IntCmd(EMAC_INT_RX_DONE, ENABLE);
  }
}

void ethernetif_frame_ready()
{
  EMAC_IntCmd(EMAC_INT_RX_DONE, DISABLE);
	if (sys_sem_valid(&sem_recv))
		sys_sem_signal(&sem_recv);
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
    
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, /*LINK_SPEED_OF_YOUR_NETIF_IN_BPS*/ 104857600);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

#endif /* 0 */
