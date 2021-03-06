//-----------------------------------------------------------------------------
//Cody Wilson
//   and
//Nathan Gray
//Budget Vector Network Analyzer
//5/--/2016
//Version 1
//Code adopted from Professor Frohne below (his heading kept).
//-----------------------------------------------------------------------------

// Run on Putty (COM4, 115200 baud)
//Known issues: baud rate should be 4800 kBaud
// Master clock frequency correct? 3 ot 48 MHz?
// For debugging it would be nice to be able to change the frequency by pressing a button
/******************************************************************************
 * 2016 WWU Direct Conversion VNA
 * Author: Rob Frohne
 * Thanks to Gerard Sequeira, 43oh for the backchannel UART ideas and to TI for
 * the examples and driverlib.
*******************************************************************************/

#define USE_SPI

/* DriverLib Includes */
#include "driverlib.h"
#include "msp432.h"
#include "hw_memmap.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

#include <string.h>
#include "printf.h"
#include <math.h>


/* Global variables */

/* I2C Variables Initialization */

//Our PLL is at address: 1101001
// In Hex, 0x69
#define SLAVE_ADDRESS       0x69
#define NUM_OF_REG_BYTES 	27 //number of register bytes
#define NUM_OF_CHANGED_REG_BYTES 3
#define NUM_BANDS 8
#define NUM_BAND_BLOCKS 3
#define FIRST_REG 0x01

#define NUM_ADC14_CHANNELS 4

const uint8_t firstReg = FIRST_REG;
static uint8_t TXByteCtr;
static uint8_t RXData[NUM_OF_REG_BYTES+0x10];
const static volatile uint8_t *TXData;
static volatile uint32_t xferIndex;
static volatile bool justSending;

/* Initial data structure for I2C parameters.
 * We will only change the parameters that need changed.
 */
const struct PllClockRegisters { /* VersaClock Register Data
	 * We first initialize registers for 1MHz operation.
	 * Then when we change frequencies, we change only the
	 * registers that change when frequencies change.
	 * These registers come in four blocks. The first block is 2 long;
	 * the second is 1; the third block is 2 long, and the last is 5 long. */
	uint8_t blockNumBytes[NUM_BAND_BLOCKS];
	uint8_t blockFirstAddress[NUM_BAND_BLOCKS];
	uint8_t changedAddresses[NUM_OF_CHANGED_REG_BYTES];
	/* The band edges:  The index of the frequency of the lower edge
	 * goes with the same index of the registers to send for that band. */
	long int frequencyBandLimit[NUM_BANDS+1];
	/* The register values for each of the 24 bands. */
	uint8_t registerValues[NUM_BANDS][NUM_OF_CHANGED_REG_BYTES];
	/* The init1MHzRegisterValues are for the 1 MHz to 1.199 MHz band. */
	uint8_t init1MHzRegisterValues[NUM_OF_REG_BYTES];
} PllClockRegisters =
		{{1,1,1}, //(three band blocks) total blocks in each change
		{0x2,0x6,0xd}, //First (addresses) registers for each block
		{0x2,0x6,0xd}, //A list of all of the addresses that actually need to be changed
		{1000,3000,4000,10000,16000,24000,38000,48000},
		{{0x50,0x0,0x14},
		{0x50,0x80,0x14},
		},
		{0x01,0x50,0b01100000,0b00000000,0b00100000,0x14,0b00111000} //70 byte initialization for beginning
		 };

/* I2C Master Configuration Parameter */
const eUSCI_I2C_MasterConfig i2cConfig =
{
        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        3000000,                                // SMCLK = 3MHz
        EUSCI_B_I2C_SET_DATA_RATE_100KBPS,      // Desired I2C Clock of 100khz
        0,                                      // No byte counter threshold
        EUSCI_B_I2C_NO_AUTO_STOP                // No Autostop
};
/* I2C Master Configuration Parameter */

