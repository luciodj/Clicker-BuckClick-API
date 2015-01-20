/********************************************************************
 FileName:      main.c
 Dependencies:  See INCLUDES section
 Processor:     PIC18 USB Microcontrollers
 Hardware:      Mikroe PIC18 Clicker and Clicker2.
 Complier:      Microchip C18 
 
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

 ********************************************************************

/** INCLUDES *******************************************************/
#include <pps.h>
#include <USB/usb.h>
#include <USB/usb_function_cdc.h>
#include "HardwareProfile.h"

/** CONFIGURATION **************************************************/

#if defined(PIC18F87J50_Clicker2)
        #pragma config XINST    = OFF   	// Extended instruction set
        #pragma config STVREN   = ON      	// Stack overflow reset
        #pragma config PLLDIV   = 3         // (12 MHz crystal used on this board)
        #pragma config WDTEN    = OFF      	// Watch Dog Timer (WDT)
        #pragma config CP0      = OFF      	// Code protect
        #pragma config CPUDIV   = OSC1      // OSC1 = divide by 1 mode
        #pragma config IESO     = OFF      	// Internal External (clock) Switchover
        #pragma config FCMEN    = OFF      	// Fail Safe Clock Monitor
        #pragma config FOSC     = HSPLL     // Firmware must also set OSCTUNE<PLLEN> to start PLL!
        #pragma config WDTPS    = 32768
//      #pragma config WAIT     = OFF      	// Commented choices are
//      #pragma config BW       = 16      	// only available on the
//      #pragma config MODE     = MM      	// 80 pin devices in the 
//      #pragma config EASHFT   = OFF      	// family.
        #pragma config MSSPMSK  = MSK5
//      #pragma config PMPMX    = DEFAULT
//      #pragma config ECCPMX   = DEFAULT
        #pragma config CCP2MX   = DEFAULT   
        

#elif defined(PIC18F47J53_Clicker)
     #pragma config WDTEN = OFF          // WDT disabled (enabled by SWDTEN bit)
     #pragma config PLLDIV = 3           // Divide by 3 (12 MHz oscillator input)
     #pragma config STVREN = ON          // stack overflow/underflow reset enabled
     #pragma config XINST = OFF          // Extended instruction set disabled
     #pragma config CPUDIV = OSC1        // No CPU system clock divide
     #pragma config CP0 = OFF            // Program memory is not code-protected
     #pragma config OSC = HSPLL          // HS oscillator, PLL enabled, HSPLL used by USB
     //#pragma config T1DIG = ON         // Sec Osc clock source may be selected
     //#pragma config LPT1OSC = OFF        // high power Timer1 mode
     #pragma config FCMEN = OFF          // Fail-Safe Clock Monitor disabled
     #pragma config IESO = OFF           // Two-Speed Start-up disabled
     #pragma config WDTPS = 32768        // 1:32768
     #pragma config DSWDTOSC = INTOSCREF // DSWDT uses INTOSC/INTRC as clock
     #pragma config RTCOSC = T1OSCREF    // RTCC uses T1OSC/T1CKI as clock
     #pragma config DSBOREN = OFF        // Zero-Power BOR disabled in Deep Sleep
     #pragma config DSWDTEN = OFF        // Disabled
     #pragma config DSWDTPS = 8192       // 1:8,192 (8.5 seconds)
     #pragma config IOL1WAY = OFF        // IOLOCK bit can be set and cleared
     #pragma config MSSP7B_EN = MSK7     // 7 Bit address masking
     #pragma config WPFP = PAGE_1        // Write Protect Program Flash Page 0
     #pragma config WPEND = PAGE_0       // Start protection at page 0
     #pragma config WPCFG = OFF          // Write/Erase last page protect Disabled
     #pragma config WPDIS = OFF          // WPFP[5:0], WPEND, and WPCFG bits ignored

#else
    #error No hardware board defined, see "HardwareProfile.h" and __FILE__
