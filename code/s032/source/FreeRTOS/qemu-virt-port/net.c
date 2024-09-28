#include "printf/printf.h"
#include <strings.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"

/* Demo application includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#define mainHOST_NAME                      "qemu_rv32"
#define mainDEVICE_NICK_NAME               "virt"

/* Use by the pseudo random number generator. */
static UBaseType_t ulNextRand;

#define configIP_ADDR0 192
#define configIP_ADDR1 168
#define configIP_ADDR2 3
#define configIP_ADDR3 10

#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 255
#define configNET_MASK3 0

/* The default IP and MAC address used by the demo.  The address configuration
 * defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
 * 1 but a DHCP server could not be contacted.  See the online documentation for
 * more information. */
static const uint8_t ucIPAddress[ 4 ] =
{
    configIP_ADDR0,
    configIP_ADDR1,
    configIP_ADDR2,
    configIP_ADDR3
};
static const uint8_t ucNetMask[ 4 ] =
{
    configNET_MASK0,
    configNET_MASK1,
    configNET_MASK2,
    configNET_MASK3
};
static const uint8_t ucGatewayAddress[ 4 ] =
{
    configIP_ADDR0,
    configIP_ADDR1,
    configIP_ADDR2,
    1
};
static const uint8_t ucDNSServerAddress[ 4 ] =
{
    8,
    8,
    8,
    8
};
const uint8_t ucMACAddress[ 6 ] =
{
    0x02,
    0xca,
    0xfe,
    0xf0,
    0x0d,
    0x01
};

#if TCPIP == 4
static NetworkInterface_t xInterfaces[ 1 ];
static NetworkEndPoint_t xEndPoints[ 1 ];
#endif

UBaseType_t uxRand( void )
{
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

    /* Utility function to generate a pseudo random number. */

    ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
    return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}
/*-----------------------------------------------------------*/

void prvSRand( UBaseType_t ulSeed )
{
    /* Utility function to seed the pseudo random number generator. */
    ulNextRand = ulSeed;
}
/*-----------------------------------------------------------*/

/* Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect
 * events are only received if implemented in the MAC driver. */
#if TCPIP == 4
void vApplicationIPNetworkEventHook_Multi( eIPCallbackEvent_t eNetworkEvent,
                                           struct xNetworkEndPoint * pxEndPoint )
#else
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
#endif
{
    uint32_t ulIPAddress;
    uint32_t ulNetMask;
    uint32_t ulGatewayAddress;
    uint32_t ulDNSServerAddress;
    char cBuffer[ 16 ];

    /* If the network has just come up...*/
    if( eNetworkEvent == eNetworkUp )
    {
        /* Print out the network configuration, which may have come from a DHCP
         * server. */
#if TCPIP == 4
        FreeRTOS_GetEndPointConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress, pxEndPoint );
#else
        FreeRTOS_GetAddressConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress );
