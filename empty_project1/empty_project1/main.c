//******************************************************************************
//      Purpose       - Budget Vector Network Analyzer
//		Written by    - Cody Wilson
//                    - Nathan Gray
//      Written on    - 5/--/2016
//      Version       - 2
//
//
/*******************************************************************************
    General Notes and Specifications

    Run on Putty (COM4, 115200 baud)
    Known issues: baud rate should be 4800 kBaud
    Issues with the master clock frequency. 3 or 48 MHz?
    For debugging it would be nice to be able to change the frequency by
    pressing a button.
								/**/
/******************************************************************************
 * 2016 WWU Direct Conversion VNA
 * Author: Rob Frohne
 * Thanks to Gerard Sequeira, 43oh for the backchannel UART ideas and to TI for
 * the examples and driverlib.
*******************************************************************************/

#define USE_SPI

// New include
#include "main.h"


/*******************************************************************************************
    Main function
    The purpose of this is to provide a kernel for the MSP432 VNA.
    Does not exit.

 */
void main(void)
{

    volatile uint32_t z;
	volatile int i, j, temp;
	volatile uint16_t test[NUM_ADC14_CHANNELS];

    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    /* Halting WDT  */ // --------------------------------- ???
    MAP_WDT_A_holdTimer();

    // Enable floating point
    MAP_FPU_enableModule();
    // MAP_FPU_enableLazyStacking();

    // FPU_disableModule();
    // FPU_disableStacking();
*/
    //MAP_Interrupt_enableSleepOnIsrExit();

    // Run system communication initialization
    systemCommunicationInit();

    // System Test Block to test the DDS
    setDDSFrequency(1000000);

    readRegs();

    uint8_t initAddresses[]={0x01,0x02,0x03,0x06,0x09,0x0d,0x13};
    System_printf("Dumping Versaclock RAM after setting to 1MHz.\n\r");
    System_printf("Address   Received    Written\n\r");
    for (i=0 ; i<7 ; i++)
    {
    	System_printf("%d    %x       %x\n\r", initAddresses[i], RXData[initAddresses[i]],
    			PllClockRegisters.init1MHzRegisterValues[i]);
    	if(RXData[initAddresses[i]]!=PllClockRegisters.init1MHzRegisterValues[i])
    		System_printf("They don't match!\n\r");
    }

 //   setDDSFrequency(2000000);
 //
    if(!updateVersaclockRegs(2000000))
    	System_printf("Registers Unchanged\n\r");

    System_printf("Address,  Read,    Tried to Write\n\r");
    for (i=0;i<NUM_OF_CHANGED_REG_BYTES; i++)
    {
    		System_printf("%d       %x        %x\n\r",PllClockRegisters.changedAddresses[i],
    				RXData[PllClockRegisters.changedAddresses[i]],
					PllClockRegisters.registerValues[1][i]);
    		if(RXData[PllClockRegisters.changedAddresses[i]]!=
    				PllClockRegisters.registerValues[1][i])
    			System_printf("VersaClock Registers did NOT match!\n\r");
    }

    // Kernel loop
	while(1)
	{

	  //  readRegs();
		//System_printf("This is the simplist function there is!\n");

	    // //Test LED light
	    GPIO_toggleOutputOnPin(
	        GPIO_PORT_P1,
			GPIO_PIN0
			);

	    // Delay
	    for(z=50000; z>0; z--);

/*
	    //DDSFrequency == 1000000;
		if(DDSFrequency == 50000000)			//If we reach max, reset to a slower frequency
			DDSFrequency = 000000;

	    setDDSFrequency(DDSFrequency);
	    DDSFrequency = DDSFrequency + 1000000; //Increment by 1 MHz
	    if(!updateVersaclockRegs(DDSFrequency)){
	    	System_printf("Registers unchanged\n\r");}
*/
	    for(z=50000; z>0; z--);

/*		Pulse the start of a conversion.	*/
	    GPIO_toggleOutputOnPin(
	        GPIO_PORT_P3,
			GPIO_PIN5
			);
	    //Test VNA LED Output
	   // GPIO_setOutputHighOnPin();
		//MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN5);

		while(!MAP_ADC14_toggleConversionTrigger()){// This function starts the adc sampling and conversion
			for(i=0;i<100;i++);  // Wait for conversion to finish.
		}

/* 		for(i=0;i<1000;i++){
			temp = i*temp;
		}*/
		System_printf("\r\n Results are:\r\n");
		for(i=0; i<NUM_ADC14_CHANNELS; i++){
			test[i] = resultsBuffer[i];
			System_printf("ADC # %d  \r\n",i);
			System_printf("Result: %d\n\r",resultsBuffer[i]);
		}
		System_printf("f= %d MHz\r\n",DDSFrequency/1000000);
		//MAP_PCM_gotoLPM0();
	}
}