#endif

/** I N C L U D E S **********************************************************/

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "usb_config.h"
#include "USB/usb_device.h"
#include "USB/usb.h"

#include "HardwareProfile.h"

/** V A R I A B L E S ********************************************************/
#pragma udata

char USB_Out_Buffer[ CDC_DATA_OUT_EP_SIZE];
char RS232_Out_Data[ CDC_DATA_IN_EP_SIZE];

unsigned char   NextUSBOut;
unsigned char   NextUSBOut;
//char RS232_In_Data;
unsigned char   LastRS232Out;  // Number of characters in the buffer
unsigned char   RS232cp;       // current position within the buffer
unsigned char   RS232_Out_Data_Rdy = 0;
USB_HANDLE      lastTransmission;

//BOOL stringPrinted;


/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void ProcessIO(void);
void USBDeviceTasks(void);
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();
void USBCBSendResume(void);
void BlinkUSBStatus(void);
void UserInit(void);
void InitializeUSART(void);
void putcUSART(char c);
unsigned char getcUSART ();

// Interrupt handling routines.
#pragma interrupt YourHighPriorityISRCode
void YourHighPriorityISRCode()
{
#if defined(USB_INTERRUPT)
        USBDeviceTasks();
#endif
}	

#pragma interruptlow YourLowPriorityISRCode
void YourLowPriorityISRCode()
{
}	


/******************************************************************************/
#pragma code

void main(void)
{   
    InitializeSystem();

    #if defined(USB_INTERRUPT)
        USBDeviceAttach();
    #endif

    while(1)
    {
        #if defined(USB_POLLING)
            // Check bus status and service USB interrupts.
            USBDeviceTasks(); // Interrupt or polling method.
        #endif			  

        // Application related code may be added here, or in the ProcessIO() function.
        ProcessIO();        
    }//end while
}//end main


/********************************************************************
 * Function:        static void InitializeSystem(void)
 *******************************************************************/
static void InitializeSystem(void)
{
#if (defined(__18CXX) & !defined(PIC18F87J50_Clicker2))
    ADCON1 |= 0x0F;                 // Default all pins to digital
#else
    AD1PCFGL = 0xFFFF;
#endif  

#if defined(PIC18F87J50_Clicker2) || defined(PIC18F47J53_Clicker)
    //On the PIC18F87J50 Family of USB microcontrollers, the PLL will not power up and be enabled
    //by default, even if a PLL enabled oscillator configuration is selected (such as HS+PLL).
    {
        unsigned int pll_startup_counter = 600;
        OSCTUNEbits.PLLEN = 1;  //Enable the PLL and wait 2+ms until the PLL locks before enabling USB module
        while(pll_startup_counter--);
    }
    //Device switches over automatically to PLL output after PLL is locked and ready.
#endif

#if defined(PIC18F87J50_Clicker2)
    //Configure all I/O pins to use digital input buffers.
    WDTCONbits.ADSHR = 1;   // Select alternate SFR location to access ANCONx registers
    ANCON0 = 0xFF;          // Default all pins to digital
    ANCON1 = 0xFF;          // Default all pins to digital
    WDTCONbits.ADSHR = 0;   // Select normal SFR locations
#endif

#if defined(PIC18F47J53_Clicker)
    //Configure all I/O pins to use digital input buffers.
    ANCON0 = 0xFF;          // All pins to digital
    ANCON1 = 0x9F;          // All pins to digital.  Bandgap on.

#endif
    
    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
    #endif
    
    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;	// See HardwareProfile.h
    #endif
    
    UserInit();

    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
    			//variables to known states.
}//end InitializeSystem



/******************************************************************************
 * Function:        void UserInit(void)
 *****************************************************************************/