/* Results buffer for ADC14 */
uint16_t resultsBuffer[NUM_ADC14_CHANNELS]={0,0,0,0}; //ADC results

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://processors.wiki.ti.com/index.php/
 *               USCI_UART_Baud_Rate_Gen_Mode_Selection
 *
 * On Linux use /dev/ttyACM0, 115200, 8N1.
 *               CuteCom works well.
 */
const eUSCI_UART_Config uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        26,                                      // BRDIV = 26
        0,                                       // UCxBRF = 0
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // MSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION  // Low Frequency Mode
};

#ifdef USE_SPI
/* SPI Master Configuration Parameter */
const eUSCI_SPI_MasterConfig spiMasterConfig =
{
		EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
		// SMCLK Clock Source
		3000000,
		// SMCLK = DCO = 3MHZ
		500000,
		// SPICLK = 500khz
		EUSCI_B_SPI_LSB_FIRST,
		// MSB First
		EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,
		// Phase
		EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW, // Low polarity
		EUSCI_B_SPI_3PIN
		// 3Wire SPI Mode
};
#endif

/* Forward Declaration of Functions */
void initializeClocks(void);
int initializeBackChannelUART(void);
int initializeADC(void);
int initializeDDS(void);
int initializeVersaclock(void);
int initializeI2C(void);
int updateVersaclockRegs(long int frequency);
void dumpI2C(void);
bool initCDCE(void);
void writeVersaClockBlock(const uint8_t *firstDataPtr ,uint8_t blockStart, uint8_t numBytes);
int setDDSFrequency(long long frequency);
void pulseFQ_UD(void);
void pulse_W_CLK(void);
void pulse_DDS_RST(void);
void initI2C(void);


/*
 * USCIA0 interrupt handler for backchannel UART.
 * For interrupts, don't forget to edit the startup...c file!
 */
void EusciA0_ISR(void)
{
    int receiveByte = UCA0RXBUF;
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    /* Echo back. */
    MAP_UART_transmitData(EUSCI_A0_BASE, receiveByte);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

/*
 * This interrupt is fired whenever a conversion is completed and placed in
 * ADC_MEM7. This signals the end of conversion and the results array is
 * grabbed and placed in resultsBuffer
 */
void ADC14_IRQHandler(void)
{
    uint64_t status;
    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    if(status & ADC_INT3)
    {
        ADC14_getMultiSequenceResult(resultsBuffer);
    }
}

/*
 * eUSCIB0 ISR.
 * For interrupts, don't forget to edit the startup...c file!
 */
void EUSCIB1_IRQHandler(void)
{
	uint_fast16_t status;

	status = I2C_getEnabledInterruptStatus(EUSCI_B1_BASE);
	I2C_clearInterruptFlag(EUSCI_B1_BASE, status);

	if(justSending) /* We don't need to worry about receiving. */
	{
		if (status & EUSCI_B_I2C_NAK_INTERRUPT)
		{
	        I2C_masterSendStart(EUSCI_B1_BASE);
		}

		if (status & EUSCI_B_I2C_TRANSMIT_INTERRUPT0)
		{
			/* Check the byte counter */
			if (TXByteCtr)
			{
				/* Send the next data and decrement the byte counter */
				I2C_masterSendMultiByteNext(EUSCI_B1_BASE, TXData[xferIndex++]);
				TXByteCtr--;
			} else
			{
				I2C_masterSendMultiByteStop(EUSCI_B1_BASE);
				xferIndex=0;
				Interrupt_disableSleepOnIsrExit();
			}
			//MAP_I2C_clearInterruptFlag(EUSCI_B1_BASE, status);
		}
	}
	else /* Not just sending.  We are reading the versaClock registers. */
	{
		/* If we reached the transmit interrupt, it means we are at the last byte (was index 1) of
		 * the transmit buffer. When doing a repeated start, before we reach the
		 * last byte we will need to change the mode to receive mode, set the start
		 * condition send bit, and then load the final byte into the TXBUF.
		 */

		if (status & EUSCI_B_I2C_TRANSMIT_INTERRUPT0)
		{
			//I2C_masterSendMultiByteNext(EUSCI_B1_BASE, firstReg);
			I2C_disableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
			I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
			xferIndex = 0;
			I2C_masterReceiveStart(EUSCI_B1_BASE);
			I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);

		}

		/* Receives bytes into the receive buffer. If we have received all bytes,
		 * send a STOP condition */
		if (status & EUSCI_B_I2C_RECEIVE_INTERRUPT0)
		{
			if(xferIndex == NUM_OF_REG_BYTES+0x10 - 2)
			{
				I2C_masterReceiveMultiByteStop(EUSCI_B1_BASE);
				RXData[xferIndex++] = I2C_masterReceiveMultiByteNext(EUSCI_B1_BASE);
			}
			else if(xferIndex == NUM_OF_REG_BYTES+0x10 - 1)
			{
				RXData[xferIndex] = I2C_masterReceiveMultiByteNext(EUSCI_B1_BASE);
				I2C_disableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);
				I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
				xferIndex = 0;
				I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0); //For the next one.
				MAP_Interrupt_disableSleepOnIsrExit();
			}
			else
			{
				RXData[xferIndex++] = I2C_masterReceiveMultiByteNext(EUSCI_B1_BASE);
			}
		}
	}
}