#endif
        FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
        FreeRTOS_printf( ( "\r\n\r\nIP Address: %s\r\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
        FreeRTOS_printf( ( "Subnet Mask: %s\r\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
        FreeRTOS_printf( ( "Gateway Address: %s\r\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
        FreeRTOS_printf( ( "DNS Server Address: %s\r\n\r\n\r\n", cBuffer ) );
    }
    else
    {
        FreeRTOS_printf( ( "Application idle hook network down\n" ) );
    }
}
/*-----------------------------------------------------------*/

#if TCPIP == 4
BaseType_t xApplicationDNSQueryHook_Multi( struct xNetworkEndPoint * pxEndPoint,
                                           const char * pcName )
{
    FreeRTOS_printf( ( "xApplicationDNSQueryHook_Multi\n" ) );
    return 0;
}
#endif
/*-----------------------------------------------------------*/

#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) || ( ipconfigDHCP_REGISTER_HOSTNAME == 1 )

    const char * pcApplicationHostnameHook( void )
    {
        /* Assign the name "FreeRTOS" to this network node.  This function will
         * be called during the DHCP: the machine will be registered with an IP
         * address plus this name. */
        return mainHOST_NAME;
    }

#endif
/*-----------------------------------------------------------*/

#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 )

    BaseType_t xApplicationDNSQueryHook( const char * pcName )
    {
        BaseType_t xReturn;

        /* Determine if a name lookup is for this node.  Two names are given
         * to this node: that returned by pcApplicationHostnameHook() and that set
         * by mainDEVICE_NICK_NAME. */
        if( strcasecmp( pcName, pcApplicationHostnameHook() ) == 0 )
        {
            xReturn = pdPASS;
        }
        else if( strcasecmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
        {
            xReturn = pdPASS;
        }
        else
        {
            xReturn = pdFAIL;
        }

        return xReturn;
    }

#endif /* if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) */

/*
 * Callback that provides the inputs necessary to generate a randomized TCP
 * Initial Sequence Number per RFC 6528.  THIS IS ONLY A DUMMY IMPLEMENTATION
 * THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION
 * SYSTEMS.
 */
extern uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                                    uint16_t usSourcePort,
                                                    uint32_t ulDestinationAddress,
                                                    uint16_t usDestinationPort )
{
    ( void ) ulSourceAddress;
    ( void ) usSourcePort;
    ( void ) ulDestinationAddress;
    ( void ) usDestinationPort;

    return uxRand();
}

/*
 * Supply a random number to FreeRTOS+TCP stack.
 * THIS IS ONLY A DUMMY IMPLEMENTATION THAT RETURNS A PSEUDO RANDOM NUMBER
 * SO IS NOT INTENDED FOR USE IN PRODUCTION SYSTEMS.
 */
BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    *( pulNumber ) = uxRand();
    return pdTRUE;
}

void net_init_all(void)
{
    prvSRand(0x1234abcd);

#if TCPIP == 4
NetworkInterface_t * pxVIRTIONET_FillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                         NetworkInterface_t * pxInterface );

    /* Initialise the interface descriptor for WinPCap for example. */
    pxVIRTIONET_FillInterfaceDescriptor( 0, &( xInterfaces[ 0 ] ) );

    FreeRTOS_FillEndPoint( &( xInterfaces[ 0 ] ), &( xEndPoints[ 0 ] ), ucIPAddress,
            ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
    #if ( ipconfigUSE_DHCP != 0 )
    {
        /* End-point 0 wants to use DHCPv4. */
        xEndPoints[ 0 ].bits.bWantDHCP = pdTRUE;
    }
    #endif /* ( ipconfigUSE_DHCP != 0 ) */

    /* Initialise the RTOS's TCP/IP stack.  The tasks that use the network
       are created in the vApplicationIPNetworkEventHook() hook function
       below.  The hook function is called when the network connects. */
     FreeRTOS_IPInit_Multi();
#else
    FreeRTOS_debug_printf( ( "FreeRTOS_IPInit\n" ) );
    FreeRTOS_IPInit( ucIPAddress,
                     ucNetMask,
                     ucGatewayAddress,
                     ucDNSServerAddress,
                     ucMACAddress );
#endif
}

#if TCPIP == 4 && ipconfigUSE_DHCP_HOOK
#include "FreeRTOS_DHCP.h"
eDHCPCallbackAnswer_t xApplicationDHCPHook_Multi( eDHCPCallbackPhase_t eDHCPPhase,
                                                          struct xNetworkEndPoint * pxEndPoint,
                                                          IP_Address_t * pxIPAddress )
{
    eDHCPCallbackAnswer_t eReturn;
    uint32_t ulStaticIPAddress, ulStaticNetMask;
    uint32_t ulIPAddress = 0;

    if (pxIPAddress) {
        ulIPAddress = pxIPAddress->ulIP_IPv4;
    }

    if (ulIPAddress) {
        char pcBuf[16];
        FreeRTOS_inet_ntoa( ulIPAddress, pcBuf );
        FreeRTOS_printf( ( "xApplicationDHCPHook_Multi: %s\n", pcBuf ) );
    } else {
        FreeRTOS_printf( ( "xApplicationDHCPHook_Multi: 0x%08x\n", ulIPAddress ) );
    }
    FreeRTOS_printf( ( "eDHCPPhase: %d\n", eDHCPPhase ) );

  /* This hook is called in a couple of places during the DHCP process, as
     identified by the eDHCPPhase parameter. */
  switch( eDHCPPhase )
  {
    case eDHCPPhasePreDiscover  :
      /* A DHCP discovery is about to be sent out. eDHCPContinue is
         returned to allow the discovery to go out.

         If eDHCPUseDefaults had been returned instead then the DHCP process
         would be stopped and the statically configured IP address would be
         used.

         If eDHCPStopNoChanges had been returned instead then the DHCP
         process would be stopped and whatever the current network
         configuration was would continue to be used. */
      eReturn = eDHCPContinue;
      break;

    case eDHCPPhasePreRequest  :
      /* An offer has been received from the DHCP server, and the offered
         IP address is passed in the ulIPAddress parameter. Convert the
         offered and statically allocated IP addresses to 32-bit values. */
      ulStaticIPAddress = FreeRTOS_inet_addr_quick( configIP_ADDR0,
                                                    configIP_ADDR1,
                                                    configIP_ADDR2,
                                                    configIP_ADDR3 );

      ulStaticNetMask = FreeRTOS_inet_addr_quick( configNET_MASK0,
                                                  configNET_MASK1,
                                                  configNET_MASK2,
                                                  configNET_MASK3 );

      /* Mask the IP addresses to leave just the sub-domain octets. */
      ulStaticIPAddress &= ulStaticNetMask;
      ulIPAddress &= ulStaticNetMask;

      /* Are the sub-domains the same? */
      if( ulStaticIPAddress == ulIPAddress )
      {
        /* The sub-domains match, so the default IP address can be
           used. The DHCP process is stopped at this point. */
        eReturn = eDHCPUseDefaults;
      }
      else
      {
        /* The sub-domains don't match, so continue with the DHCP
           process so the offered IP address is used. */
        eReturn = eDHCPContinue;
      }

      break;

    default :
      /* Cannot be reached, but set eReturn to prevent compiler warnings
         where compilers are disposed to generating one. */
      eReturn = eDHCPContinue;
      break;
  }
  FreeRTOS_printf( ( "eReturn: %d\n", eReturn ) );

  return eReturn;
}
#endif

void iptraceethernet_rx_event_lost(void)
{
    static u32 rx_lost_cnt = 0;
    ++rx_lost_cnt;

    FreeRTOS_printf( ( "rx_lost_cnt: %d\n", rx_lost_cnt ) );
}

void iptracenetwork_interface_receive(void)
{
    static u32 rx_recv_cnt = 0;
    static u32 step = 1000;
    ++rx_recv_cnt;

    if (rx_recv_cnt >= step) {
        step += 1000;
        FreeRTOS_printf( ( "rx_recv_cnt: %d\n", rx_recv_cnt ) );
    }
}