void UserInit(void)
{
    unsigned char i;
    InitializeUSART();

//  Initialize the arrays
    for (i=0; i<sizeof(USB_Out_Buffer); i++)
    {
        USB_Out_Buffer[i] = 0;
    }

    NextUSBOut = 0;
    LastRS232Out = 0;
    lastTransmission = 0;

    mInitAllLEDs();
    mInitAllSwitches();

    // init RST and CS as output pins (default high)
    mInitMikroBus();
}//end UserInit

/******************************************************************************
 * Function:        void InitializeUSART(void)
 * Overview:        This routine initializes the UART to 19200
 *****************************************************************************/
void InitializeUSART(void)
{
    unsigned char c;
    // use UART2

//    UART_TRISRx=1;		// RX   input   -- automatic
    PPSUnLock();
    PPSInput( PPS_RX2DT2, PPS_RP22);
    UART_TRISTx=0;		// TX   output  -- automatic
    PPSOutput( PPS_RP23, PPS_TX2CK2);

    TXSTA2 = 0x24;       	// TX enable, BRGH=1
    RCSTA2 = 0x90;       	// RX enable
    SPBRG2 = 0x71;
    SPBRGH2 = 0x02;      	// 0x0271 for 48MHz (/4) -> 19200 baud
    BAUDCON2 = 0x08;     	// select 16-bit baud rate generator (BRG16=1)
    c = RCREG2;			// read
}// InitializeUSART

#define mDataRdyUSART() PIR3bits.RC2IF
#define mTxRdyUSART()   TXSTA2bits.TRMT

/******************************************************************************/
void putcUSART(char c)  
{
    TXREG2 = c;
}


/******************************************************************************
 * Function:        void mySetLineCodingHandler(void)
 *
 * PreCondition:    USB_CDC_SET_LINE_CODING_HANDLER is defined
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function gets called when a SetLineCoding command
 *                  is sent on the bus.  This function will evaluate the request
 *                  and determine if the application should update the baudrate
 *                  or not.
 *
 * Note:            
 *
 *****************************************************************************/
#if defined(USB_CDC_SET_LINE_CODING_HANDLER)
void mySetLineCodingHandler(void)
{
    //If the request is not in a valid range
    if(cdc_notice.GetLineCoding.dwDTERate.Val > 115200)
    {
        //NOTE: There are two ways that an unsupported baud rate could be
        //handled.  The first is just to ignore the request and don't change
        //the values.  That is what is currently implemented in this function.
        //The second possible method is to stall the STATUS stage of the request.
        //STALLing the STATUS stage will cause an exception to be thrown in the 
        //requesting application.  Some programs, like HyperTerminal, handle the
        //exception properly and give a pop-up box indicating that the request
        //settings are not valid.  Any application that does not handle the
        //exception correctly will likely crash when this requiest fails.  For
        //the sake of example the code required to STALL the status stage of the
        //request is provided below.  It has been left out so that this demo
        //does not cause applications without the required exception handling
        //to crash.
        //---------------------------------------
        //USBStallEndpoint(0,1);
    }
    else
    {
        //Update the baudrate info in the CDC driver
        CDCSetBaudRate(cdc_notice.GetLineCoding.dwDTERate.Val);
        
        //Update the baudrate of the UART
        {
            DWORD_VAL dwBaud;
            dwBaud.Val = (DWORD)(GetSystemClock()/4)/line_coding.dwDTERate.Val-1;
            SPBRG2 = dwBaud.v[0];
            SPBRGH2 = dwBaud.v[1];
        }    
    }
}
#endif

/******************************************************************************/
unsigned char getcUSART ()
{
    char  c;

    if (RCSTA2bits.OERR)  // in case of overrun error
    {                    // we should never see an overrun error, but if we do,
        RCSTA2bits.CREN = 0;  // reset the port
        c = RCREG2;
        RCSTA2bits.CREN = 1;  // and keep going.
    }
    else
    {
        c = RCREG2;
    }
// not necessary.  EUSART auto clears the flag when RCREG is cleared
//	PIR1bits.RCIF = 0;    // clear Flag
    return c;
}

