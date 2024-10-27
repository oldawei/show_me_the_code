#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "printf/printf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "FreeRTOS_CLI.h"

void print_task_list( void )
{
    char buff[512] = { 0 };
    char *pcWriteBuffer = buff;
    const char *const pcHeader = "     State   Priority  Stack    #\r\n************************************************\r\n";
    BaseType_t xSpacePadding;

    memset(buff, 0, sizeof(buff));
	/* Generate a table of task stats. */
	strcpy( pcWriteBuffer, "Task" );
    // printf("11: %s", pcWriteBuffer);
	pcWriteBuffer += strlen( pcWriteBuffer );

	/* Minus three for the null terminator and half the number of characters in
	"Task" so the column lines up with the centre of the heading. */
	configASSERT( configMAX_TASK_NAME_LEN > 3 );
	for( xSpacePadding = strlen( "Task" ); xSpacePadding < ( configMAX_TASK_NAME_LEN - 3 ); xSpacePadding++ )
	{
		/* Add a space to align columns after the task's name. */
		*pcWriteBuffer = ' ';
		pcWriteBuffer++;

		/* Ensure always terminated. */
		*pcWriteBuffer = 0x00;
	}
	strcpy( pcWriteBuffer, pcHeader );
    // printf("22: %s", pcWriteBuffer);
	vTaskList( pcWriteBuffer + strlen( pcHeader ) );

    printf("%s", buff);
}
/*-----------------------------------------------------------*/

static BaseType_t prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    const char *const pcHeader = "     State   Priority  Stack    #\r\n************************************************\r\n";
    BaseType_t xSpacePadding;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	/* Generate a table of task stats. */
	strcpy( pcWriteBuffer, "Task" );
	pcWriteBuffer += strlen( pcWriteBuffer );

	/* Minus three for the null terminator and half the number of characters in
	"Task" so the column lines up with the centre of the heading. */
	configASSERT( configMAX_TASK_NAME_LEN > 3 );
	for( xSpacePadding = strlen( "Task" ); xSpacePadding < ( configMAX_TASK_NAME_LEN - 3 ); xSpacePadding++ )
	{
		/* Add a space to align columns after the task's name. */
		*pcWriteBuffer = ' ';
		pcWriteBuffer++;

		/* Ensure always terminated. */
		*pcWriteBuffer = 0x00;
	}
	strcpy( pcWriteBuffer, pcHeader );
	vTaskList( pcWriteBuffer + strlen( pcHeader ) );

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

#if( configGENERATE_RUN_TIME_STATS == 1 )
static BaseType_t prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    printf("Total ticks: %lu\n", (u32)goldfish_rtc_read_ticks());

    const char * const pcHeader = "  Abs Time      % Time\r\n****************************************\r\n";
    BaseType_t xSpacePadding;

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    /* Generate a table of task stats. */
    strcpy( pcWriteBuffer, "Task" );
    pcWriteBuffer += strlen( pcWriteBuffer );

    /* Pad the string "task" with however many bytes necessary to make it the
    length of a task name.  Minus three for the null terminator and half the
    number of characters in	"Task" so the column lines up with the centre of
    the heading. */
    for( xSpacePadding = strlen( "Task" ); xSpacePadding < ( configMAX_TASK_NAME_LEN - 3 ); xSpacePadding++ )
    {
        /* Add a space to align columns after the task's name. */
        *pcWriteBuffer = ' ';
        pcWriteBuffer++;

        /* Ensure always terminated. */
        *pcWriteBuffer = 0x00;
    }

    strcpy( pcWriteBuffer, pcHeader );
    vTaskGetRunTimeStats( pcWriteBuffer + strlen( pcHeader ) );

    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}
#endif /* configGENERATE_RUN_TIME_STATS */
/*-----------------------------------------------------------*/

#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
static BaseType_t prvQueryHeapCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    sprintf( pcWriteBuffer, "Current free heap %d bytes, minimum ever free heap %d bytes\r\n", ( int ) xPortGetFreeHeapSize(), ( int ) xPortGetMinimumEverFreeHeapSize() );

    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}
#endif /* configINCLUDE_QUERY_HEAP */
/*-----------------------------------------------------------*/

char *get_dec_1(const char *cmd, u32 *v1)
{
    char *endp;
    *v1 = (u32)strtoul(cmd, &endp, 10);

    return endp;
}