int main(void)
{

    volatile uint32_t z;

    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    // Set P1.0 to output direction
    GPIO_setAsOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN0
		);
	// Set P3.5 to output direction
	GPIO_setAsOutputPin(
		GPIO_PORT_P3,
		GPIO_PIN5
        );



	volatile int i, j, temp;
	volatile uint16_t test[NUM_ADC14_CHANNELS];
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();

/*  MAP_FPU_enableModule();
    MAP_FPU_enableLazyStacking();*/
/*
    FPU_disableModule();
    FPU_disableStacking();
*/

    //MAP_Interrupt_enableSleepOnIsrExit();


    while(!initializeBackChannelUART())
    {
		for(i=0;i<100;i++);  /*Wait to try again. */
		printf("Unsuccessful backChannelUARTinitialization");
	}

    while(!initializeADC())
    {
		for(i=0;i<100;i++); // Wait to try again.
		printf("Unsuccessful ADCinitialization");
	}

	Interrupt_enableMaster();

    while(!initCDCE())
    {
		for(i=0;i<100;i++);  /*Wait to try again. */
		printf("Unsuccessful CDCEinitialization");
	}

			//This tests the I2C for the versclock
     /* Enabling the FPU for floating point operation */
    while(!initializeDDS())
    {
    	for(i=0;i<100;i++);  //Wait to try again.
    }

    while(!initializeVersaclock())
    {
    	for(i=0;i<100;i++);  //Wait to try again.
    }

    while(!initializeI2C())
    {
    	for(i=0;i<100;i++);  //Wait to try again.
    }

    setDDSFrequency(1000000); // Test the DDS out.