/********************************************************************
 * Function:        void ProcessIO(void)
 *
 *******************************************************************/
void ProcessIO(void)
{   
    // copy the status of the left and right button to the MCLR and CS pins
    mRST = swL;
    mCS = swR;

    //Blink the LEDs according to the USB device status
    BlinkUSBStatus();
    // User Application USB tasks
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;

    if (RS232_Out_Data_Rdy == 0)    // only check for new USB buffer if the old RS232 buffer is
    {                               // empty.  This will cause additional USB packets to be NAK'd
        LastRS232Out = getsUSBUSART(RS232_Out_Data,64); //until the buffer is free.
        if(LastRS232Out > 0)
        {
            RS232_Out_Data_Rdy = 1; // signal buffer full
            RS232cp = 0;            // Reset the current position
        }
    }

    //Check if one or more bytes are waiting in the physical UART transmit
    //queue.  If so, send it out the UART TX pin.
    if(RS232_Out_Data_Rdy && mTxRdyUSART())
    {
    	#if defined(USB_CDC_SUPPORT_HARDWARE_FLOW_CONTROL)
            //Make sure the receiving UART device is ready to receive data before
            //actually sending it.
            if(UART_CTS == USB_CDC_CTS_ACTIVE_LEVEL)
            {
                putcUSART(RS232_Out_Data[RS232cp]);
                ++RS232cp;
                if (RS232cp == LastRS232Out)
                    RS232_Out_Data_Rdy = 0;
    	    }
        #else
            //Hardware flow control not being used.  Just send the data.
            putcUSART(RS232_Out_Data[RS232cp]);
            ++RS232cp;
            if (RS232cp == LastRS232Out)
                RS232_Out_Data_Rdy = 0;
        #endif
    }

    //Check if we received a character over the physical UART, and we need
    //to buffer it up for eventual transmission to the USB host.
    if(mDataRdyUSART() && (NextUSBOut < (CDC_DATA_OUT_EP_SIZE - 1)))
    {
        USB_Out_Buffer[NextUSBOut] = getcUSART();
        ++NextUSBOut;
        USB_Out_Buffer[NextUSBOut] = 0;
    }
	
    #if defined(USB_CDC_SUPPORT_HARDWARE_FLOW_CONTROL)
    //Drive RTS pin, to let UART device attached know if it is allowed to
    //send more data or not.  If the receive buffer is almost full, we
    //deassert RTS.
    if(NextUSBOut <= (CDC_DATA_OUT_EP_SIZE - 5u))
    {
        UART_RTS = USB_CDC_RTS_ACTIVE_LEVEL;
    }
    else
    {
        UART_RTS = (USB_CDC_RTS_ACTIVE_LEVEL ^ 1);
    }
    #endif	

    //Check if any bytes are waiting in the queue to send to the USB host.
    //If any bytes are waiting, and the endpoint is available, prepare to
    //send the USB packet to the host.
    if((USBUSARTIsTxTrfReady()) && (NextUSBOut > 0))
    {
            putUSBUSART(&USB_Out_Buffer[0], NextUSBOut);
            NextUSBOut = 0;
    }

    CDCTxService();

}//end ProcessIO

/********************************************************************
 * Function:        void BlinkUSBStatus(void)
 *
 *******************************************************************/
