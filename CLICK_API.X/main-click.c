/********************************************************************
 FileName:      main-click.c
 Processor:     PIC18F47J53
 Hardware:      Mikroe PIC18 Clicker
 Complier:      Microchip C18 (for PIC18), 
 
********************************************************************/

#include "./USB/usb.h"
#include "HardwareProfile.h"
#include "./USB/usb_function_hid.h"
#include "pps.h"

// PIC18F47J53 configuration
 #pragma config WDTEN = OFF          // WDT disabled (enabled by SWDTEN bit)
 #pragma config PLLDIV = 4           // Divide by 4 (16 MHz oscillator input)
 #pragma config STVREN = ON          // stack overflow/underflow reset enabled
 #pragma config XINST = OFF          // Extended instruction set disabled
 #pragma config CPUDIV = OSC4_PLL6   // CPU system clock divide 
 #pragma config CP0 = OFF            // Program memory is not code-protected
 #pragma config OSC = HSPLL          // HS oscillator, PLL enabled, HSPLL used by USB
 #pragma config FCMEN = ON           // Fail-Safe Clock Monitor disabled
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
 #pragma config CFGPLLEN = OFF


/** VARIABLES ******************************************************/
#pragma udata

#define RX_DATA_BUFFER_ADDRESS
#define TX_DATA_BUFFER_ADDRESS

unsigned char RxBuffer[64] RX_DATA_BUFFER_ADDRESS;
unsigned char TxBuffer[64] TX_DATA_BUFFER_ADDRESS;

USB_HANDLE USBOutHandle = 0;    //USB handle.  Must be initialized to 0 at startup.
USB_HANDLE USBInHandle = 0;     //USB handle.  Must be initialized to 0 at startup.
BOOL blinkStatusValid = TRUE;

/** PRIVATE PROTOTYPES *********************************************/
void BlinkUSBStatus(void);
void High_ISR(void);
static void InitializeSystem(void);
void ProcessIO(void);
void USBCBSendResume(void);
WORD_VAL ReadPOT(void);

#pragma code HIGH_INTERRUPT_VECTOR = 0x08
void High_Vector (void)
{
     _asm goto High_ISR _endasm
}

#pragma code
#pragma interrupt High_ISR
void High_ISR (void)
{
    //#if defined(USB_INTERRUPT)
    USBDeviceTasks();
}

/** DECLARATIONS ***************************************************/

void main(void)
{   
    InitializeSystem();

    //#if defined(USB_INTERRUPT)
    USBDeviceAttach();

    while(1)
    {
        //Blink the LEDs according to the USB device status and mirror INT line
        if(blinkStatusValid)
        {
            BlinkUSBStatus();   

            // mirror the INT (RD4 line)
            if ( PORTDbits.RD4) mLED_2_On()
            else    mLED_2_Off();
        }

        // if connected and active
        if((USBDeviceState == CONFIGURED_STATE)&&(USBSuspendControl!=1))
            ProcessIO();
    } // end while
} // end main


/********************************************************************
 * Function:        void InitializeSystem(void)
 *******************************************************************/
void InitializeSystem(void)
{

    // On the PIC18FxxJ5x Family of USB microcontrollers, the PLL will not power up and be enabled
    // by default, even if a PLL enabled oscillator configuration is selected (such as HS+PLL).
    // This allows the device to power up at a lower initial operating frequency, which can be
    // advantageous when powered from a source which is not gauranteed to be adequate for 48MHz
    // operation.  On these devices, user firmware needs to manually set the OSCTUNE<PLLEN> bit to
    // power up the PLL.
    {
        unsigned int pll_startup_counter = 600;
        OSCTUNEbits.PLLEN = 1;  //Enable the PLL and wait 2+ms until the PLL locks before enabling USB module
        while(pll_startup_counter--);
    }
    //Device switches over automatically to PLL output after PLL is locked and ready.

    //use the ANCONx registers to control this, which is different from other devices which
    ANCON0 = 0xFB;                  // All pins to digital (except AN2)
    ANCON1 = 0xBF;                  // Default all pins to digital.  Bandgap on.

    //Initialize all of the LED pins
    mInitAllLEDs();

    //Initialize all of the push buttons
    mInitAllSwitches();

    //Initialize I/O pin and ADC settings to collect analog measurements
    mInitADC();

    // Initialize the PWM port
    P_PWM = 0;
    T_PWM = 1;      // make it an input temporarily 
    P_SCL = 0;
    T_SCL = 1;      // make it an input temporarily

    //initialize the variable holding the handle for the last transmission
    USBOutHandle = 0;
    USBInHandle = 0;

    blinkStatusValid = TRUE;
    
    USBDeviceInit();	// Initializes USB module SFRs and firmware
    				
} // end InitializeSystem