//    initCDCE();
/*
    dumpI2C();
    printf("Dumping Versaclock RAM after setting to 1MHz.\n");
    printf("Address   Received    Written");
    for (i=0x10;i<NUM_OF_REG_BYTES+0x10;i++)
    {
    	printf("%#04x    %#04x       %#04x\n",i,RXData[i],
    			PllClockRegisters.init1MHzRegisterValues[i-0x10]);
    	if(RXData[i]!=PllClockRegisters.init1MHzRegisterValues[i-0x10])
    		printf("They don't match!\n");
    }

    setDDSFrequency(10000000);
    if(!updateVersaclockRegs(10000000))
    	printf("updateVersaClockRegs failed!\n");
  //  dumpI2C();

    printf("Address,  Read,    Tried to Write\n");
    for (i=0;i<NUM_OF_CHANGED_REG_BYTES; i++)
    {
    		printf("%#04x       %#04x        %#04x\n",PllClockRegisters.changedAddresses[i],
    				RXData[PllClockRegisters.changedAddresses[i]],
					PllClockRegisters.registerValues[12][i]);
    		if(RXData[PllClockRegisters.changedAddresses[i]]!=
    				PllClockRegisters.registerValues[12][i])
    			printf("VersaClock Registers did NOT match!\n");
    }
*/
    /* Main while loop */
	while(1)
	{

		//printf("This is the simplist function there is!\n");

	    // //Test LED light
	    GPIO_toggleOutputOnPin(
	        GPIO_PORT_P1,
			GPIO_PIN0
			);

	    // Delay
	    for(z=50000; z>0; z--);

/*		Pulse the start of a conversion.	*/
	    GPIO_toggleOutputOnPin(
	        GPIO_PORT_P3,
			GPIO_PIN5
			);
	    //Test VNA LED Output
	   // GPIO_setOutputHighOnPin();
		//MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN5);

		while(!MAP_ADC14_toggleConversionTrigger()){
			for(i=0;i<100;i++);  // Wait for conversion to finish.
		}

/* 		for(i=0;i<1000;i++){
			temp = i*temp;
		}*/
		printf("\r\n Results are:\r\n");
		for(i=0; i<NUM_ADC14_CHANNELS; i++){
			test[i] = resultsBuffer[i];
			printf("ADC # %d  \r\n",i);
			printf("Result: %d\n\r",resultsBuffer[i]);
		}
		//MAP_PCM_gotoLPM0();
	}
}

void initializeClocks(void)
{
    /* Initialize main clock to 48MHz.  To make it 3 MHz change the 48 to 3
     * and the 16's to 1.*/
	printf("Initialized clocks");
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48); // Full speed ahead! changes uart speed, baud rate, i2c clock speed
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_16 );
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_16 );
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_16 );
}

int initializeBackChannelUART(void){

	initializeClocks();
    /* Selecting P1.2 and P1.3 in UART mode. */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
        GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    if(MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig)==STATUS_FAIL)
    	return (STATUS_FAIL);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);

    /* Enable UART interrupts for backchannel UART
     * We may or may not need to do this.  The simple
     * printf() routine doesn't seem to use it.  */
    //UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);
    return 1;
}

int initializeADC(void){
    /* Initializing ADC (MCLK/1/1) */
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1,
            ADC_NOROUTE);
