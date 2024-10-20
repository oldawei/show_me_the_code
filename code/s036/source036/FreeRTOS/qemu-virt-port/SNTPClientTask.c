#include "core_sntp_config.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* SNTP library include. */
#include "core_sntp_client.h"

/* Synchronization primitive include. */
#include "semphr.h"

/* FreeRTOS+TCP includes */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_Sockets.h"

/* Backoff Algorithm include. */
#include "backoff_algorithm.h"


#define xPlatformIsNetworkUp FreeRTOS_IsNetworkUp

/* Compile time error for undefined configs. */

#ifndef democonfigLIST_OF_TIME_SERVERS
    #define democonfigLIST_OF_TIME_SERVERS "ntp.aliyun.com", "ntp1.aliyun.com", "ntp2.aliyun.com"
#endif

#ifndef democonfigSNTP_CLIENT_POLLING_INTERVAL_SECONDS
    #define democonfigSNTP_CLIENT_POLLING_INTERVAL_SECONDS ( 16 )
#endif

#ifndef democonfigSYSTEM_START_YEAR
    #define democonfigSYSTEM_START_YEAR ( 2021 )
#endif

/*-----------------------------------------------------------*/
/* Default values for timeout configurations . */

#ifndef democonfigSERVER_RESPONSE_TIMEOUT_MS
    #define democonfigSERVER_RESPONSE_TIMEOUT_MS    ( 5000 )
#endif

#ifndef democonfigSEND_TIME_REQUEST_TIMEOUT_MS
    #define democonfigSEND_TIME_REQUEST_TIMEOUT_MS    ( 50 )
#endif

#ifndef democonfigRECEIVE_SERVER_RESPONSE_BLOCK_TIME_MS
    #define democonfigRECEIVE_SERVER_RESPONSE_BLOCK_TIME_MS    ( 200 )
#endif

/**
 * @brief The size for network buffer that is allocated for initializing the coreSNTP library in the
 * demo.
 *
 * @note The size of the buffer MUST be large enough to hold an entire SNTP packet, which includes the standard SNTP
 * packet data of 48 bytes and authentication data for security mechanism, if used, in communication with time server.
 */
#define SNTP_CONTEXT_NETWORK_BUFFER_SIZE        ( SNTP_PACKET_BASE_SIZE )

/**
 * @brief The constant for storing the number of milliseconds per FreeRTOS tick in the system.
 * @note This value represents the time duration per tick from the perspective of the
 * of Windows Simulator based FreeRTOS system that carries lagging clock drift in relation to
 * internet time or UTC time. Thus, the actual time duration value per tick of the system will be
 * larger from the perspective of internet time.
 */
#define MILLISECONDS_PER_TICK                   ( 1000 / configTICK_RATE_HZ )

/*-----------------------------------------------------------*/

/**
 * @brief The maximum poll period that the SNTP client can use as back-off on receiving a rejection from a time server.
 *
 * @note This demo performs back-off in polling rate from time server ONLY for the case when a single time server being
 * is configured through the democonfigLIST_OF_TIME_SERVERS macro.
 * This is because when more than one time server is configured, the coreSNTP library automatically handles the case
 * of server rejection of time request by rotating to the next configured server for subsequent time polling requests.
 */
#define SNTP_DEMO_POLL_MAX_BACKOFF_DELAY_SEC    UINT16_MAX

/**
 * @brief The maximum number of times of retrying time requests at exponentially backed-off polling frequency
 * from a server that rejects time requests.
 *
 * @note This macro is only relevant for the case when a single time server is configured in
 * the demo through, democonfigLIST_OF_TIME_SERVERS.
 */
#define SNTP_DEMO_MAX_SERVER_BACKOFF_RETRIES    10

/*-----------------------------------------------------------*/

/**
 * @brief The definition of the @ref NetworkContext_t structure for the demo.
 * The structure wraps a FreeRTOS+TCP socket that is used for UDP communication
 * with time servers.
 *
 * @note The context is used in the @ref UdpTransportInterface_t interface required
 * by the coreSNTP library.
 */
struct NetworkContext
{
    Socket_t socket;
};

/**
 * @brief Structure aggregating state variables for RAM-based wall-clock time
 * in Coordinated Universal Time (UTC) for system.
 *
 * @note This demo uses the following mathematical model to represent current
 * time in RAM.
 *
 *  BaseTime = Time set at boot or the last synchronized time
 *  Slew Rate = Number of milliseconds to adjust per system time second
 *  No. of ticks since last SNTP sync = Current FreeRTOS Tick Count -
 *                                      Tick count at last SNTP sync
 *
 *  Time Elapsed since last SNTP sync = No. of ticks since last SNTP sync
 *                                                    x
 *                                      Number of milliseconds per FreeRTOS tick
 *
 *  Slew Adjustment = Slew Rate x Time Elapsed since last SNTP sync
 *
 *  Current Time = Base Time +
 *                 Time Elapsed since last SNTP sync +
 *                 Slew Adjustment
 */
typedef struct SystemClock
{
    UTCTime_t baseTime;
    TickType_t lastSyncTickCount;
    uint32_t pollPeriod;
    uint64_t slewRate; /* Milliseconds/Seconds */
    bool firstTimeSyncDone;
} SystemClock_t;

