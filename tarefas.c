#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/watchdog.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "lwiplib.h"
#include "sntp.h"
#include "httpd.h"
#include "random.h"
#include "pinout.h"
#include "buttons.h"

#include "tarefas.h"
#include "OSTime.h"

xTaskHandle procId1;
xTaskHandle procId2;
xTaskHandle procId3;
xTaskHandle procId4;
xTaskHandle procId5;
xTaskHandle procId6;
xTaskHandle procId7;
xTaskHandle procId8;
xTaskHandle procId9;

void disk_timerproc (void);
void UARTPutString(uint32_t ui32Base, char *string);

char string[128];
void System_Time(void *param){
	   // task setup
       int i = 0;
       OS_RTC localtime;
	   (void)param;

	   OSResetTime();
	   Init_Calendar();

	   // task main loop
	   for (;;){
	      #if (WATCHDOG == 1)
	        __RESET_WATCHDOG();
	      #endif

	      vTaskDelay(10);
	      i++;

	      if (i >= 100){
	          i = 0;
	          OSUpdateUptime();
	          OSUpdateCalendar();
	          GetCalendar(&localtime);
	          sprintf(string, "\n\rCurrent local time and date: %d:%d:%d\n\r", localtime.Hour, localtime.Min, localtime.Sec);
	          UARTPutString(UART0_BASE, string);
	      }
	      //disk_timerproc();
	   }
}



void exec(void *param)
{
	(void)param;

	while(1)
	{
		vTaskDelay(5000);
		vTaskSuspend(procId3);
		vTaskDelay(5000);
		vTaskResume(procId3);
	}
}


// Declares a semaphore structure
xSemaphoreHandle semaforo = NULL;

void exec2(void *param)
{
    (void)param;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

	semaforo = xSemaphoreCreateBinary();

    while(1)
    {
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
        vTaskDelay(200);
        xSemaphoreGive(semaforo);
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1 , GPIO_PIN_1);
        vTaskDelay(200);
        xSemaphoreGive(semaforo);
    }
}








