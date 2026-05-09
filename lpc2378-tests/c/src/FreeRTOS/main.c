/*
	FreeRTOS V5.4.2 - Copyright (C) 2009 Real Time Engineers Ltd.

	This file is part of the FreeRTOS distribution.

	FreeRTOS is free software; you can redistribute it and/or modify it	under 
	the terms of the GNU General Public License (version 2) as published by the 
	Free Software Foundation and modified by the FreeRTOS exception.
	**NOTE** The exception to the GPL is included to allow you to distribute a
	combined work that includes FreeRTOS without being obliged to provide the 
	source code for proprietary components outside of the FreeRTOS kernel.  
	Alternative commercial license and support terms are also available upon 
	request.  See the licensing section of http://www.FreeRTOS.org for full 
	license details.

	FreeRTOS is distributed in the hope that it will be useful,	but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
	more details.

	You should have received a copy of the GNU General Public License along
	with FreeRTOS; if not, write to the Free Software Foundation, Inc., 59
	Temple Place, Suite 330, Boston, MA  02111-1307  USA.


	***************************************************************************
	*                                                                         *
	* Looking for a quick start?  Then check out the FreeRTOS eBook!          *
	* See http://www.FreeRTOS.org/Documentation for details                   *
	*                                                                         *
	***************************************************************************

	1 tab == 4 spaces!

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting,
	licensing and training services.
*/


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Demo application definitions. */
#define mainQUEUE_SIZE			( 3 )

/* Task priorities. */
#define mainSERIAL_TASK_PRIORITY	( tskIDLE_PRIORITY + 5 )
#define mainFLASH_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
#define mainSTATS_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )

/*
 * The UART is written to by more than one task so is controlled by a 
 * 'gatekeeper' task.  This is the only task that is actually permitted to 
 * access the UART directly.  Other tasks wanting to display a message send
 * the message to the gatekeeper.
 */
static void vSerialTask( void *pvParameters );
static void vFlashTask( void *pvParameters );
static void vStatsTask( void *pvParameters );

/* Configure the hardware as required by the demo. */
static void prvSetupHardware( void );

/* The queue used to send messages to the Serial task. */
xQueueHandle xSerialQueue;

/*-----------------------------------------------------------*/

int main( void )
{
	prvSetupHardware();
	
	/* 
	   Create the queue used by the Serial task.  
	   Messages for display on the Serial are received via this queue.
	*/

	xSerialQueue = xQueueCreate( mainQUEUE_SIZE, sizeof( xSerialMessage ) );

	/* Start the tasks */
	xTaskCreate( vSerialTask, ( signed portCHAR * ) "Serial", 
		configMINIMAL_STACK_SIZE, NULL, mainSERIAL_TASK_PRIORITY, NULL );

	xTaskCreate( vFlashTask, ( signed portCHAR * ) "Flash", 
		configMINIMAL_STACK_SIZE, NULL, mainFLASH_TASK_PRIORITY, NULL );

	xTaskCreate( vStatsTask, ( signed portCHAR * ) "Stats", 
		configMINIMAL_STACK_SIZE, NULL, mainSTATS_TASK_PRIORITY, NULL );

	/* Start the scheduler */
	vTaskStartScheduler();

    	/* Will only get here if there was insufficient memory to create the idle task */
	return 0; 
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
static xSerialMessage xMessage = { "Tick Hook" };
static unsigned portLONG ulTicksSinceLastDisplay = 0;
static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* Send the message to the Serial gatekeeper for display. */
	xHigherPriorityTaskWoken = pdFALSE;
	xQueueSendToBackFromISR( xSerialQueue, &xMessage, &xHigherPriorityTaskWoken );
}

/*-----------------------------------------------------------*/

void vSerialTask( void *pvParameters )
{
static xSerialMessage xMessage;

	/* Initialise the Serial and display a startup message. */
	vTaskSuspendAll();
	{
		xprintf("Start 'Serial' task\n");
	}
	xTaskResumeAll();

	for( ;; )
	{
		/* Wait for a message to arrive that requires displaying. */
		while( xQueueReceive( xSerialQueue, &xMessage, portMAX_DELAY ) != pdPASS );
		
		/* Display the message.  Print each message to a different position. */
		xprintf("Got message: %s\n", xMessage.pcMessage);
	}

}

void vFlashTask( void *pvParameters )
{
const portTickType xDelay = 5000 / portTICK_RATE_MS;	/* 5sec */

static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
static xSerialMessage xMessage = { "Blink" };
unsigned int count = 0;

	/* Initialise LEDs */
	vTaskSuspendAll();
	{
		xprintf("Start 'Flash' task\n");
		xprintf("LED init completed\n");
	}
	xTaskResumeAll();

	for( ;; )
	{
		/* Wait for a message to arrive that requires displaying. */
		xQueueSendToBackFromISR( xSerialQueue, &xMessage, &xHigherPriorityTaskWoken );
		
		/* TODO: Toggle LEDs */

		/* Now busy loop just to create visible effect in stats  */
		for(count = 0; count < 0xFFFF; count++);

		vTaskSuspendAll();
		{
			count = portGET_RUN_TIME_COUNTER_VALUE();
			xprintf("Stats timer: %u\n", count);
		}
		xTaskResumeAll();

		vTaskDelay(xDelay);	
	}

}

static signed portCHAR stats_buffer[200];
static int stats_ready = 0;

void vStatsTask( void *pvParameters )
{
const portTickType statsDelay = 20000 / portTICK_RATE_MS;	/* 20sec */

	vTaskSuspendAll();
	{
		xprintf("Start 'Stats' task\n");
	}
	xTaskResumeAll();

	for( ;; )
	{
		if (stats_ready++ > 0) {
			vTaskGetRunTimeStats(stats_buffer);
        		
			vTaskSuspendAll();
			{
				xprintf(stats_buffer);
			}
        		xTaskResumeAll();
		}

		vTaskDelay(statsDelay);	
	}

}

/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	init_mcu();
	init_vic();
	init_serial_noirq(9600);

	xprintf("HW init completed\n");
}