char *get_dec_6(const char *cmd, u32 *v1, u32 *v2, u32 *v3, u32 *v4, u32 *v5, u32 *v6)
{
    char *endp;
    *v1 = (u32)strtoul(cmd, &endp, 10);
    if (*v1)
        *v2 = (u32)strtoul(endp + 1, &endp, 10);
    if (*v2)
        *v3 = (u32)strtoul(endp + 1, &endp, 10);
    if (*v3)
        *v4 = (u32)strtoul(endp + 1, &endp, 10);
    if (*v4)
        *v5 = (u32)strtoul(endp + 1, &endp, 10);
    if (*v5)
    *v6 = (u32)strtoul(endp + 1, &endp, 10);

    return endp;
}

char *get_hex_1(const char *cmd, u32 *v1)
{
    char *endp;
    *v1 = (u32)strtoul(cmd, &endp, 16);

    return endp;
}

char *get_hex_2(const char *cmd, u32 *v1, u32 *v2)
{
    char *endp;

    *v1 = (u32)strtoul(cmd, &endp, 16);
    *v2 = (u32)strtoul(endp + 1, &endp, 16);

    return endp;
}

static BaseType_t prvMemReadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    (void)xWriteBufferLen;
    BaseType_t xParameter1StringLength;

    /* Obtain the name of the source file, and the length of its name, from
    the command string. The name of the source file is the first parameter. */
    const char *pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

    u32 addr = 0;
    u32 num = 0;
    // sscanf(pcParameter1, "%x %x", &addr, &num);
    get_hex_2(pcParameter1, &addr, &num);

    *pcWriteBuffer = 0x00;
    printf("Addr\t\t\tVaule\r\n");
    u32 *pt = (u32 *)addr;
    for (int i = 0; i < num; ++i) {
        printf("0x%08x\t\t0x%08x\r\n", (u32)(pt+i), pt[i]);
    }

    /* There is only a single line of output produced in all cases.  pdFALSE is
    returned because there is no more output to be generated. */
    return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvMemWriteCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    (void)xWriteBufferLen;
    BaseType_t xParameter1StringLength;

    /* Obtain the name of the source file, and the length of its name, from
    the command string. The name of the source file is the first parameter. */
    const char *pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

    u32 addr = 0;
    u32 value = 0;
    // sscanf(pcParameter1, "%x %x", &addr, &value);
    get_hex_2(pcParameter1, &addr, &value);

    *pcWriteBuffer = 0x00;
    u32 *pt = (u32 *)addr;

    printf("Before write:\r\nAddr\t\t\tVaule\r\n");
    printf("0x%08x\t\t0x%08x\r\n", addr, *pt);

    *pt = value;
    printf("After write:\r\nAddr\t\t\tVaule\r\n");
    printf("0x%08x\t\t0x%08x\r\n", addr, *pt);

    /* There is only a single line of output produced in all cases.  pdFALSE is
    returned because there is no more output to be generated. */
    return pdFALSE;
}
/*-----------------------------------------------------------*/