void exec3(void *param)
{
	leds_t *led = (leds_t *)param;

	SysCtlPeripheralEnable(led->clock_port);

	GPIOPadConfigSet(led->port_base, led->pin, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
	GPIOPinTypeGPIOOutput(led->port_base, led->pin);

#if 0
	while(semaforo == NULL){
		vTaskDelay(1000);
	}
#endif

	while(1)
	{
		GPIOPinWrite(led->port_base, led->pin, 0);
		//xSemaphoreTake(semaforo,portMAX_DELAY);
		vTaskDelay(200);
		GPIOPinWrite(led->port_base, led->pin, 1);
		//xSemaphoreTake(semaforo,portMAX_DELAY);
		vTaskDelay(200);
	}
}



// Declares a queue structure for the UART
xQueueHandle qUART0;

// Declares a semaphore structure for the UART
xSemaphoreHandle sUART0;

// Declares a mutex structure for the UART
xSemaphoreHandle mutexTx0;

portBASE_TYPE UARTGetChar(char *data);
void UARTPutChar(uint32_t ui32Base, char ucData);

void Terminal(void *param){
    char data;
    (void)param;

   sUART0 = xSemaphoreCreateBinary();

    if( sUART0 == NULL ){
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
        vTaskSuspend(NULL);
    }
    else
    {
        mutexTx0 = xSemaphoreCreateMutex();
        if( mutexTx0 == NULL ){
            /* There was insufficient FreeRTOS heap available for the semaphore to
            be created successfully. */
            vSemaphoreDelete( sUART0);
            vTaskSuspend(NULL);
        }else{
            qUART0 = xQueueCreate(128, sizeof(char));
            if( qUART0 == NULL ){
                /* There was insufficient FreeRTOS heap available for the queue to
                be created successfully. */
                vSemaphoreDelete( sUART0);
                vSemaphoreDelete( mutexTx0);
                vTaskSuspend(NULL);
            }else
            {
                   //
                   // Enable the peripherals used by this example.
                   //
                   MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
                   MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

                   //
                   // Set GPIO A0 and A1 as UART pins.
                   //
                   MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
                   MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
                   MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

                   //
                   // Configure the UART for 115,200, 8-N-1 operation.
                   MAP_UARTConfigSetExpClk(UART0_BASE,  configCPU_CLOCK_HZ, 115200,
                                           (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                            UART_CONFIG_PAR_NONE));

                   MAP_UARTFIFODisable(UART0_BASE);

                   //
                   // Enable the UART interrupt.
                   MAP_IntPrioritySet(INT_UART0, 0xC0);
                   ROM_IntEnable(INT_UART0);
                   ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
            }
        }
    }

    while(1)
    {
        (void)UARTGetChar(&data);
        if (data != 13){
            UARTPutChar(UART0_BASE, data);
        }else{
            UARTPutChar(UART0_BASE, '\n');
            UARTPutChar(UART0_BASE, '\r');
        }
    }
}



//*****************************************************************************
//
// The UART interrupt handler.
//
//*****************************************************************************
void UARTIntHandler(void){
    uint32_t ui32Status;
    signed portBASE_TYPE pxHigherPriorityTaskWokenRX = pdFALSE;
    signed portBASE_TYPE pxHigherPriorityTaskWokenTX = pdFALSE;
    char data;

    //
    // Get the interrrupt status.
    ui32Status = ROM_UARTIntStatus(UART0_BASE, true);

    UARTIntClear(UART0_BASE, ui32Status);

    if ((ui32Status&UART_INT_RX) == UART_INT_RX){
        //
        // Loop while there are characters in the receive FIFO.
        while(ROM_UARTCharsAvail(UART0_BASE)){
            //
            // Read the next character from the UART and write it back to the UART.
            data = (char)ROM_UARTCharGetNonBlocking(UART0_BASE);
            xQueueSendToBackFromISR(qUART0, &data, &pxHigherPriorityTaskWokenRX);
        }
    }

    if ((ui32Status&UART_INT_TX) == UART_INT_TX){
        ROM_UARTIntDisable(UART0_BASE, UART_INT_TX);

        // Call the keyboard analysis task
        xSemaphoreGiveFromISR(sUART0, &pxHigherPriorityTaskWokenTX);
    }

    if ((pxHigherPriorityTaskWokenRX == pdTRUE) || (pxHigherPriorityTaskWokenTX == pdTRUE)){
        portYIELD();
    }
}


portBASE_TYPE UARTGetChar(char *data){
    return xQueueReceive(qUART0, data, portMAX_DELAY);
}


void UARTPutChar(uint32_t ui32Base, char ucData){
    if (mutexTx0 != NULL){
        if (xSemaphoreTake(mutexTx0, portMAX_DELAY) == pdTRUE){
            //
            // Send the char.
            HWREG(ui32Base + UART_O_DR) = ucData;

            //
            // Wait until space is available.
            ROM_UARTIntEnable(UART0_BASE, UART_INT_TX);

            // Wait indefinitely for a UART interrupt
            xSemaphoreTake(sUART0, portMAX_DELAY);

            xSemaphoreGive(mutexTx0);
        }
    }
}

void UARTPutString(uint32_t ui32Base, char *string){
    if (mutexTx0 != NULL){
        if (xSemaphoreTake(mutexTx0, portMAX_DELAY) == pdTRUE){
            while(*string){
                //
                // Send the char.
                HWREG(ui32Base + UART_O_DR) = *string;

                ROM_UARTIntEnable(UART0_BASE, UART_INT_TX);

                // Wait indefinitely for a UART interrupt
                xSemaphoreTake(sUART0, portMAX_DELAY);

                string++;
            }

            xSemaphoreGive(mutexTx0);
        }
    }
}



// Declares a queue structure for the KEYB
xQueueHandle qKEYB;

// Declares a semaphore structure for the KEYB
xSemaphoreHandle sKEYB;


void Keyboard_Handler(void *param){
   // task setup
   unsigned char  key      = NO_KEY;
   unsigned int   read = 0;

   (void)param;

    sKEYB = xSemaphoreCreateBinary();

    if( sKEYB == NULL ){
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
        vTaskSuspend(NULL);
    }
    else{
        qKEYB = xQueueCreate(128, sizeof(char));

        if( qKEYB == NULL ){
            /* There was insufficient FreeRTOS heap available for the queue to
            be created successfully. */
            vTaskSuspend(NULL);
        }else{
            ButtonsInit();
        }
    }

   // task main loop
   for (;;){
      // Wait for a keyboard interrupt
      xSemaphoreTake(sKEYB,portMAX_DELAY);
      vTaskDelay(50);

      read = MAP_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);

      // Find out which key was pressed
      key = (unsigned char)read;

      // Copy the key to the keyboard buffer
      if(key != NO_KEY){
          xQueueSendToBack(qKEYB, &key,portMAX_DELAY);
      }

      key = NO_KEY;
      vTaskDelay(50);
      // Enable interrupt to the next key detection
      MAP_GPIOIntEnable(BUTTONS_GPIO_BASE, ALL_BUTTONS);
   }
}