// I2C2 utilities --------------------------------------------------------------
#define TIMEOUT         2000
#define NotAckI2C2()    SSP2CON2bits.ACKDT=1, SSP2CON2bits.ACKEN=1;
#define AckI2C2()       SSP2CON2bits.ACKDT=0, SSP2CON2bits.ACKEN=1;
#define CloseI2C2()     SSP2CON1 &= 0xDF

char IdleI2C2TO( void)
{
    int i;
    for( i=0; i<TIMEOUT; i++)      // wait maximum 1000 loops (approx 1ms)
    {
        if ( ((SSP2CON2 & 0x1F) | (SSP2STATbits.R_NOT_W)) == 0)
            return 0;   // succesfully completed
    }

    // timeout
    CloseI2C2();        // disable I2C module
    return 1;           // return timeout
} // Idle I2C2 TO


char ReadI2C2TO( void)
{
    int i,j;

    SSP2CON2bits.RCEN = 1;       // enable master for 1 byte reception
    for (i=0; i<TIMEOUT; i++)
    {
        if ( SSP2CON2bits.RCEN == 0)   // wait until byte received
            return 0;   // successfully received
    }

    // timeout

    SSP2CON2bits.PEN = 1;       // STOP
    for( j=0; j<100; j++)
        Nop();

    CloseI2C2();                // disable I2C2
    return 1;
} // read I2C2 TO


/********************************************************************
 * Function:        void ProcessIO(void)
 *
 * Description:     decodes commands based on first byte
 *                  commands 80, 81 and 37 are reserved for legacy
 * Response:        echo command if successful
 *                  00   if unrecognized
 *                  Fx   error code 
 *******************************************************************/