//All s parameter imnputs set for our board
    /* Configuring GPIOs for Analog In  (updated)
     * Pin 4.7 is S21_Im, A6
     * Pin 4.5 is S21_Re, A8
     * Pin 5.5 is S11_Re, A0
     * Pin 5.4 is S11_Im, A1  */

    //Zero-filling buffer ----test
    memset(resultsBuffer, 0x00, 8);

    	//Configure for Analog_in
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,
            GPIO_PIN4 | GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
            GPIO_PIN5 | GPIO_PIN7, GPIO_TERTIARY_MODULE_FUNCTION);//updated

    /* Configuring ADC Memory (ADC_MEM0 - ADC_MEM3, with A12, A10, A5, A3
     * with no repeat) with VCC and VSS reference */
    if(!ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM3, false))
    {
    		printf("Failed to initialize multi sequence.\r\n");
    		return(0);
    }
    if(!ADC14_configureConversionMemory(ADC_MEM0,
            ADC_VREFPOS_AVCC_VREFNEG_VSS,
            ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS))//updated for our board
    {
        		printf("Failed to initialize A0.\r\n");
        		return(0);
    }
    if(!MAP_ADC14_configureConversionMemory(ADC_MEM1,
            ADC_VREFPOS_AVCC_VREFNEG_VSS,
            ADC_INPUT_A1, ADC_NONDIFFERENTIAL_INPUTS))//updated
    {
        		printf("Failed to initialize A1.\r\n");
        		return(0);
    }
    if(!MAP_ADC14_configureConversionMemory(ADC_MEM2,
            ADC_VREFPOS_AVCC_VREFNEG_VSS,
            ADC_INPUT_A8, ADC_NONDIFFERENTIAL_INPUTS))
    {
        		printf("Failed to initialize A8.\r\n");
        		return(0);
    }
    if(!MAP_ADC14_configureConversionMemory(ADC_MEM3,
            ADC_VREFPOS_AVCC_VREFNEG_VSS,
            ADC_INPUT_A6, ADC_NONDIFFERENTIAL_INPUTS))
    {
        		printf("Failed to initialize A6.\r\n");
        		return(0);
    }

    /* Enabling the interrupt when a conversion on channel 3 (end of sequence)
     *  is complete and enabling conversions */
    ADC14_enableInterrupt(ADC_INT3);
    printf("Initialized interrupts.\r\n");


    /* Enabling Interrupts */
    Interrupt_enableInterrupt(INT_ADC14);

    /* Setting up the sample timer to automatically step through the sequence
     * convert.
     */
    if(!MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION))
    {
        		printf("Failed to initialize enable sample timer.\r\n");
        		return(0);
    }

    /* Enable conversion */
    if(!MAP_ADC14_enableConversion())
    {
        		printf("Failed to enable conversion.\r\n");
        		return(0);
    }

    return 1;
}

void pulseFQ_UD(void)
{
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);
}

void pulse_W_CLK(void)
{
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN5);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN5);
}

	//Reset signal for the DDS p4.4
void pulse_DDS_RST(void)
{
		//MAP_ takes the newest file from TI, withstanding updates and everything
		// works without MAP_ too
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4); //Fixed for our board (P4.4)
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4);

}
#ifndef USE_SPI
void transmit_DDS_Byte(uint8_t data)
{ // Bit banging method of sending data to DDS.
	int i;
	for (i=0; i<8; i++, data>>=1) {
		if(data & 0x01)
		{
			MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6);
		}
		else
		{
			MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN6);
		}
		pulse_W_CLK();
	}
}
#endif


int initializeDDS(void)
{
	/* This function initializes the DDS.  The first thing needed is to
	 * set the DDS into serial mode, which is done by pulsing W_CLK, and
	 * then FQ_UD.  Next the SPI is initialized.
	 */
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6|GPIO_PIN7); // Set FQ_UD, DDS_RST as output.
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN5|GPIO_PIN6); // Set W_CLK initially as output.
	pulse_DDS_RST();
	// Set DDS to serial mode.
	pulse_W_CLK();
	pulseFQ_UD();
	// This concludes setting DDS to serial.
#ifdef USE_SPI
	/* Selecting P1.5 P1.6 and P1.7 in SPI mode */
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
			GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION); // Not really using P1.7.
	/* Configuring SPI in 3wire master mode */
	MAP_SPI_initMaster(EUSCI_B0_BASE, &spiMasterConfig);
	/* Enable SPI module */
	MAP_SPI_enableModule(EUSCI_B0_BASE);
	/* Enabling interrupts */
	MAP_Interrupt_enableInterrupt(INT_EUSCIB0);
	MAP_Interrupt_enableSleepOnIsrExit();
#endif
	setDDSFrequency(0); // Safety for serial mode.
	printf("Initalized DDS");

	return 1; // Need to come back here and make these all if loops if the function supports it.
}