/**
 * @brief Shared global system clock object for representing UTC/wall-clock
 * time in system.
 */
static SystemClock_t systemClock;

/**
 * @brief Mutex for protecting access to the shared memory of the
 * system clock parameters.
 */
static SemaphoreHandle_t xMutex = NULL;

/*
 * @brief Stores the configured time servers in an array.
 */
static const char * pTimeServers[] = { democonfigLIST_OF_TIME_SERVERS };
const size_t numOfServers = sizeof( pTimeServers ) / sizeof( char * );

/**
 * @brief Utility function to convert the passed year to UNIX time representation
 * of seconds since 1st Jan 1970 00h:00m:00s seconds to 1st Jan 00h:00m:00s of the
 * the passed year.
 *
 * This utility does account for leap years.
 *
 * @param[in] The year to translate.
 */
static uint32_t translateYearToUnixSeconds( uint16_t year );

/**
 * @brief Calculates the current time in the system.
 * It calculates the current time as:
 *
 *   BaseTime = Time set at device boot or the last synchronized time
 *   SlewRate = Number of milliseconds to adjust per system time second
 *
 *   Current Time = Base Time +
 *                  Time since last SNTP Synchronization +
 *                  Slew Adjustment (if slew rate > 0) for time period since
 *                  last SNTP synchronization
 *
 * @param[in] pBaseTime The base time in the system clock parameters.
 * @param[in] lastSyncTickCount The tick count at the last time synchronization
 * with a time server.
 * @param[in] slewRate The slew rate as seconds of clock adjustment per FreeRTOS
 * system time second.
 * @param[out] pCurrentTime This will be populated with the calculated current
 * UTC time in the system.
 */
static void calculateCurrentTime( UTCTime_t * pBaseTime,
                                  TickType_t lastSyncTickCount,
                                  uint64_t slewRate,
                                  UTCTime_t * pCurrentTime );

/**
 * @brief Initializes the SNTP context for the SNTP client task.
 * This function generates an array of the configured time servers, creates a FreeRTOS UDP socket
 * for the UDP transport interface and initializes the passed SNTP context by calling the
 * Sntp_Init() API of the coreSNTP library.
 *
 * @param[in, out] pContext The memory for the SNTP client context that will be initialized with
 * Sntp_Init API.
 * @param[in] pTimeServers The list of time servers configured through the democonfigLIST_OF_TIME_SERVERS
 * macro in demo_config.h.
 * @param[in] numOfServers The number of time servers configured in democonfigLIST_OF_TIME_SERVERS.
 * @param[in] pContextBuffer The allocated network buffer that will be initialized in the SNTP context.
 * @param[in] pUdpContext The memory for the network context for the UDP transport interface that will
 * be passed to the SNTP client context. This will be filled with a UDP context created by this function.
 *
 * @return Returns `true` if initialization of SNTP client context is successful; otherwise `false`.
 */
static bool initializeSntpClient( SntpContext_t * pContext,
                                  const char ** pTimeServers,
                                  size_t numOfServers,
                                  uint8_t * pContextBuffer,
                                  size_t contextBufferSize,
                                  NetworkContext_t * pUdpContext,
                                  SntpAuthContext_t * pAuthContext );

/**
 * @brief The demo implementation of the @ref SntpResolveDns_t interface to
 * allow the coreSNTP library to resolve DNS name of a time server being
 * used for requesting time from.
 *
 * @param[in] pTimeServer The time-server whose IPv4 address is to be resolved.
 * @param[out] pIpV4Addr This is filled with the resolved IPv4 address of
 * @p pTimeServer.
 */
static bool resolveDns( const SntpServerInfo_t * pServerAddr,
                        uint32_t * pIpV4Addr );

/**
 * @brief The demo implementation of the @ref UdpTransportSendTo_t function
 * of the UDP transport interface to allow the coreSNTP library to perform
 * network operation of sending time request over UDP to the provided time server.
 *
 * @param[in] pNetworkContext This will be the NetworkContext_t context object
 * representing the FreeRTOS UDP socket to use for network send operation.
 * @param[in] serverAddr The IPv4 address of the time server.
 * @param[in] serverPort The port of the server to send data to.
 * @param[in] pBuffer The demo-supplied network buffer of size, SNTP_CONTEXT_NETWORK_BUFFER_SIZE,
 * containing the data to send over the network.
 * @param[in] bytesToSend The size of data in @p pBuffer to send.
 *
 * @return Returns the return code of FreeRTOS UDP send API, FreeRTOS_sendto, which returns
 * 0 for error or timeout OR the number of bytes sent over the network.
 */
static int32_t UdpTransport_Send( NetworkContext_t * pNetworkContext,
                                  uint32_t serverAddr,
                                  uint16_t serverPort,
                                  const void * pBuffer,
                                  uint16_t bytesToSend );