void BlinkUSBStatus(void)
{
    static WORD led_count=0;
    
    if(led_count == 0)led_count = 10000U;
    led_count--;

    #define mLED_Both_Off()         {mLED_1_Off();mLED_2_Off();}
    #define mLED_Both_On()          {mLED_1_On();mLED_2_On();}
    #define mLED_Only_1_On()        {mLED_1_On();mLED_2_Off();}
    #define mLED_Only_2_On()        {mLED_1_Off();mLED_2_On();}

    if(USBSuspendControl == 1)
    {
        if(led_count==0)
        {
            mLED_1_Toggle();
            if(mGetLED_1())
            {
                mLED_2_On();
            }
            else
            {
                mLED_2_Off();
            }
        }//end if
    }
    else
    {
        if(USBDeviceState == DETACHED_STATE)
        {
            mLED_Both_Off();
        }
        else if(USBDeviceState == ATTACHED_STATE)
        {
            mLED_Both_On();
        }
        else if(USBDeviceState == POWERED_STATE)
        {
            mLED_Only_1_On();
        }
        else if(USBDeviceState == DEFAULT_STATE)
        {
            mLED_Only_2_On();
        }
        else if(USBDeviceState == ADDRESS_STATE)
        {
            if(led_count == 0)
            {
                mLED_1_Toggle();
                mLED_2_Off();
            }//end if
        }
        else if(USBDeviceState == CONFIGURED_STATE)
        {
            if(led_count==0)
            {
                mLED_1_Toggle();
                if(mGetLED_1())
                {
                    mLED_2_Off();
                }
                else
                {
                    mLED_2_On();
                }
            }//end if
        }//end if(...)
    }//end if(UCONbits.SUSPND...)

}//end BlinkUSBStatus


// ******************************************************************************************************
// ************** USB Callback Functions ****************************************************************
// ******************************************************************************************************
// The USB firmware stack will call the callback functions USBCBxxx() in response to certain USB related
// events.  For example, if the host PC is powering down, it will stop sending out Start of Frame (SOF)
// packets to your device.  In response to this, all USB devices are supposed to decrease their power
// consumption from the USB Vbus to <2.5mA* each.  The USB module detects this condition (which according
// to the USB specifications is 3+ms of no bus activity/SOF packets) and then calls the USBCBSuspend()
// function.  You should modify these callback functions to take appropriate actions for each of these
// conditions.  For example, in the USBCBSuspend(), you may wish to add code that will decrease power
// consumption from Vbus to <2.5mA (such as by clock switching, turning off LEDs, putting the
// microcontroller to sleep, etc.).  Then, in the USBCBWakeFromSuspend() function, you may then wish to
// add code that undoes the power saving things done in the USBCBSuspend() function.

// The USBCBSendResume() function is special, in that the USB stack will not automatically call this
// function.  This function is meant to be called from the application firmware instead.  See the
// additional comments near the function.

// Note *: The "usb_20.pdf" specs indicate 500uA or 2.5mA, depending upon device classification. However,
// the USB-IF has officially issued an ECN (engineering change notice) changing this to 2.5mA for all 
// devices.  Make sure to re-download the latest specifications to get all of the newest ECNs.

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Call back that is invoked when a USB suspend is detected
 *
 * Note:            None
 *****************************************************************************/
void USBCBSuspend(void)
{
}


/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The host may put USB peripheral devices in low power
 *			suspend mode (by "sending" 3+ms of idle).  Once in suspend
 *			mode, the host may wake the device back up by sending non-
 *			idle state signalling.
 *				
 *		This call back is invoked when a wakeup from USB suspend 
 *			is detected.
 *
 * Note:            None
 *****************************************************************************/
void USBCBWakeFromSuspend(void)
{
}

/********************************************************************
 * Function:        void USBCB_SOF_Handler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB host sends out a SOF packet to full-speed
 *                  devices every 1 ms. This interrupt may be useful
 *                  for isochronous pipes. End designers should
 *                  implement callback routine as necessary.
 *
 * Note:            None
 *******************************************************************/
void USBCB_SOF_Handler(void)
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The purpose of this callback is mainly for
 *                  debugging during development. Check UEIR to see
 *                  which error causes the interrupt.
 *
 * Note:            None
 *******************************************************************/
void USBCBErrorHandler(void)
{
}


/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 *
 * Note:            None
 *******************************************************************/