void Keyb_Task(void *param)
{
    uint8_t key = 0;
    char buffer[768];

    (void)param;

    while( qKEYB == NULL ){
        vTaskDelay(100);
    }

    // Limpa a tela
    UARTPutString(UART0_BASE, "\033[2J\033[H");

    while(1){
        if(xQueueReceive(qKEYB, &key, portMAX_DELAY) == pdTRUE){
            switch(key){
                case SW1_KEY:
                    // Block LED RGB task
                    UARTPutString(UART0_BASE, "Bot�o SW1 pressionado!\n\r");
                    UARTPutString(UART0_BASE, "\n\r");
                    break;

                case SW2_KEY:
                    // UnBlock LED RGB task
                    UARTPutString(UART0_BASE, "Bot�o SW2 pressionado!\n\r");
                    UARTPutString(UART0_BASE, "\n\r");
                    UARTPutString(UART0_BASE, "Name                 State  Priority  Stack  Number\n\r");
                    UARTPutString(UART0_BASE, "*************************************************************\n\r");
                    vTaskList(buffer);
                    UARTPutString(UART0_BASE, buffer);
                    UARTPutString(UART0_BASE, "\n\r");
                    break;

                case BOTH_KEY:
                    UARTPutString(UART0_BASE, "Ambos os bot�es pressionados!\n\r");
                    UARTPutString(UART0_BASE, "\n\r");
                    #if 1
                    UARTPutString(UART0_BASE, "Name                  Abs Time      % Time\n\r");
                    UARTPutString(UART0_BASE, "****************************************************\n\r");
                    vTaskGetRunTimeStats(buffer);
                    UARTPutString(UART0_BASE, buffer);
                    UARTPutString(UART0_BASE, "\n\r");
                    #endif
                    break;

                default:
                    break;
            }
        }
    }
}


void ButtonsHandler(void){
    signed portBASE_TYPE pxHigherPriorityTaskWoken = pdFALSE;

    MAP_GPIOIntClear(BUTTONS_GPIO_BASE, ALL_BUTTONS);
    MAP_GPIOIntDisable(BUTTONS_GPIO_BASE, ALL_BUTTONS);

    // Call the keyboard analysis task
    xSemaphoreGiveFromISR(sKEYB, &pxHigherPriorityTaskWoken);

    if (pxHigherPriorityTaskWoken == pdTRUE){
        portYIELD();
    }
}

//*****************************************************************************
//
// Display an lwIP type IP Address.
//
//*****************************************************************************
void
DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    //
    // Convert the IP Address into a string.
    //
    sprintf(pcBuf, "%d.%d.%d.%d", (int)(ui32Addr & 0xff), (int)((ui32Addr >> 8) & 0xff),
            (int)((ui32Addr >> 16) & 0xff), (int)((ui32Addr >> 24) & 0xff));

    //
    // Display the string.
    //
    UARTPutString(UART0_BASE, pcBuf);
}

//*****************************************************************************
//
// Required by lwIP library to support any host-related timer functions.
//
//*****************************************************************************
volatile BaseType_t lwip_link_up = pdFALSE;
uint32_t g_ui32IPAddress;
void
lwIPHostTimerHandler(void)
{
    uint32_t ui32Idx, ui32NewIPAddress;

    //
    // Get the current IP address.
    //
    ui32NewIPAddress = lwIPLocalIPAddrGet();

    //
    // See if the IP address has changed.
    //
    if(ui32NewIPAddress != g_ui32IPAddress)
    {
        //
        // See if there is an IP address assigned.
        //
        if(ui32NewIPAddress == 0xffffffff)
        {
            //
            // Indicate that there is no link.
            //
            UARTPutString(UART0_BASE, "Waiting for link.\n\r");
        }
        else if(ui32NewIPAddress == 0)
        {
            //
            // There is no IP address, so indicate that the DHCP process is
            // running.
            //
            UARTPutString(UART0_BASE, "Waiting for IP address.\n\r");
        }
        else
        {
            //
            // Display the new IP address.
            //
            lwip_link_up = pdTRUE;
            UARTPutString(UART0_BASE, "IP Address: ");
            DisplayIPAddress(ui32NewIPAddress);
            UARTPutString(UART0_BASE, "\n\rOpen a browser and enter the IP address.\n\r");
        }

        //
        // Save the new IP address.
        //
        g_ui32IPAddress = ui32NewIPAddress;

        //
        // Turn GPIO off.
        //
       // MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~GPIO_PIN_1);
    }

    //
    // If there is not an IP address.
    //
    if((ui32NewIPAddress == 0) || (ui32NewIPAddress == 0xffffffff))
    {
        //
        // Loop through the LED animation.
        //

        for(ui32Idx = 1; ui32Idx < 17; ui32Idx++)
        {

            //
            // Toggle the GPIO
            //
#if 0
            MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,
                    (MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_1) ^
                     GPIO_PIN_1));

           // DelayTask(1000/ui32Idx);
#endif
        }
    }
}