/**
 * @brief The demo implementation of the @ref UdpTransportRecvFrom_t function
 * of the UDP transport interface to allow the coreSNTP library to perform
 * network operation of reading expected time response over UDP from
 * provided time server.
 *
 * @param[in] pNetworkContext This will be the NetworkContext_t context object
 * representing the FreeRTOS UDP socket to use for network read operation.
 * @param[in] pTimeServer The IPv4 address of the time server to receive data from.
 * @param[in] serverPort The port of the server to receive data from.
 * @param[out] pBuffer The demo-supplied network buffer of size, SNTP_CONTEXT_NETWORK_BUFFER_SIZE,
 * that will be filled with data received from the network.
 * @param[in] bytesToRecv The expected number of bytes to receive from the network
 * for the server response server.
 *
 * @return Returns one of the following:
 * - 0 for timeout in receiving any data from the network (by translating the
 * -pdFREERTOS_ERRNO_EWOULDBLOCK return code from FreeRTOS_recvfrom API )
 *                         OR
 * - The number of bytes read from the network.
 */
static int32_t UdpTransport_Recv( NetworkContext_t * pNetworkContext,
                                  uint32_t serverAddr,
                                  uint16_t serverPort,
                                  void * pBuffer,
                                  uint16_t bytesToRecv );

/**
 * @brief The demo implementation of the @ref SntpGetTime_t interface
 * for obtaining system clock time for the coreSNTP library.
 *
 * @param[out] pTime This will be populated with the current time from
 * the system.
 */
static void sntpClient_GetTime( SntpTimestamp_t * pCurrentTime );

/**
 * @brief The demo implementation of the @ref SntpSetTime_t interface
 * for correcting the system clock time based on the  time received
 * from the server response and the clock-offset value calculated by
 * the coreSNTP library.
 *
 * @note This demo uses a combination of "step" AND "slew" methodology
 * for system clock correction.
 * 1. "Step" correction is ALWAYS used to immediately correct the system clock
 *    to match server time on every successful time synchronization with a
 *    time server (that occurs periodically on the poll interval gaps).
 *
 * 2. "Slew" correction approach is used for compensating system clock drift
 *    during the poll interval period between time synchronization attempts with
 *    time server(s) when latest time server is not known. The "slew rate" is
 *    calculated ONLY once on the occasion of the second successful time
 *    synchronization with a time server. This is because the demo initializes
 *    system time with (the first second of) the democonfigSYSTEM_START_YEAR
 *    configuration, and thus, the the actual system clock drift over a period
 *    of time can be calculated only AFTER the demo system time has been synchronized
 *    with server time once. Thus, after the first time period of poll interval has
 *    transpired, the system clock drift is calculated correctly on the subsequent
 *    successful time synchronization with a time server.
 *
 * @note The above system clock correction algorithm is just one example of a correction
 * approach. It can be modified to suit your application needs. For example, your
 * application can use ONLY the "step" correction methodology for simplicity of system clock
 * time calculation logic if the application is not sensitive to abrupt time changes
 * (that occur at the instances of periodic time synchronization attempts). In such a case,
 * the Sntp_CalculatePollInterval() API of coreSNTP library can be used to calculate
 * the optimum time polling period for your application based on the factors of your
 * system's clock drift rate and the maximum clock drift tolerable by your application.
 *
 *
 * @param[in] pTimeServer The time server from whom the time has been received.
 * @param[in] pServerTime The most recent time of the server, @p pTimeServer, sent in its
 * time response.
 * @param[in] clockOffsetMs The value, in milliseconds, of system clock offset relative
 * to the server time calculated by the coreSNTP library. If the value is positive, then
 * the system is BEHIND the server time, and a "slew" clock correction approach is used in
 * this demo. If the value is negative, then the system time is AHEAD of the server time,
 * and a "step" clock correction approach is used in this demo.
 * @param[in] leapSecondInfo This indicates whether there is an upcoming leap second insertion
 * or deletion (according to astronomical time) the last minute of the end of the month that the
 * system time needs to adjust for. Leap second adjustment is valuable for applications that
 * require non-abrupt increment of time for use cases like logging. This demo DOES NOT showcase
 * leap second adjustment in system clock.
 */
static void sntpClient_SetTime( const SntpServerInfo_t * pTimeServer,
                                const SntpTimestamp_t * pServerTime,
                                int64_t clockOffsetMs,
                                SntpLeapSecondInfo_t leapSecondInfo );

/**
 * @brief Generates a random number using PKCS#11.
 *
 * @note It is RECOMMENDED to generate a random number for the call to Sntp_SendTimeRequest API
 * of coreSNTP library to protect against server response spoofing attacks from "network off-path"
 * attackers.
 *
 * @return The generated random number.
 */
static uint32_t generateRandomNumber();

/**
 * @brief Utility to create a new FreeRTOS UDP socket and bind a random
 * port to it.
 * A random port is used for the created UDP socket as a protection mechanism
 * against spoofing attacks from malicious actors that are off the network
 * path of the client-server communication.
 *
 * @param[out] This will be populated with a new FreeRTOS UDP socket
 * that is bound to a random port.
 *
 * @return Returns #true for successful creation of UDP socket; #false
 * otherwise for failure.
 */
static bool createUdpSocket( Socket_t * pSocket );

/**
 * @brief Utility to close the passed FreeRTOS UDP socket.
 *
 * @param pSocket The UDP socket to close.
 */
