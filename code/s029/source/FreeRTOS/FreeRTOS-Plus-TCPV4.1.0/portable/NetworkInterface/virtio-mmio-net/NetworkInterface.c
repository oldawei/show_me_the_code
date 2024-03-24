/*
 * FreeRTOS+TCP V3.1.0
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/*****************************************************************************
* Note: This file is Not! to be used as is. The purpose of this file is to provide
* a template for writing a network interface. Each network interface will have to provide
* concrete implementations of the functions in this file.
*
* See the following URL for an explanation of this file and its functions:
* https://freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/Embedded_Ethernet_Porting.html
*
*****************************************************************************/

#include "riscv-virt.h"
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "list.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"

int virtio_net_init(u32 base);
int virtio_net_tx(NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t xReleaseAfterSend);
int virtio_net_link_status(void);
void *virtio_net_rx_pkt(long int *p_size);
NetworkInterface_t * pxVIRTIONET_FillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                         NetworkInterface_t * pxInterface );

/* Interrupt events to process.  Currently only the Rx event is processed
 * although code for other events is included to allow for possible future
 * expansion. */
#define VIRTIO_NET_RX_EVENT         1UL
#define VIRTIO_NET_TX_EVENT         2UL
#define VIRTIO_NET_ERR_EVENT        4UL
#define VIRTIO_NET_ALL_EVENT        ( VIRTIO_NET_RX_EVENT | VIRTIO_NET_TX_EVENT | VIRTIO_NET_ERR_EVENT )

 #define ETH_MAX_PACKET_SIZE        ( 1526U + ipBUFFER_PADDING)
 #define VIRTIO_BUFFER_SIZE         ( ( uint32_t ) ( ETH_MAX_PACKET_SIZE - ipBUFFER_PADDING ) )

/* If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 1, then the Ethernet
 * driver will filter incoming packets and only pass the stack those packets it
 * considers need processing. */
#if ( ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0 )
    #define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer )    eProcessBuffer
#else
    #define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer )    eConsiderFrameForProcessing( ( pucEthernetBuffer ) )
#endif

/* Holds the handle of the task used as a deferred interrupt processor.  The
 * handle is used so direct notifications can be sent to the task for all virtio-net
 * related interrupts. */
static TaskHandle_t xVirtioTaskHandle = NULL;
static NetworkInterface_t * pxMyInterface = NULL;

static BaseType_t prvNetworkInterfaceInput( void )
{
    BaseType_t xReceivedLength = 0;
    NetworkBufferDescriptor_t *pxBufferDescriptor = NULL;

    while (1) {
        xReceivedLength = 0;
        pxBufferDescriptor = virtio_net_rx_pkt(&xReceivedLength);
        if (xReceivedLength > 0) {
            pxBufferDescriptor->xDataLength = xReceivedLength;
        } else {
            goto L_ERR;
        }

        if( ipCONSIDER_FRAME_FOR_PROCESSING( pxBufferDescriptor->pucEthernetBuffer ) != eProcessBuffer ) {
            /* The Ethernet frame can be dropped, but the Ethernet buffer must be released. */
            //FreeRTOS_printf( ( "prvPassEthMessages: eProcessBuffer!\n" ) );
            vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );
            continue;
        }

        pxBufferDescriptor->pxInterface = pxMyInterface;
        pxBufferDescriptor->pxEndPoint = FreeRTOS_MatchingEndpoint( pxMyInterface, pxBufferDescriptor->pucEthernetBuffer );

        IPStackEvent_t xRxEvent;
        xRxEvent.eEventType = eNetworkRxEvent;
        xRxEvent.pvData = ( void * ) pxBufferDescriptor;

        if( xSendEventStructToIPTask( &xRxEvent, ( TickType_t ) 1000 ) != pdPASS )
        {
            /* The buffer could not be sent to the stack so must be released again.
            * This is a deferred handler task, not a real interrupt, so it is ok to
            * use the task level function here. */
            #if ( ipconfigUSE_LINKED_RX_MESSAGES != 0 )
                {
                    do
                    {
                        NetworkBufferDescriptor_t * pxNext = pxBufferDescriptor->pxNextBuffer;
                        vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );
                        pxBufferDescriptor = pxNext;
                    } while( pxBufferDescriptor != NULL );
                }
            #else
                {
                    //vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );
                }
            #endif /* ipconfigUSE_LINKED_RX_MESSAGES */
            iptraceETHERNET_RX_EVENT_LOST();
            FreeRTOS_printf( ( "prvPassEthMessages: Can not queue return packet!\n" ) );
            goto L_ERR;
        }
        else
        {
            iptraceNETWORK_INTERFACE_RECEIVE();
        }
    }

    return( xReceivedLength > 0 );

