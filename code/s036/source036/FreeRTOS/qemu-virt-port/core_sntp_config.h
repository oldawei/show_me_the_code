#ifndef CORE_SNTP_CONFIG_H
#define CORE_SNTP_CONFIG_H

#include "printf/printf.h"
#include <stdint.h>

#define LOGGING_LEVEL_INFO

#define TZONE (8*60*60)

/************* Define Logging Macros using printf function ***********/

#define PrintfError( ... )         printf( "Error: "__VA_ARGS__ );  printf( "\n" )
#define PrintfWarn( ... )          printf( "Warn: "__VA_ARGS__ );  printf( "\n" )
#define PrintfInfo( ... )          printf( "Info: " __VA_ARGS__ ); printf( "\n" )
#define PrintfDebug( ... )         printf( "Debug: " __VA_ARGS__ ); printf( "\n" )

#ifdef LOGGING_LEVEL_ERROR
    #define LogError( message )    PrintfError message
    #define LogWarn( message )
    #define LogInfo( message )
    #define LogDebug( message )
#elif defined( LOGGING_LEVEL_WARNING )
    #define LogError( message )    PrintfError message
    #define LogWarn( message )     PrintfWarn message
    #define LogInfo( message )
    #define LogDebug( message )
#elif defined( LOGGING_LEVEL_INFO )
    #define LogError( message )    PrintfError message
    #define LogWarn( message )     PrintfWarn message
    #define LogInfo( message )     PrintfInfo message
    #define LogDebug( message )
#elif defined( LOGGING_LEVEL_DEBUG )
    #define LogError( message )    PrintfError message
    #define LogWarn( message )     PrintfWarn message
    #define LogInfo( message )     PrintfInfo message
    #define LogDebug( message )    PrintfDebug message
#endif /* ifdef LOGGING_LEVEL_ERROR */

/**************************************************/


/**
 * @brief Utility macro to convert milliseconds to the fractions value of an SNTP timestamp.
 * @note The fractions value MUST be less than 1000 as duration of seconds is not represented
 * as fractions part of SNTP timestamp.
 */
#define MILLISECONDS_TO_SNTP_FRACTIONS( ms )    ( ms * 1000 * SNTP_FRACTION_VALUE_PER_MICROSECOND )

/**
 * @brief Type representing system time in Coordinated Universal Time (UTC)
 * zone as time since 1st January 1970 00h:00m:00s.
 *
 * @note This demo uses RAM-based mathematical model to represent UTC time
 * in system.
 */
typedef struct UTCTime
{
    uint32_t secs;
    uint32_t msecs;
} UTCTime_t;

/**
 * @brief Utility function to print the system time as both UNIX time (i.e.
 * time since 1st January 1970 00h:00m:00s) and human-readable time (in the
 * YYYY-MM-DD dd:mm:ss format).
 *
 * @param[in] pTime The system time to be printed.
 */
void printTime( const UTCTime_t * pTime );

/**
 * @brief Initializes the system clock with the first second of the year (i.e. at midnight
 * of 1st January) that is configured in the democonfigSYSTEM_START_YEAR config of
 * demo_config.h file.
 */
void initializeSystemClock( void );

/**
 * @brief The demo function for an application to query wall-clock
 * time as Coordinated Universal Time (UTC) from the system.
 *
 * @note This demo showcases a RAM-based mathematical model for
 * representing current UTC time in the system.
 *
 * @param[out] pTime This will be populated with the current time
 * in the system as total time since 1st January 1900 00h:00m:00s.
 */
void systemGetWallClockTime( UTCTime_t * pTime );

/**
 * @brief The task function for a sample application that queries
 * system time periodically.
 */
void sampleAppTask( void * pvParameters );

/**
 * @brief The task function that represents a daemon SNTP client task
 * that is responsible for periodically synchronizing system time with
 * time servers from the list of configured time servers in
 * democonfigLIST_OF_TIME_SERVERS.
 *
 * @note The usage of the coreSNTP library API is encapsulated within
 * this task. The rest of the FreeRTOS tasks/application does not need
 * to be aware of the SNTP client as they can query time from the
 * @ref systemGetWallClockTime() function.
 */
void sntpTask( void * parameters );

#endif // CORE_SNTP_CONFIG_H