static void closeUdpSocket( Socket_t * pSocket );

/**
 * @brief Utility to calculate new poll period with exponential backoff and jitter
 * algorithm.
 *
 * @note The demo applies time polling frequency backoff only when a single time server
 * is configured, through the democonfigLIST_OF_SERVERS macro, and the single server
 * rejects time requests.
 *
 * @param[in, out] pContext The context representing the back-off parameters. This
 * context is initialized by the function whenever the caller indicates it with the
 * @p shouldInitializeContext flag.
 * @param[in] shouldInitializeContext Flag to indicate if the passed context should be
 * initialized to start a new sequence of backed-off time request retries.
 * @param[in] minPollPeriod The minimum poll period
 * @param[in] pPollPeriod The new calculated poll period.
 *
 * @return Return #true if a new poll interval is calculated to retry time request
 * from the server; #false otherwise to indicate exhaustion of time request retry attempts
 * with the server.
 */
static bool calculateBackoffForNextPoll( BackoffAlgorithmContext_t * pContext,
                                         bool shouldInitializeContext,
                                         uint32_t minPollPeriod,
                                         uint32_t * pPollPeriod );

/*------------------------------------------------------------------------------*/

static uint32_t translateYearToUnixSeconds( uint16_t year )
{
    configASSERT( year >= 1970 );

    uint32_t numOfDaysSince1970 = ( year - 1970 ) * 365;

    /* Calculate the extra days in leap years (for February 29) over the time
    * period from 1st Jan 1970 to 1st Jan of the passed year.
    * By subtracting from the year 1969, the extra day in 1972 is covered. */
    numOfDaysSince1970 += ( ( year - 1969 ) / 4 );

    return( numOfDaysSince1970 * 24 * 3600 );
}

void calculateCurrentTime( UTCTime_t * pBaseTime,
                           TickType_t lastSyncTickCount,
                           uint64_t slewRate,
                           UTCTime_t * pCurrentTime )
{
    uint64_t msElapsedSinceLastSync = 0;
    uint64_t currentTimeSecs;
    TickType_t ticksElapsedSinceLastSync = xTaskGetTickCount() - lastSyncTickCount;

    /* Calculate time elapsed since last synchronization according to the number
     * of system ticks passed. */
    msElapsedSinceLastSync = ticksElapsedSinceLastSync * MILLISECONDS_PER_TICK;

    /* If slew rate is set, then apply the slew-based clock adjustment for the elapsed time. */
    if( slewRate > 0 )
    {
        /* Slew Adjustment = Slew Rate ( Milliseconds/seconds )
         *                                      x
         *                   No. of seconds since last synchronization. */
        msElapsedSinceLastSync += slewRate * ( msElapsedSinceLastSync / 1000 );
    }

    /* Set the current UTC time in the output parameter. */
    if( msElapsedSinceLastSync >= 1000 )
    {
        currentTimeSecs = ( uint64_t ) ( pBaseTime->secs ) + ( msElapsedSinceLastSync / 1000 );

        /* Support case of UTC timestamp rollover on 7 February 2038. */
        if( currentTimeSecs > UINT32_MAX )
        {
            /* Assert when the UTC timestamp rollover. */
            configASSERT( !( currentTimeSecs > UINT32_MAX ) );

            /* Subtract an extra second as timestamp 0 represents the epoch for
             * UTC era 1. */
            LogWarn( ( "UTC timestamp rollover." ) );
            pCurrentTime->secs = ( uint32_t ) ( currentTimeSecs - UINT32_MAX - 1 );
        }
        else
        {
            pCurrentTime->secs = ( uint32_t ) ( currentTimeSecs );
        }

        pCurrentTime->msecs = msElapsedSinceLastSync % 1000;
    }
    else
    {
        pCurrentTime->secs = pBaseTime->secs;
        pCurrentTime->msecs = ( uint32_t ) ( msElapsedSinceLastSync );
    }
}

/********************** DNS Resolution Interface *******************************/
static bool resolveDns( const SntpServerInfo_t * pServerAddr,
                        uint32_t * pIpV4Addr )
{
    uint32_t resolvedAddr = 0;
    bool status = false;

L1:
    resolvedAddr = FreeRTOS_gethostbyname( pServerAddr->pServerName );

    /* Set the output parameter if DNS look up succeeded. */
    if( resolvedAddr != 0 )
    {
        /* DNS Look up succeeded. */
        status = true;

        *pIpV4Addr = FreeRTOS_ntohl( resolvedAddr );

        #if defined( LIBRARY_LOG_LEVEL ) && ( LIBRARY_LOG_LEVEL != LOG_NONE )
            uint8_t stringAddr[ 16 ];
            FreeRTOS_inet_ntoa( resolvedAddr, stringAddr );
            LogInfo( ( "Resolved time server as %s", stringAddr ) );
        #endif
    } else {
        LogInfo( ( "try to resolve %s", pServerAddr->pServerName ) );
        vTaskDelay( pdMS_TO_TICKS( 1000U ) );
        goto L1;
    }

    return status;
}