int setDDSFrequency(long long frequency)
{
	/* This function sets the AD9851 frequency using SPI for the data and clock
	 * and P4.6 as FQ_UD. It requires the DDS already be in serial mode, which
	 * is set in the intitialization, which also sets the P6.4 and P6.4 up to do
	 * SPI.  40 bits are sent (5 bytes via SPI) before FQ_UD is pulsed.  The
	 * AD9851 can handle data faster than we can send it via SPI.
	 */
	int i;
	unsigned long long tuning_word = roundl((frequency << 32) / 180000000);
	if((frequency<1000000)|(frequency>70000000)) return 1; // Frequency out of range.
#ifdef USE_SPI
	for (i=0;i<4;i++,tuning_word >>=8) // Send the frequency words
	{
		while (!(MAP_SPI_getInterruptStatus(EUSCI_B0_BASE,EUSCI_B_SPI_TRANSMIT_INTERRUPT)));
		MAP_SPI_transmitData ( EUSCI_B0_BASE, (uint8_t)((tuning_word)&(long long)0xff) );
	}
	MAP_SPI_transmitData ( EUSCI_B0_BASE, (uint8_t)(1) ); //O phase, 6X multiply
#else // Bitbanging below:
		for (i=0;i<4;i++,tuning_word >>=8) // Send the frequency words
		{
			transmit_DDS_Byte( (uint8_t)((tuning_word)&(long long)0xff) );
		}
		transmit_DDS_Byte( (uint8_t)(1) ); //O phase, 6X multiply
#endif
	pulseFQ_UD();
	return 1;
}

/*
 * Initialize Versaclock.
 */
int initializeVersaclock(void)
{
//specific to versaclock                                                                            !!!!
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN5|GPIO_PIN4); // Outputs for CKSEL and SD.

	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN4|GPIO_PIN5);
	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);

	 /* Wait for 10 ms for Versaclock to power up. */
	volatile long int i,j;
	for(i=0;i<6400; i++)  //Need to delay at least 10 ms.  This is just a guess at that.
	{
		j=j+100*j/(i+1)+i;
	}
	return 1;
}

int initializeI2C(void)//Updated to our pinout
{
    /* Select Port 6 for I2C - Set Pin 4, 5 to input Primary Module Function,
     *   (UCB1SIMO/UCB1SDA, UCB1SOMI/UCB1SCL).
     */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
            GPIO_PIN5 + GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);

//    memset(RXData, 0x00, NUM_OF_REG_BYTES+0x10); // Start with 0 so we can see the changes.

    /* Initializing I2C Master to parameters in i2cConfig */
    I2C_initMaster(EUSCI_B1_BASE, &i2cConfig);

    /* Specify slave address */
    I2C_setSlaveAddress(EUSCI_B1_BASE, SLAVE_ADDRESS);

    /* Set Master in transmit mode */
    I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

    /* Enable I2C Module to start operations */
    I2C_enableModule(EUSCI_B1_BASE);
    /* Enable and clear the interrupt flag */
    I2C_clearInterruptFlag(EUSCI_B1_BASE,
            EUSCI_B_I2C_TRANSMIT_INTERRUPT0 + EUSCI_B_I2C_RECEIVE_INTERRUPT0
			+ EUSCI_B_I2C_NAK_INTERRUPT);
    //Enable master Transmit interrupt
    I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0
    		+ EUSCI_B_I2C_NAK_INTERRUPT);
    //MAP_Interrupt_enableSleepOnIsrExit();
    Interrupt_enableInterrupt(INT_EUSCIB1);
    return 1;
}

/*
 *This routine dumps all the registers of the VersaCLock.
 */
void dumpI2C(void)// Checked for our board -ng
{
    /* Making sure the last transaction has been completely sent out */
    while ((I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP)|
    		(TXByteCtr!=0));

	justSending=false;  // We want to receive data back.

	/* See the following address for info as to why this is different that the example.
     * https://e2e.ti.com/support/microcontrollers/msp430/f/166/p/462976/1666648
     */
    I2C_masterSendMultiByteStart(EUSCI_B1_BASE, 0x00);

    /* Enabling transfer interrupt after stop has been sent */
    I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    /* While the stop condition hasn't been sent out... */
/*

    MAP_PCM_gotoLPM0InterruptSafe();
*/
}