//*****************************************************************************
//
// Interrupt priority definitions.  The top 3 bits of these values are
// significant with lower values indicating higher priority interrupts.
//
//*****************************************************************************
//*****************************************************************************
//
// Keeps track of elapsed time in milliseconds.
//
//*****************************************************************************
uint32_t g_ui32SystemTimeMS = 0;
#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0

#include "aws_clientcredential.h"
#include "SD_API.h"

/* Demo priorities & stack sizes. */
//#include "aws_demo_config.h"

/* FreeRTOS header files. */
#include "FreeRTOS.h"
#include "task.h"


/* Demo files. */
#include "aws_demo_runner.h"
#include "aws_system_init.h"
#include "aws_dev_mode_key_provisioning.h"

int utfpr_auth(void);

void UpLwIP(void *param)
{
    uint32_t ui32User0, ui32User1;
    //uint32_t ui32Loop;
    uint8_t pui8MACArray[8];
    (void)param;

    // Configure the device pins.
    PinoutSet(true, false);

    // Adiciona entropia - substituir por conversor A/D
    RandomAddEntropy(1324);

    vTaskDelay(1500);
    UARTPutString(UART0_BASE, "Ethernet lwIP example\n\r");

    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    ui32User0 = 0x0000001a;
    ui32User1 = 0xb60318cc;

    //MAP_FlashUserSet(ui32User0, ui32User1);
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff)){
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        // Let the user know there is no MAC address
        UARTPutString(UART0_BASE, "No MAC programmed!\n\r");
        while(1)
        {
        }
    }

    // Tell the user what we are doing just now.
    UARTPutString(UART0_BASE, "Waiting for IP.\n\r");

    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

    // Initialize the lwIP library, using DHCP.
    lwIPInit(configCPU_CLOCK_HZ, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);

    // Set the interrupt priorities.  We set the SysTick interrupt to a higher
    // priority than the Ethernet interrupt to ensure that the file system
    // tick is processed if SysTick occurs while the Ethernet handler is being
    // processed.  This is very likely since all the TCP/IP and HTTP work is
    // done in the context of the Ethernet interrupt.
    while(lwip_link_up != pdTRUE)
    {
        vTaskDelay(1000);
    }

#if 0
    // UTFPR login
    while(utfpr_auth() != pdTRUE){
        vTaskDelay(2000);
    }
#endif

    // Initialize a sample httpd server.
    httpd_init();

    // Inicia cliente SNTP
    sntp_init();

    //sys_thread_new("LwIP TCP client", SocketTCPClient, NULL, 1280, 4);

#if 0
    sys_thread_new("LwIP SMTP client", send_mail_task, NULL, 2048, 5);
#endif

#if 0
    uint8_t ret = 0;
    do
    {
        ret = InitSD(VERBOSE_ON);
        if (ret != SD_OK)
        {
            vTaskDelay(500);
        }
    }
    while(ret != SD_OK);
#endif

    /* A simple example to demonstrate key and certificate provisioning in
     * flash using PKCS#11 interface. This should be replaced
     * by production ready key provisioning mechanism. This function must be called after
     * initializing the TI File System using WIFI_On. */
    vTaskDelay(1000);
    vDevModeKeyProvisioning();

    /* Initialize the AWS Libraries system. */
    if ( SYSTEM_Init() == pdPASS ){
        DEMO_RUNNER_RunDemos();
    }

    // Loop forever.  All the work is done in the created tasks
    while(1){
        // Delay ou pode inclusive apagar a tarefa
        vTaskDelay(10000);
    }

}


void SD_Task(void *param)
{
    uint8_t ret = 0;
    uint8_t name[256]; //vetor para a leitura dos nomes dos arquivos
    do
    {
        ret = InitSD(VERBOSE_ON);
        if (ret != SD_OK)
        {
            vTaskDelay(5000);
        }
    }
    while(ret != SD_OK);

    name[0]=0;

    vTaskDelay(3000);
    ListFiles(name);

    //(void)ReadFile("atualiza.txt", VERBOSE_ON);
    //(void)UpdateFile("atualiza.txt", "Modificar essa string\r\n", "Essa � a nova string\r\n", 0, VERBOSE_ON);
    //(void)ReadFile("atualiza.txt", VERBOSE_ON);

    vTaskDelay(3000);
    while(1)
    {
        //(void)ReadFile("teste.txt", VERBOSE_OFF);
        //(void)WriteUptimeLog(VERBOSE_ON);
        //(void)ReadFile("uptime.txt", VERBOSE_ON);
        vTaskDelay(5000);
    }
}