void ProcessIO(void)
{   
    WORD_VAL w;
    unsigned char i;
    unsigned char Buffer[64];

    //Check if we have received an OUT data packet from the host
    if( !HIDRxHandleBusy( USBOutHandle))
    {   
        // default echo back the command
        Buffer[0] = RxBuffer[0];

        //Check the first byte of the packet 
        switch( RxBuffer[0])
        {
// legacy commands 80, 81, 37 are for MCHP Simple HID Demo compatibility
            
            // Toggle LEDs command
            case 0x80:
		blinkStatusValid = FALSE;       // Stop blinking the LEDs automatically
                mLED_1_Toggle();
                mLED_2_Toggle();
                // note: legacy command, no reply provided!!!
                break;

            // Get push button state
            case 0x81:
                Buffer[1] = sw2;
                Buffer[2] = sw3;
                break;

            // Read AN (RA2) command
            case 0x37:
                w = ReadPOT();          // Use ADC to read the I/O pin voltage.
                Buffer[1] = w.v[0];  	// Measured analog voltage LSB
                Buffer[2] = w.v[1];  	// Measured analog voltage MSB
                break;

// new commands ----------------------------------------------------------------
            case 0x82:  // Set LED
                blinkStatusValid = FALSE;   // stop blinking if still on
                mLED_1 = RxBuffer[1];
                mLED_2 = RxBuffer[2];
                break;


// RST configure, default high (OD) output
            case 0x10:
                ANCON1bits.PCFG10 = 0;      // configure as digital
                T_RST = 1;                  // configure as input (open drain simulation)
                P_RST = 0;                  // prepare the reset line low
                break;

            // RST pulse (OD simulation)
            case 0x11:
                T_RST = 0;                  // make it an output temp (oD simulation)
            for( i=0; i<120; i++)           // ensure > 120 Tcy (~ 10us)
                Nop();
                T_RST = 1;                  // release the OD
                break;

            // RST write
            case 0x12:
                P_RST = RxBuffer[1];
                break;

                // RST read
            case 0x13:
                Buffer[1] = P_RST;
                break;

// CS configure, default high output
            case 0x14:
                T_CS = 0;                   // make CS an output
                P_CS = 1;                   // default high
                break;

            // CS read
            case 0x15:
                i = P_CS;                   // read I/O pin voltage
                Buffer[1] = i;              // return value
                break;

            // CS write
            case 0x16:
                P_CS = RxBuffer[1];         // use LSB of first argument
                break;


// INT configure, arg 1: 1/0  input/output, arg 2: state
            case 0x17:
                P_INT = RxBuffer[2];        // set the output latch
                T_INT = RxBuffer[1];        // make INT an 1 input / 0 output
                break;

            // INT Write
            case 0x18:
                P_INT = RxBuffer[1];
                break;

            // INT read
            case 0x19:
                Buffer[1] = P_INT;
                break;

// PWM configure 0-9 [period] [0][1][2][3][4][5][6][7][8][9]
//           1  uB   RB2 - CCP1 RP5
//           2  uB   RB3 - CCP2 RP6
//           3  uB   RB4 - CCP4
//           4  uB   RB5 - CCP5
//           5  uB   RC6 - CCP9
//           6  uB   RC7 - CCP10
//           7  HD2  RC0 - CCP3 RP11
//           8  HD2  RC1 - CCP8
//           *  HD2  RC2 - CCP1/2/3 RP13
//           9  icsp RB6 - CCP6
//          10  icsp RB7 - CCP7
            case 0x50:
                // all ECCP sourced from TMR1, default
                
                // all CCP sourced from TMR2, default
                T2CON = 0b00000110;                     // Fosc/4 /16
                PR2 = 255;    // this could be too fast @12MHz, also try INTOSC

                if (RxBuffer[1]){
                    CCP1CON = 0x0c;                     // single PWM P1A
                    PPSOutput( PPS_RP5, PPS_CCP1P1A);
                    TRISBbits.TRISB2 = 0;
                }

                if (RxBuffer[2]){
                    CCP2CON = 0x0c;
                    PPSOutput( PPS_RP6, PPS_CCP2P2A);
                    TRISBbits.TRISB3 = 0;
                }

                if (RxBuffer[3]){
                    CCP4CON = 0x0c;
                    TRISBbits.TRISB4 = 0;
                }

                if (RxBuffer[4]){
                    CCP5CON = 0x0c;
                    TRISBbits.TRISB5 = 0;
                }

                if (RxBuffer[5]){
                    CCP9CON = 0x0c;
                    TRISCbits.TRISC6 = 0;
                }

                if (RxBuffer[6]){
                    CCP10CON = 0x0c;
                    TRISCbits.TRISC7 = 0;
                }

                if (RxBuffer[7]){
                    CCP3CON = 0x0c;
                    PPSOutput( PPS_RP11, PPS_CCP3P3A);
                    TRISCbits.TRISC0 = 0;
                }

                if (RxBuffer[8]){
                    CCP8CON = 0x0c;
                    TRISCbits.TRISC1 = 0;
                }

                if (RxBuffer[9]){
                    CCP6CON = 0x0c;
                    TRISBbits.TRISB6 = 0;
                }

                if (RxBuffer[10]){
                    CCP7CON = 0x0c;
                    TRISBbits.TRISB7 = 0;
                }
                break;

//             read DC 0-9
            case 0x51:
                Buffer[1] = CCPR1L;
                Buffer[2] = CCPR2L;
                Buffer[3] = CCPR4L;
                Buffer[4] = CCPR5L;
                Buffer[5] = CCPR9L;
                Buffer[6] = CCPR10L;
                Buffer[7] = CCPR3L;
                Buffer[8] = CCPR8L;
                Buffer[9] = CCPR6L;
                Buffer[10]= CCPR7L;

//             PWM set All DC 0-9
            case 0x52:
                CCPR1L = RxBuffer[1];
                CCPR2L = RxBuffer[2];
                CCPR4L = RxBuffer[3];
                CCPR5L = RxBuffer[4];
                CCPR9L = RxBuffer[5];
                CCPR10L= RxBuffer[6];
                CCPR3L = RxBuffer[7];
                CCPR8L = RxBuffer[8];
                CCPR6L = RxBuffer[9];
                CCPR7L = RxBuffer[10];
                break;
//          PWM set individual


// SIDE port configure
            // SIDE port write
            // SIDE port read digital
            // SIDE port read analog
//          1   GND
//          2   Vdd
//          3   RE0 AN5
//          4   RE1 AN6
//          5   RE2 AN7
//          6   RB0 AN12 RP3
//          7   RB1 AN10 RP4
//          8   RC0 RP11
//          9   RC1 CCP8 RP12
//         10   RC2 AN11 CCP1/2/3 RP13

            // PING config
            // PING read

            // UART configure
            // TX  buffer
            // Get RX buffer
            // Get Status

// I2C commands 0x2X -----------------------------------------------------------

            case 0x20:      // I2C configure [no param]
                // set default 100kHz BRate, master mode, configure SCL(RD0), SDA(RD1)
                SSP2ADD = 119;              // BRate = (Fosc/(4*SSPADD+1))
                SSP2CON1 = 0b00101000;      // I2C2 master (~100kHz @4MHz)
                T_SCL = 1;                  // ensure SCL is set as input
                T_SDA = 1;                  // ensure SDA is set as input
                P_PWM = 0;
                break;


            case 0x21:      // I2C simple read[ add/cmd, length]
                // send a read command
                SSP2CON2bits.SEN = 1;       // START
                if ( IdleI2C2TO())  goto I2CStuck;

                SSP2BUF =  RxBuffer[1];         // send the address/comand byte
                if ( IdleI2C2TO())  goto I2CStuck;

                if ( SSP2CON2bits.ACKSTAT != 0)
                    goto I2CNack;               // exit if it did not receive ACK

                // read data in
                i = 1;
                Buffer[0] = RxBuffer[0];        // Echo back the command
                while (RxBuffer[2]--)
                {
                    if ( ReadI2C2TO())
                        goto I2CStuck;

                    Buffer[i++] = SSP2BUF;
                    if ( RxBuffer[2]> 0)    
                    {
                        AckI2C2();   // request more
                        if ( IdleI2C2TO())  goto I2CStuck;
                    }
                }
                NotAckI2C2();               // signal finished reading
                if ( IdleI2C2TO())  goto I2CStuck;

                SSP2CON2bits.PEN = 1;       // STOP terminate successfully
                if ( IdleI2C2TO())  goto I2CStuck;
                break;


            case 0x22:      // I2C simple write [ add/cmd, length, byte1, byte2...]
                // send a write command
                SSP2CON2bits.SEN = 1;       // START
                if ( IdleI2C2TO())  goto I2CStuck;

                SSP2BUF = RxBuffer[1];      // send the address/comand byte
                if ( IdleI2C2TO())  goto I2CStuck;

                if ( SSP2CON2bits.ACKSTAT != 0)
                    goto I2CNack;           // exit if it did not receive ACK
                // send data out
                i = 3;
                while (RxBuffer[2]--)
                {
                    SSP2BUF = RxBuffer[ i++];
                    if ( IdleI2C2TO())  goto I2CStuck;

                    if ( SSP2CON2bits.ACKSTAT != 0)
                        goto I2CNack;       // exit if did not receive ACK
                }
                SSP2CON2bits.PEN = 1;       // STOP terminate successfully
                if ( IdleI2C2TO())  goto I2CStuck;

                break;

I2CNack:
                SSP2CON2bits.PEN = 1;       // STOP
                if ( IdleI2C2TO())  goto I2CStuck;

                Buffer[0] = 0xFE;           // signal NACK
                break;
I2CStuck:
                Buffer[0] = 0xFF;           // fatal failure must re-inti I2C!!
                break;


// Analog commands -------------------------------------------------------
// NOTE command 0x37 is reserved for legacy with HID demo (see top)

            // AN configure (restore default)
            case 0x30:
                ANCON0 = 0xFB;              // All pins to digital (except AN2)
                ANCON1 = 0xBF;              // Default all pins to digital.  Bandgap on.
                break;

            // AN read analog (see legacy command 37)
            // AN read digital
            // AN write


// SPI commands -------------------------------------------------------

            // SPI1 configure
            case 0x40:
                TRISCbits.TRISC7 = 0;       // MOSI output
                TRISBbits.TRISB5 = 1;       // MISO input
                PORTBbits.RB4 = 0;          // SCK default low
                TRISBbits.TRISB4 = 0;       // SCK output
                SSP1STATbits.CKE = 1;       // CKE=1 clock on rising edge
                SSP1CON1 = 0b00100010;      // Enable SPI, CKP=0, Fosc/64
                break;

                // SPI write/read
            case 0x41:
                Buffer[1] = SSP1BUF;        // get the data
                SSP1BUF = RxBuffer[1];      // send data
                break;

// unrecognized -------------------------------------------------------------
            default:
                Buffer[0] = 0;              // command not recognized
                break;
        } //switch

        // send the return packet
        if ( ( !HIDTxHandleBusy(USBInHandle)) && (Buffer[0]!=0))
        {
            memcpy( TxBuffer, (const void*)Buffer, 64);
            // Prepare the USB module to send the data packet to the host
            USBInHandle = HIDTxPacket(HID_EP,(BYTE*)&TxBuffer[0],64);
        }

        //Re-arm the OUT endpoint, so we can receive the next OUT data packet 
        //that the host may try to send us.
        USBOutHandle = HIDRxPacket(HID_EP, (BYTE*)&RxBuffer, 64);
    }  // if command received
} // end ProcessIO