L_ERR:
    if (pxBufferDescriptor)
        vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );
    return 0;
}

static void prvVirtioHandlerTask( void * pvParameters )
{
    uint32_t ulISREvents = 0U;

    /* Remove compiler warnings about unused parameters. */
    ( void ) pvParameters;

    FreeRTOS_printf( ("prvVirtioHandlerTask\n") );
    for( ; ; )
    {
        #if ( ipconfigHAS_PRINTF != 0 )
            {
                /* Call a function that monitors resources: the amount of free network
                 * buffers and the amount of free space on the heap.  See FreeRTOS_IP.c
                 * for more detailed comments. */
                vPrintResourceStats();
            }
        #endif /* ( ipconfigHAS_PRINTF != 0 ) */

        /* Wait for a new event or a time-out. */
        //FreeRTOS_printf( ("xTaskNotifyWait...\n") );
        xTaskNotifyWait( 0U,                    /* ulBitsToClearOnEntry */
                         VIRTIO_NET_ALL_EVENT,  /* ulBitsToClearOnExit */
                         &( ulISREvents ),      /* pulNotificationValue */
                         portMAX_DELAY );
        //FreeRTOS_printf( ("xTaskNotifyWait done\n") );

        if( ( ulISREvents & VIRTIO_NET_RX_EVENT ) != 0 )
        {
            //FreeRTOS_printf( ("prvNetworkInterfaceInput\n") );
            prvNetworkInterfaceInput();
        }

        if( ( ulISREvents & VIRTIO_NET_TX_EVENT ) != 0 )
        {
            /* Code to release TX buffers in case zero-copy is used. */
            /* Check if DMA packets have been delivered. */
            //vClearTXBuffers();
        }

        if( ( ulISREvents & VIRTIO_NET_ERR_EVENT ) != 0 )
        {
            /* Future extension: logging about errors that occurred. */
        }
    }
}
/*-----------------------------------------------------------*/

BaseType_t xVIRTIONET_NetworkInterfaceInitialise( NetworkInterface_t * pxInterface )
{
    pxMyInterface = pxInterface;
    if( xTaskCreate( prvVirtioHandlerTask, "VIRTIO_NET0", ( 2 * configMINIMAL_STACK_SIZE ), NULL, configMAX_PRIORITIES - 1, &xVirtioTaskHandle ) != pdPASS )
    {
        FreeRTOS_printf( ("xVIRTIONET_NetworkInterfaceInitialise failed!\n") );
        return pdFALSE;
    }

	int r = virtio_net_init(VIRTIO2_ADDR);
	printf("r: %d\n", r);

    FreeRTOS_printf( ("xVIRTIONET_NetworkInterfaceInitialise ok!\n") );
    return pdTRUE;
}
/*-----------------------------------------------------------*/

static BaseType_t xVIRTIONET_NetworkInterfaceOutput( NetworkInterface_t * pxInterface,
                                                  NetworkBufferDescriptor_t * const pxDescriptor,
                                                  BaseType_t bReleaseAfterSend )
{
    //FreeRTOS_printf( ("xVIRTIONET_NetworkInterfaceOutput\n") );

    taskENTER_CRITICAL();
    virtio_net_tx(pxDescriptor, bReleaseAfterSend);
    taskEXIT_CRITICAL();

    return pdTRUE;
}
/*-----------------------------------------------------------*/

static BaseType_t xVIRTIONET_GetPhyLinkStatus( NetworkInterface_t * pxInterface )
{
    (void)pxInterface;
    BaseType_t r = virtio_net_link_status();
    FreeRTOS_printf( ("xGetPhyLinkStatus: %d\n", r) );
    return r;
}
/*-----------------------------------------------------------*/