/********************** UDP Interface definition *******************************/
int32_t UdpTransport_Send( NetworkContext_t * pNetworkContext,
                           uint32_t serverAddr,
                           uint16_t serverPort,
                           const void * pBuffer,
                           uint16_t bytesToSend )
{
    struct freertos_sockaddr destinationAddress;
    int32_t bytesSent;

    #if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )
    {
        destinationAddress.sin_address.ulIP_IPv4 = FreeRTOS_htonl( serverAddr );
    }
    #else
    {
        destinationAddress.sin_addr = FreeRTOS_htonl( serverAddr );
    }
    #endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */

    destinationAddress.sin_port = FreeRTOS_htons( serverPort );
    destinationAddress.sin_family = FREERTOS_AF_INET;

    /* Send the buffer with ulFlags set to 0, so the FREERTOS_ZERO_COPY bit
     * is clear. */
    bytesSent = FreeRTOS_sendto( /* The socket being send to. */
        pNetworkContext->socket,
        /* The data being sent. */
        pBuffer,
        /* The length of the data being sent. */
        bytesToSend,
        /* ulFlags with the FREERTOS_ZERO_COPY bit clear. */
        0,
        /* Where the data is being sent. */
        &destinationAddress,
        /* Not used but should be set as shown. */
        sizeof( destinationAddress )
        );

    return bytesSent;
}

static int32_t UdpTransport_Recv( NetworkContext_t * pNetworkContext,
                                  uint32_t serverAddr,
                                  uint16_t serverPort,
                                  void * pBuffer,
                                  uint16_t bytesToRecv )
{
    struct freertos_sockaddr sourceAddress;
    int32_t bytesReceived;
    socklen_t addressLength = sizeof( struct freertos_sockaddr );

    /* Receive into the buffer with ulFlags set to 0, so the FREERTOS_ZERO_COPY bit
     * is clear. */
    bytesReceived = FreeRTOS_recvfrom( /* The socket data is being received on. */
        pNetworkContext->socket,

        /* The buffer into which received data will be
         * copied. */
        pBuffer,

        /* The length of the buffer into which data will be
         * copied. */
        bytesToRecv,
        /* ulFlags with the FREERTOS_ZERO_COPY bit clear. */
        0,
        /* Will get set to the source of the received data. */
        &sourceAddress,
        /* Not used but should be set as shown. */
        &addressLength
        );

    /* If data is received from the network, discard the data if  received from a different source than
     * the server. */
    #if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )
        if( ( bytesReceived > 0 ) && ( ( FreeRTOS_ntohl( sourceAddress.sin_address.ulIP_IPv4 ) != serverAddr ) ||
                                       ( FreeRTOS_ntohs( sourceAddress.sin_port ) != serverPort ) ) )
    #else
        if( ( bytesReceived > 0 ) && ( ( FreeRTOS_ntohl( sourceAddress.sin_addr ) != serverAddr ) ||
                                       ( FreeRTOS_ntohs( sourceAddress.sin_port ) != serverPort ) ) )
    #endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */
    {
        bytesReceived = 0;

        #if defined( LIBRARY_LOG_LEVEL ) && ( LIBRARY_LOG_LEVEL != LOG_NONE )
            /* Convert the IP address of the sender's address to string for logging. */
            char stringAddr[ 16 ];

        #if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )
            {
                FreeRTOS_inet_ntoa( sourceAddress.sin_address.ulIP_IPv4, stringAddr );
            }
        #else
            {
                FreeRTOS_inet_ntoa( sourceAddress.sin_addr, stringAddr );
            }
        #endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */

        /* Log about reception of packet from unexpected sender. */
        LogWarn( ( "Received UDP packet from unexpected source: Addr=%s Port=%u",
                   stringAddr, FreeRTOS_ntohs( sourceAddress.sin_port ) ) );
        #endif /* if defined( LIBRARY_LOG_LEVEL ) && ( LIBRARY_LOG_LEVEL != LOG_NONE ) */
    }

    /* Translate the return code of timeout to the UDP transport interface expected
     * code to indicate read retry. */
    else if( bytesReceived == -pdFREERTOS_ERRNO_EWOULDBLOCK )
    {
        bytesReceived = 0;
    }

    return bytesReceived;
}


/**************************** Time Interfaces ************************************************/
static void sntpClient_GetTime( SntpTimestamp_t * pCurrentTime )
{
    UTCTime_t currentTime;
    uint32_t ntpSecs;

    /* Obtain mutex for accessing system clock variables */
    xSemaphoreTake( xMutex, portMAX_DELAY );

    calculateCurrentTime( &systemClock.baseTime,
                          systemClock.lastSyncTickCount,
                          systemClock.slewRate,
                          &currentTime );

    /* Release mutex. */
    xSemaphoreGive( xMutex );

    /* Convert UTC time from UNIX timescale to SNTP timestamp format. */
    ntpSecs = currentTime.secs + SNTP_TIME_AT_UNIX_EPOCH_SECS;

    /* Support case of SNTP timestamp rollover on 7 February 2036 when
     * converting from UNIX time to SNTP timestamp. */
    if( ntpSecs > UINT32_MAX )
    {
        /* Assert when SNTP time rollover. */
        configASSERT( !( ntpSecs > UINT32_MAX ) );

        /* Subtract an extra second as timestamp 0 represents the epoch for
         * NTP era 1. */
        LogWarn( ( "SNTP timestamp rollover." ) );
        pCurrentTime->seconds = ntpSecs - UINT32_MAX - 1;
    }
    else
    {
        pCurrentTime->seconds = ntpSecs;
    }

    pCurrentTime->fractions = MILLISECONDS_TO_SNTP_FRACTIONS( currentTime.msecs );
}