/******************************************************************************
 * Function:        WORD_VAL ReadPOT(void)
 *
 * Output:          WORD_VAL - the 10-bit right justified POT value
 *
 * Side Effects:    ADC buffer value updated
 *
 * Overview:        This function reads the POT and leaves the value in the 
 *                  ADC buffer register
 *
 * Note:            None
 *****************************************************************************/
WORD_VAL ReadPOT(void)
{
    WORD_VAL w;

    w.Val = 0;

    ADCON0bits.GO = 1;              // Start AD conversion
    while(ADCON0bits.GO);           // Wait for conversion
    w.v[0] = ADRESL;
    w.v[1] = ADRESH;

    return w;
} // end ReadPOT


/********************************************************************
 * Function:        void BlinkUSBStatus(void)
 *
 * Overview:        BlinkUSBStatus turns on and off LEDs 
 *                  corresponding to the USB device state.
 *
 * Note:            mLED macros can be found in HardwareProfile.h
 *                  USBDeviceState is declared and updated in
 *                  usb_device.c.
 *******************************************************************/
void BlinkUSBStatus(void)
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
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
        if( USBDeviceState == DETACHED_STATE)
        {
            mLED_Both_Off();
        }
        else if( USBDeviceState == ATTACHED_STATE)
        {
            mLED_Both_On();
        }
        else if( USBDeviceState == POWERED_STATE)
        {
            mLED_Only_1_On();
        }
        else if( USBDeviceState == DEFAULT_STATE)
        {
            mLED_Only_2_On();
        }
        else if( USBDeviceState == ADDRESS_STATE)
        {
            if(led_count == 0)
            {
                mLED_1_Toggle();
                mLED_2_Off();
            }//end if
        }
        else if( USBDeviceState == CONFIGURED_STATE)
        {
            if( led_count==0)
            {
                mLED_1_Toggle();
            }
            
        }
    }
} // end BlinkUSBStatus

/** EOF main.c *************************************************/