/*
 * Initialize PllClockGen chip
 * Initially we want to write to: Byte1, Byte2, Byte3, Byte9, Byte13, and Byte 19.
 * Byte 1: PLL1 reference Divider
 * Byte 2: PLL1 Feedback Divider N
 * Byte 3: PLL Mux1-0, Pll Mux2-x, Pll Mux3-x, PLL1 Feedback Divider N-?
 * Byte 9: PLL Selection for P0
 * Byte 13: 7-bit divider P0
 * Byte 19: bits2,1,0-Y0 Divider selection.
 */
bool initCDCE(void)// Inititial
{

			//Experimental initialization
	writeVersaClockBlock(&(PllClockRegisters.init1MHzRegisterValues[1]), firstReg, 1);
	writeVersaClockBlock(&(PllClockRegisters.init1MHzRegisterValues[2]), 0x02, 1);
	writeVersaClockBlock(&(PllClockRegisters.init1MHzRegisterValues[3]), 0x03, 1);
	writeVersaClockBlock(&(PllClockRegisters.init1MHzRegisterValues[4]), 0x06, 1);
	writeVersaClockBlock(&(PllClockRegisters.init1MHzRegisterValues[5]), 0x09, 1);
	writeVersaClockBlock(&(PllClockRegisters.init1MHzRegisterValues[6]), 0x0d, 1);
	writeVersaClockBlock(&(PllClockRegisters.init1MHzRegisterValues[7]), 0x13, 1);

	return true;
}

/* Send a block of data, bumBytes long and store it in the I2C address at blockStart.
 * The firstDataPtr is a pointer to the memory address where the first data byte is.
 * The blockStart variable is the address in the versaclock where the data starts.
 * The numBytes is the numberof bytes to send.
 */

/* We need to write to Byte2, Byte6, and Byte9 often.
 *
 * */

void writeVersaClockBlock(const uint8_t *firstDataPtr, uint8_t blockStart, uint8_t numBytes)
{
	volatile int i;

	/* Making sure the last transaction has been completely sent out */
    while (I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);

	justSending=true;
	TXData = firstDataPtr;
    TXByteCtr = numBytes;
    xferIndex = 0;

    //Enable master Transmit interrupt
    I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0
    		+ EUSCI_B_I2C_NAK_INTERRUPT);

    I2C_masterSendMultiByteStart(EUSCI_B1_BASE, blockStart); // Send the address.

    printf("Versaclock block written\n\r");
}


int updateVersaclockRegs(long int frequency)
{
	int i,j, offset = 0;
	volatile int m;
	static int presentBandIndex=0;
	bool changed = false;
	frequency = frequency/1000;

	if((PllClockRegisters.frequencyBandLimit[presentBandIndex] <= frequency)&
			(frequency<PllClockRegisters.frequencyBandLimit[presentBandIndex+1])) return 0; //Exit if we don't need to update

		//If we actually need to update, for each block band...
	for(i=0;i<NUM_BANDS;i++)
	{
		if((PllClockRegisters.frequencyBandLimit[i] <= frequency)&
				(frequency<PllClockRegisters.frequencyBandLimit[i+1]))
		{
			presentBandIndex = i;  // We found the band.
			for(j=0;j<NUM_BAND_BLOCKS;j++)
			{
				writeVersaClockBlock(&(PllClockRegisters.registerValues[i][offset]),
						PllClockRegisters.blockFirstAddress[j],
						PllClockRegisters.blockNumBytes[j]);
				for(m=0;m<100;m++);
				offset += PllClockRegisters.blockNumBytes[j];
				changed = true;
			}
		}

	    printf("Versaclock registers updated.");

		if(changed == true)
		{return 0;}
	}
		//Indicates an error.
	return 1;  // We didn't find the band to fit the frequency.  Probably should do something with this error.
}