#if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
/* Do not call the following function directly. It is there for downward compatibility.
 * The function FreeRTOS_IPInit() will call it to initialice the interface and end-point
 * objects.  See the description in FreeRTOS_Routing.h. */

NetworkInterface_t * pxFillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                NetworkInterface_t * pxInterface )
{
    return pxVIRTIONET_FillInterfaceDescriptor( xEMACIndex, pxInterface );
}
#endif
/*-----------------------------------------------------------*/

BaseType_t xNetworkInterfaceInitialise( NetworkInterface_t * pxInterface )
{
    return xVIRTIONET_NetworkInterfaceInitialise( pxInterface );
}
/*-----------------------------------------------------------*/

BaseType_t xNetworkInterfaceOutput( NetworkInterface_t * pxInterface,
                                    NetworkBufferDescriptor_t * const pxBuffer,
                                    BaseType_t bReleaseAfterSend )
{
    return xVIRTIONET_NetworkInterfaceOutput( pxInterface, pxBuffer, bReleaseAfterSend );
}
/*-----------------------------------------------------------*/

void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    static uint8_t ucNetworkPackets[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS * ETH_MAX_PACKET_SIZE ] __attribute__( ( aligned( 32 ) ) );
    uint8_t * ucRAMBuffer = ucNetworkPackets;
    uint32_t ul;

    FreeRTOS_printf( ("vNetworkInterfaceAllocateRAMToBuffers\n") );
    for( ul = 0; ul < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; ul++ )
    {
        pxNetworkBuffers[ ul ].pucEthernetBuffer = ucRAMBuffer + ipBUFFER_PADDING;
        *( ( uintptr_t * ) ucRAMBuffer ) = ( uintptr_t ) &( pxNetworkBuffers[ ul ] );
        ucRAMBuffer += ETH_MAX_PACKET_SIZE;
    }
}
/*-----------------------------------------------------------*/

BaseType_t xGetPhyLinkStatus( NetworkInterface_t * pxInterface )
{
    return xVIRTIONET_GetPhyLinkStatus( pxInterface );
}
/*-----------------------------------------------------------*/

void vNetworkInterfaceRxNotify( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Pass an RX-event and wakeup the xVirtioTaskHandle. */
    if( xVirtioTaskHandle != NULL )
    {
        //FreeRTOS_printf( ("vNetworkInterfaceRxNotify...\n") );
        xTaskNotifyFromISR( xVirtioTaskHandle, VIRTIO_NET_RX_EVENT, eSetBits, &( xHigherPriorityTaskWoken ) );
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}
/*-----------------------------------------------------------*/

void vNetworkInterfaceTxNotify( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Pass an RX-event and wakeup the xVirtioTaskHandle. */
    if( xVirtioTaskHandle != NULL )
    {
        xTaskNotifyFromISR( xVirtioTaskHandle, VIRTIO_NET_TX_EVENT, eSetBits, &( xHigherPriorityTaskWoken ) );
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}
/*-----------------------------------------------------------*/

NetworkInterface_t * pxVIRTIONET_FillInterfaceDescriptor( BaseType_t xEMACIndex,
                                                         NetworkInterface_t * pxInterface )
{
    static char pcName[ 17 ];

/* This function pxSTM32Fxx_FillInterfaceDescriptor() adds a network-interface.
 * Make sure that the object pointed to by 'pxInterface'
 * is declared static or global, and that it will remain to exist. */

    snprintf( pcName, sizeof( pcName ), "eth%u", ( unsigned ) xEMACIndex );

    memset( pxInterface, '\0', sizeof( *pxInterface ) );
    pxInterface->pcName = pcName;                    /* Just for logging, debugging. */
    pxInterface->pvArgument = ( void * ) xEMACIndex; /* Has only meaning for the driver functions. */
    pxInterface->pfInitialise = xVIRTIONET_NetworkInterfaceInitialise;
    pxInterface->pfOutput = xVIRTIONET_NetworkInterfaceOutput;
    pxInterface->pfGetPhyLinkStatus = xVIRTIONET_GetPhyLinkStatus;

    FreeRTOS_AddNetworkInterface( pxInterface );

    return pxInterface;
}
/*-----------------------------------------------------------*/