static void sntpClient_SetTime( const SntpServerInfo_t * pTimeServer,
                                const SntpTimestamp_t * pServerTime,
                                int64_t clockOffsetMs,
                                SntpLeapSecondInfo_t leapSecondInfo )
{
    /* Note: This demo DOES NOT show adjustment of leap second in system time,
     * if an upcoming leap second adjustment is mentioned in server response.
     * Leap second adjustment occurs at low frequency (only for the last minute of June
     * or December) and can be useful for applications that require smooth system
     * time continuum ALWAYS including the time of the leap second adjustment.
     *
     * For more information on leap seconds, refer to
     * https://www.nist.gov/pml/time-and-frequency-division/leap-seconds-faqs.
     */
    ( void ) leapSecondInfo;

    LogInfo( ( "Received time from time server: %s", pTimeServer->pServerName ) );

    /* Obtain the mutext for accessing system clock variables. */
    xSemaphoreTake( xMutex, portMAX_DELAY );

    /* Always correct the system base time on receiving time from server.*/
    SntpStatus_t status;
    uint32_t unixSecs;
    uint32_t unixMicroSecs;

    /* Convert server time from NTP timestamp to UNIX format. */
    status = Sntp_ConvertToUnixTime( pServerTime,
                                     &unixSecs,
                                     &unixMicroSecs );
    configASSERT( status == SntpSuccess );

    /* Always correct the base time of the system clock as the time received from the server. */
    systemClock.baseTime.secs = unixSecs;
    systemClock.baseTime.msecs = unixMicroSecs / 1000;

    /* Set the clock adjustment "slew" rate of system clock if it wasn't set already and this is NOT
     * the first clock synchronization since device boot-up. */
    if( ( systemClock.firstTimeSyncDone == true ) && ( systemClock.slewRate == 0 ) )
    {
        /* We will use a "slew" correction approach to compensate for system clock
         * drift over poll interval period that exists between consecutive time synchronizations
         * with time server. */

        /* Calculate the "slew" rate for system clock as milliseconds of adjustment needed per second. */
        systemClock.slewRate = clockOffsetMs / systemClock.pollPeriod;
    }

    /* Set the system clock flag that indicates completion of the first time synchronization since device boot-up. */
    if( systemClock.firstTimeSyncDone == false )
    {
        systemClock.firstTimeSyncDone = true;
    }

    /* Store the tick count of the current time synchronization in the system clock. */
    systemClock.lastSyncTickCount = xTaskGetTickCount();

    xSemaphoreGive( xMutex );
}

static uint32_t generateRandomNumber()
{
    static uint32_t ulNextRand = 1024;
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

    ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
    return ulNextRand;
}

/*************************************************************************************/

void initializeSystemClock( void )
{
    /* On boot-up initialize the system time as the first second in the configured year. */
    uint32_t startupTimeInUnixSecs = translateYearToUnixSeconds( democonfigSYSTEM_START_YEAR );

    systemClock.baseTime.secs = startupTimeInUnixSecs;
    systemClock.baseTime.msecs = 0;

    LogInfo( ( "System time has been initialized to the year %u", democonfigSYSTEM_START_YEAR ) );
    printTime( &systemClock.baseTime );

    /* Initialize semaphore for guarding access to system clock variables. */
    xMutex = xSemaphoreCreateMutex();
    configASSERT( xMutex );

    /* Clear the first time sync completed flag of the system clock object so that a "step" correction
     * of system time is utilized for the first time synchronization from a time server. */
    systemClock.firstTimeSyncDone = false;
}

/*-----------------------------------------------------------*/

static bool initializeSntpClient( SntpContext_t * pContext,
                                  const char ** pTimeServers,
                                  size_t numOfServers,
                                  uint8_t * pContextBuffer,
                                  size_t contextBufferSize,
                                  NetworkContext_t * pUdpContext,
                                  SntpAuthContext_t * pAuthContext )
{
    bool initStatus = false;

    /* Populate the list of time servers. */
    SntpServerInfo_t * pServers = pvPortMalloc( sizeof( SntpServerInfo_t ) * numOfServers );

    if( pServers == NULL )
    {
        LogError( ( "Unable to initialize SNTP client: Malloc failed for memory of configured time servers." ) );
    }
    else
    {
        UdpTransportInterface_t udpTransportIntf;

        for( uint8_t index = 0; index < numOfServers; index++ )
        {
            pServers[ index ].pServerName = pTimeServers[ index ];
            pServers[ index ].port = SNTP_DEFAULT_SERVER_PORT;
        }

        /* Set the UDP transport interface object. */
        udpTransportIntf.pUserContext = pUdpContext;
        udpTransportIntf.sendTo = UdpTransport_Send;
        udpTransportIntf.recvFrom = UdpTransport_Recv;

        /* Initialize context. */
        Sntp_Init( pContext,
                   pServers,
                   numOfServers,
                   democonfigSERVER_RESPONSE_TIMEOUT_MS,
                   pContextBuffer,
                   contextBufferSize,
                   resolveDns,
                   sntpClient_GetTime,
                   sntpClient_SetTime,
                   &udpTransportIntf,
                   NULL );

        initStatus = true;
    }

    return initStatus;
}