void USBCBCheckOtherReq(void)
{
    USBCheckCDCRequest();
}//end


/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USBCBStdSetDscHandler() callback function is
 *					called when a SETUP, bRequest: SET_DESCRIPTOR request
 *					arrives.  Typically SET_DESCRIPTOR requests are
 *					not used in most applications, and it is
 *					optional to support this type of request.
 *
 * Note:            None
 *******************************************************************/
void USBCBStdSetDscHandler(void)
{
}//end


/*******************************************************************
 * Function:        void USBCBInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the device becomes
 *                  initialized, which occurs after the host sends a
 * 					SET_CONFIGURATION (wValue not = 0) request.  This 
 *					callback function should initialize the endpoints 
 *					for the device's usage according to the current 
 *					configuration.
 *
 * Note:            None
 *******************************************************************/
void USBCBInitEP(void)
{
    //Enable the CDC data endpoints
    CDCInitEP();
}

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB specifications allow some types of USB
 * 					peripheral devices to wake up a host PC (such
 *					as if it is in a low power suspend to RAM state).
 *					This can be a very useful feature in some
 *					USB applications, such as an Infrared remote
 *					control	receiver.  If a user presses the "power"
 *					button on a remote control, it is nice that the
 *					IR receiver can detect this signalling, and then
 *					send a USB "command" to the PC to wake up.
 *					
 *					The USBCBSendResume() "callback" function is used
 *					to send this special USB signalling which wakes 
 *					up the PC.  This function may be called by
 *					application firmware to wake up the PC.  This
 *					function will only be able to wake up the host if
 *                  all of the below are true:
 *					
 *					1.  The USB driver used on the host PC supports
 *						the remote wakeup capability.
 *					2.  The USB configuration descriptor indicates
 *						the device is remote wakeup capable in the
 *						bmAttributes field.
 *					3.  The USB host PC is currently sleeping,
 *						and has previously sent your device a SET 
 *						FEATURE setup packet which "armed" the
 *						remote wakeup capability.   
 *
 *                  If the host has not armed the device to perform remote wakeup,
 *                  then this function will return without actually performing a
 *                  remote wakeup sequence.  This is the required behavior, 
 *                  as a USB device that has not been armed to perform remote 
 *                  wakeup must not drive remote wakeup signalling onto the bus;
 *                  doing so will cause USB compliance testing failure.
 *                  
 *					This callback should send a RESUME signal that
 *                  has the period of 1-15ms.
 *
 * Note:            This function does nothing and returns quickly, if the USB
 *                  bus and host are not in a suspended condition, or are 
 *                  otherwise not in a remote wakeup ready state.  Therefore, it
 *                  is safe to optionally call this function regularly, ex: 
 *                  anytime application stimulus occurs, as the function will
 *                  have no effect, until the bus really is in a state ready
 *                  to accept remote wakeup. 
 *
 *                  When this function executes, it may perform clock switching,
 *                  depending upon the application specific code in 
 *                  USBCBWakeFromSuspend().  This is needed, since the USB
 *                  bus will no longer be suspended by the time this function
 *                  returns.  Therefore, the USB module will need to be ready
 *                  to receive traffic from the host.
 *
 *                  The modifiable section in this routine may be changed
 *                  to meet the application needs. Current implementation
 *                  temporary blocks other functions from executing for a
 *                  period of ~3-15 ms depending on the core frequency.
 *
 *                  According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at least 1 ms but for no more than 15 ms."
 *                  The idea here is to use a delay counter loop, using a
 *                  common value that would work over a wide range of core
 *                  frequencies.
 *                  That value selected is 1800. See table below:
 *                  ==========================================================
 *                  Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 *                  ==========================================================
 *                      48              12          1.05
 *                       4              1           12.6
 *                  ==========================================================
 *                  * These timing could be incorrect when using code
 *                    optimization or extended instruction mode,
 *                    or when having other interrupts enabled.
 *                    Make sure to verify using the MPLAB SIM's Stopwatch
 *                    and verify the actual signal on an oscilloscope.
 *******************************************************************/