/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xTaskStats =
{
	"task-stats", /* The command string to type. */
	"\r\ntask-stats:\r\n Displays a table showing the state of each FreeRTOS task\r\n",
	prvTaskStatsCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

#if( configGENERATE_RUN_TIME_STATS == 1 )
/* Structure that defines the "run-time-stats" command line command.   This
generates a table that shows how much run time each task has */
static const CLI_Command_Definition_t xRunTimeStats =
{
    "run-time-stats", /* The command string to type. */
    "\r\nrun-time-stats:\r\n Displays a table showing how much processing time each FreeRTOS task has used\r\n",
    prvRunTimeStatsCommand, /* The function to run. */
    0 /* No parameters are expected. */
};
#endif /* configGENERATE_RUN_TIME_STATS */

#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
/* Structure that defines the "query_heap" command line command. */
static const CLI_Command_Definition_t xQueryHeap =
{
    "query-heap",
    "\r\nquery-heap:\r\n Displays the free heap space, and minimum ever free heap space.\r\n",
    prvQueryHeapCommand, /* The function to run. */
    0 /* The user can enter any number of commands. */
};
#endif /* configQUERY_HEAP_COMMAND */

/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xMemRead =
{
	"mem-read", /* The command string to type. */
	"\r\nmem-read <addr> <number>:\r\n Read <number> dwords memory value from <addr> to <addr + 4*number >\r\n",
	prvMemReadCommand, /* The function to run. */
	2 /* No parameters are expected. */
};

/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xMemWrite =
{
	"mem-write", /* The command string to type. */
	"\r\nmem-write <addr> <value>:\r\n Write memory <addr> with <value>\r\n",
	prvMemWriteCommand, /* The function to run. */
	2 /* No parameters are expected. */
};

static StreamBufferHandle_t xStreamBuffer = NULL;

#define MAX_INPUT_LENGTH    64
#define MAX_OUTPUT_LENGTH   128

static const char * const pcWelcomeMessage =
  "FreeRTOS command server.\r\nType help to view a list of registered commands.\r\n";

void vCommandConsoleTask( void *pvParameters )
{
    (void)pvParameters;
    char cRxedChar;
    int cInputIndex = 0;
    BaseType_t xMoreDataToFollow;
    /* The input and output buffers are declared static to keep them off the stack. */
    //static char pcOutputString[ MAX_OUTPUT_LENGTH ];
    char *pcOutputString = FreeRTOS_CLIGetOutputBuffer();
    static char pcInputString[ MAX_INPUT_LENGTH ];

    /* Send a welcome message to the user knows they are connected. */
    printf( "%s\n", pcWelcomeMessage );

    for ( ;; ) {
        /* This implementation reads a single character at a time.  Wait in the
        Blocked state until a character is received. */
        xStreamBufferReceive( xStreamBuffer, &cRxedChar, sizeof( cRxedChar ), portMAX_DELAY );

        if ( cRxedChar == '\n' ) {
            /* A newline character was received, so the input command string is
            complete and can be processed.  Transmit a line separator, just to
            make the output easier to read. */
            //printf( "%s\n", pcInputString );

            /* The command interpreter is called repeatedly until it returns
            pdFALSE.  See the "Implementing a command" documentation for an
            exaplanation of why this is. */
            do {
                /* Send the command string to the command interpreter.  Any
                output generated by the command interpreter will be placed in the
                pcOutputString buffer. */
                xMoreDataToFollow = FreeRTOS_CLIProcessCommand
                              (
                                  pcInputString,   /* The command string.*/
                                  pcOutputString,  /* The output buffer. */
                                  configCOMMAND_INT_MAX_OUTPUT_SIZE/* The size of the output buffer. */
                              );

                /* Write the output generated by the command interpreter to the
                console. */
                printf( "%s\n", pcOutputString );

            } while( xMoreDataToFollow != pdFALSE );

            /* All the strings generated by the input command have been sent.
            Processing of the command is complete.  Clear the input string ready
            to receive the next command. */
            cInputIndex = 0;
            memset( pcInputString, 0x00, MAX_INPUT_LENGTH );
        } else {
            /* The if() clause performs the processing after a newline character
            is received.  This else clause performs the processing if any other
            character is received. */

            if ( cRxedChar == '\r' ) {
                /* Ignore carriage returns. */
            } else if( cRxedChar == '\b' ) {
                /* Backspace was pressed.  Erase the last character in the input
                buffer - if there are any. */
                if( cInputIndex > 0 )
                {
                    cInputIndex--;
                    pcInputString[ cInputIndex ] = '\0';
                    printf("\r\033[K");
                    printf( "%s", pcInputString );
                }
            } else {
                /* A character was entered.  It was not a new line, backspace
                or carriage return, so it is accepted as part of the input and
                placed into the input buffer.  When a n is entered the complete
                string will be passed to the command interpreter. */
                if ( cInputIndex < MAX_INPUT_LENGTH ) {
                    pcInputString[ cInputIndex ] = cRxedChar;
                    cInputIndex++;
                }
            }
        }
    }
}

void cli_init(void)
{
    FreeRTOS_CLIRegisterCommand( &xTaskStats );
#if( configGENERATE_RUN_TIME_STATS == 1 )
    FreeRTOS_CLIRegisterCommand( &xRunTimeStats );
#endif
#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
    FreeRTOS_CLIRegisterCommand( &xQueryHeap );
#endif
    FreeRTOS_CLIRegisterCommand( &xMemRead );
    FreeRTOS_CLIRegisterCommand( &xMemWrite );

    xStreamBuffer = xStreamBufferCreate( 64, 1 );
    xTaskCreate(vCommandConsoleTask, (portCHAR *)"CliTask", 512, NULL, 10, NULL);
}

void cli_putchar(u8 c)
{
    BaseType_t xHigherPriorityTaskWoken = 0;
    if (xStreamBuffer) {
        xStreamBufferSendFromISR(xStreamBuffer, &c, 1, &xHigherPriorityTaskWoken );
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}