/*-----------------------------------------------------------*/

static bool createUdpSocket( Socket_t * pSocket )
{
    bool status = false;
    struct freertos_sockaddr bindAddress;

    configASSERT( pSocket != NULL );

    /* Call the FreeRTOS+TCP API to create a UDP socket. */
    *pSocket = FreeRTOS_socket( FREERTOS_AF_INET,
                                FREERTOS_SOCK_DGRAM,
                                FREERTOS_IPPROTO_UDP );

    /* Check the socket was created successfully. */
    if( *pSocket == FREERTOS_INVALID_SOCKET )
    {
        /* There was insufficient FreeRTOS heap memory available for the socket
         * to be created. */
        LogError( ( "Failed to create UDP socket for SNTP client due to insufficient memory." ) );
    }
    else
    {
        /* Use a random UDP port for SNTP communication with server for protection against
         * spoofing vulnerability from "network off-path" attackers. */
        uint16_t randomPort = ( generateRandomNumber() % UINT16_MAX );
        bindAddress.sin_port = FreeRTOS_htons( randomPort );
        bindAddress.sin_family = FREERTOS_AF_INET;

        if( FreeRTOS_bind( *pSocket, &bindAddress, sizeof( bindAddress ) ) == 0 )
        {
            /* The bind was successful. */
            LogDebug( ( "UDP socket has been bound to port %u", randomPort ) );
            status = true;
        }
        else
        {
            LogError( ( "Failed to bind UDP socket to port %u", randomPort ) );
        }
    }

    return status;
}

/*-----------------------------------------------------------*/

static void closeUdpSocket( Socket_t * pSocket )
{
    configASSERT( pSocket != NULL );

    FreeRTOS_shutdown( *pSocket, FREERTOS_SHUT_RDWR );

    /* Close the socket again. */
    FreeRTOS_closesocket( *pSocket );
}

/*-----------------------------------------------------------*/

static bool calculateBackoffForNextPoll( BackoffAlgorithmContext_t * pBackoffContext,
                                         bool shouldInitializeContext,
                                         uint32_t minPollPeriod,
                                         uint32_t * pPollPeriod )
{
    uint16_t newPollPeriod = 0U;
    BackoffAlgorithmStatus_t status;

    configASSERT( pBackoffContext != NULL );
    configASSERT( pPollPeriod != NULL );

    if( shouldInitializeContext == true )
    {
        /* Initialize reconnect attempts and interval.*/
        BackoffAlgorithm_InitializeParams( pBackoffContext,
                                           minPollPeriod,
                                           SNTP_DEMO_POLL_MAX_BACKOFF_DELAY_SEC,
                                           SNTP_DEMO_MAX_SERVER_BACKOFF_RETRIES );
    }

    /* Generate a random number and calculate the new backoff poll period to wait before the next
     * time poll attempt. */
    status = BackoffAlgorithm_GetNextBackoff( pBackoffContext, generateRandomNumber(), &newPollPeriod );

    if( status == BackoffAlgorithmRetriesExhausted )
    {
        LogError( ( "All backed-off attempts of polling time server have expired: MaxAttempts=%d",
                    SNTP_DEMO_MAX_SERVER_BACKOFF_RETRIES ) );
    }
    else
    {
        /* Store the calculated backoff period as the new poll period. */
        *pPollPeriod = newPollPeriod;
    }

    return( status == BackoffAlgorithmSuccess );
}

/*-----------------------------------------------------------*/