void USBCBSendResume(void)
{
    static WORD delay_count;
    
    //First verify that the host has armed us to perform remote wakeup.
    //It does this by sending a SET_FEATURE request to enable remote wakeup,
    //usually just before the host goes to standby mode (note: it will only
    //send this SET_FEATURE request if the configuration descriptor declares
    //the device as remote wakeup capable, AND, if the feature is enabled
    //on the host (ex: on Windows based hosts, in the device manager 
    //properties page for the USB device, power management tab, the 
    //"Allow this device to bring the computer out of standby." checkbox 
    //should be checked).
    if(USBGetRemoteWakeupStatus() == TRUE) 
    {
        //Verify that the USB bus is in fact suspended, before we send
        //remote wakeup signalling.
        if(USBIsBusSuspended() == TRUE)
        {
            USBMaskInterrupts();
            
            //Clock switch to settings consistent with normal USB operation.
            USBCBWakeFromSuspend();
            USBSuspendControl = 0; 
            USBBusIsSuspended = FALSE;  //So we don't execute this code again, 
                                        //until a new suspend condition is detected.

            //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
            //device must continuously see 5ms+ of idle on the bus, before it sends
            //remote wakeup signalling.  One way to be certain that this parameter
            //gets met, is to add a 2ms+ blocking delay here (2ms plus at 
            //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yeilds
            //5ms+ total delay since start of idle).
            delay_count = 3600U;        
            do
            {
                delay_count--;
            }while(delay_count);
            
            //Now drive the resume K-state signalling onto the USB bus.
            USBResumeControl = 1;       // Start RESUME signaling
            delay_count = 1800U;        // Set RESUME line for 1-13 ms
            do
            {
                delay_count--;
            }while(delay_count);
            USBResumeControl = 0;       //Finished driving resume signalling

            USBUnmaskInterrupts();
        }
    }
}


/*******************************************************************
 * Function:        void USBCBEP0DataReceived(void)
 *
 * PreCondition:    ENABLE_EP0_DATA_RECEIVED_CALLBACK must be
 *                  defined already (in usb_config.h)
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called whenever a EP0 data
 *                  packet is received.  This gives the user (and
 *                  thus the various class examples a way to get
 *                  data that is received via the control endpoint.
 *                  This function needs to be used in conjunction
 *                  with the USBCBCheckOtherReq() function since 
 *                  the USBCBCheckOtherReq() function is the apps
 *                  method for getting the initial control transfer
 *                  before the data arrives.
 *
 * Note:            None
 *******************************************************************/
#if defined(ENABLE_EP0_DATA_RECEIVED_CALLBACK)
void USBCBEP0DataReceived(void)
{
}
#endif

/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        USB_EVENT event, void *pdata, WORD size)
 *
 * PreCondition:    None
 *
 * Input:           USB_EVENT event - the type of event
 *                  void *pdata - pointer to the event data
 *                  WORD size - size of the event data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called from the USB stack to
 *                  notify a user application that a USB event
 *                  occured.  This callback is in interrupt context
 *                  when the USB_INTERRUPT option is selected.
 *
 * Note:            None
 *******************************************************************/
BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size)
{
    switch(event)
    {
        case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_CONFIGURED: 
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            //Add application specific callback task or callback function here if desired.
            //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            //FEATURE (endpoint halt) request on an application endpoint which was 
            //previously armed (UOWN was = 1).  Here would be a good place to:
            //1.  Determine which endpoint the transaction that just got terminated was 
            //      on, by checking the handle value in the *pdata.
            //2.  Re-arm the endpoint if desired (typically would be the case for OUT 
            //      endpoints).
            break;
        default:
            break;
    }      
    return TRUE; 
}

/** EOF main.c *************************************************/