void sntpTask( void * pParameters )
{
    SntpContext_t clientContext;
    bool initStatus = false;

    /* Variable representing the SNTP client context. */
    //static SntpContext_t context;

    /* Memory for the SNTP packet buffer in the SNTP context. */
    static uint8_t contextBuffer[ SNTP_PACKET_BASE_SIZE ];

    /* Memory for the network context representing the UDP socket that will be
     * passed to the SNTP client context. */
    static NetworkContext_t udpContext;

    /* Context used for calculating backoff that is applied to polling interval when the configured
     * time server rejects time request.
     * Note: Backoff is applied to polling interval ONLY when a single server is configured in the demo
     * because in the case of multiple server configurations, the coreSNTP library handles server
     * rejection by rotating server. */
    static BackoffAlgorithmContext_t backoffContext;

    initStatus = initializeSntpClient( &clientContext,
                                       pTimeServers,
                                       numOfServers,
                                       contextBuffer,
                                       sizeof( contextBuffer ),
                                       &udpContext,
                                       NULL );

    if( initStatus == true )
    {
        SntpStatus_t status;
        bool backoffModeFlag = false;

        /* Set the polling interval for periodic time synchronization attempts by the SNTP client. */
        systemClock.pollPeriod = democonfigSNTP_CLIENT_POLLING_INTERVAL_SECONDS;

        LogDebug( ( "Minimum SNTP client polling interval calculated as %lus", systemClock.pollPeriod ) );

        LogInfo( ( "Initialized SNTP Client context. Starting SNTP client loop to poll time every %lu seconds",
                   systemClock.pollPeriod ) );

        /* The loop of the SNTP Client task that synchronizes system time with a time server (in the configured list of time servers)
         * periodically at intervals of polling period. Each iteration of time synchronization is performed by calling the coreSNTP
         * APIs for sending time request to the server and receiving time response from the server. */
        while( 1 )
        {
            LogInfo( ( "---------STARTING DEMO---------\r\n" ) );
            bool socketStatus = false;

            /* For security, this demo keeps a UDP socket open only for one iteration of SNTP request-response cycle.
             * There is a security risk of a UDP socket being flooded with invalid or malicious server response packets
             * when a UDP socket is kept open across multiple time polling cycles. In such a scenario where the UDP
             * socket buffer has received multiple server response packets from a single time request, the extraneous
             * server response present in the UDP socket buffer will prevent the SNTP client application from correctly
             * reading network data of server responses that correspond to future time requests.
             * By closing the UDP socket after receiving the first acceptable server response (within the server response
             * timeout window), any extraneous or malicious server response packets for the same time request will be
             * ignored by the demo. */

            /* Wait for Networking */
            if( xPlatformIsNetworkUp() == pdFALSE )
            {
                LogInfo( ( "Waiting for the network link up event..." ) );

                while( xPlatformIsNetworkUp() == pdFALSE )
                {
                    vTaskDelay( pdMS_TO_TICKS( 1000U ) );
                }
            }

            /* Create a UDP socket for the current iteration of time polling. */
            socketStatus = createUdpSocket( &udpContext.socket );
            configASSERT( socketStatus == true );

            status = Sntp_SendTimeRequest( &clientContext, generateRandomNumber(), democonfigSEND_TIME_REQUEST_TIMEOUT_MS );
            configASSERT( status == SntpSuccess );

            /* Wait for server response for a maximum time of server response timeout. */
            do
            {
                /* Attempt to receive server response each time for a smaller block time
                 * than the total duration for the server response to time out. */
                status = Sntp_ReceiveTimeResponse( &clientContext, democonfigRECEIVE_SERVER_RESPONSE_BLOCK_TIME_MS );
            } while( status == SntpNoResponseReceived );

            /* Close the UDP socket irrespective of whether a server response is received. */
            closeUdpSocket( &udpContext.socket );

            /* Apply back-off delay before the next poll iteration if the demo has been configured with only
             * a single time server. */
            if( ( status == SntpRejectedResponse ) && ( numOfServers == 1 ) )
            {
                bool backoffStatus = false;

                /* Determine if this is the first back-off attempt we are making since the most recent server rejection
                 * for time request. */
                bool firstBackoffAttempt = false;

                if( backoffModeFlag == false )
                {
                    firstBackoffAttempt = true;

                    /* Set the flag to indicate we are in back-off retry mode for requesting time from the server. */
                    backoffModeFlag = true;
                }

                LogInfo( ( "The single configured time server, %s, rejected time request. Backing-off before ",
                           "next time poll....", strlen( pTimeServers[ 0 ] ) ) );

                /* Add exponential back-off to polling period. */
                backoffStatus = calculateBackoffForNextPoll( &backoffContext,
                                                             firstBackoffAttempt,
                                                             systemClock.pollPeriod,
                                                             &systemClock.pollPeriod );
                configASSERT( backoffStatus == true );

                /* Wait for the increased poll interval before retrying request for time from server. */
                vTaskDelay( pdMS_TO_TICKS( systemClock.pollPeriod * 1000 ) );
            }
            else
            {
                /* Reset flag to indicate that we are not backing-off for the next time poll. */
                backoffModeFlag = false;

                /* Wait for the poll interval period before the next iteration of time synchronization. */
                vTaskDelay( pdMS_TO_TICKS( systemClock.pollPeriod * 1000 ) );
            }
        }
    }
    else
    {
        configASSERT( false );

        /* Terminate the task as the SNTP client failed to be run. */
        LogError( ( "Failed to initialize SNTP client. Terminating SNTP client task.." ) );

        vTaskDelete( NULL );
    }
}

/*-----------------------------------------------------------*/

void systemGetWallClockTime( UTCTime_t * pTime )
{
    //TickType_t xTickCount = 0;
    //uint32_t ulTimeMs = 0UL;

    /* Obtain the mutext for accessing system clock variables. */
    xSemaphoreTake( xMutex, portMAX_DELAY );

    /* Calculate the current RAM-based time using a mathematical formula using
     * system clock state parameters and the time transpired since last synchronization. */
    calculateCurrentTime( &systemClock.baseTime,
                          systemClock.lastSyncTickCount,
                          systemClock.slewRate,
                          pTime );

    xSemaphoreGive( xMutex );
}

/*-----------------------------------------------------------*/