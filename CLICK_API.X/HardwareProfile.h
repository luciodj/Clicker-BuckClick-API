/********************************************************************
 FileName:      HardwareProfile.h
 Processor:     PIC1847J53
 Hardware:      Mikroe PIC18 Clicker Board
 Compiler:      Microchip C18 (for PIC18)
 ********************************************************************/

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H


    /*******************************************************************/
    /******** USB stack hardware selection options *********************/
    /*******************************************************************/
    //This section is the set of definitions required by the MCHPFSUSB
    //  framework.  These definitions tell the firmware what mode it is
    //  running in, and where it can find the results to some information
    //  that the stack needs.
    //These definitions are required by every application developed with
    //  this revision of the MCHPFSUSB framework.  Please review each
    //  option carefully and determine which options are desired/required
    //  for your application.

    //#define USE_SELF_POWER_SENSE_IO
//    #define tris_self_power     TRISCbits.TRISC2    // Input
//    #define self_power          1

    //#define USE_USB_BUS_SENSE_IO
//    #define tris_usb_bus_sense  TRISCbits.TRISC2    // Input
//    #define USB_BUS_SENSE       1

    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /******** Application specific definitions *************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/

//    #define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER

    /** Board definition ***********************************************/
    //These defintions will tell the main() function which board is
    //  currently selected.  This will allow the application to add
    //  the correct configuration bits as wells use the correct
    //  initialization functions for the board.  These defitions are only
    //  required in the stack provided demos.  They are not required in
    //  final application design.
    #define DEMO_BOARD PIC18F47J53_CLICKER
    #define PIC18F47J53_PIM
    #define CLOCK_FREQ 8000000
    #define GetSystemClock() CLOCK_FREQ

    /** LED ************************************************************/
    #define mInitAllLEDs()      LATA &= 0xFC; TRISA &= 0xFC;

    #define mLED_1              LATAbits.LATA0
    #define mLED_2              LATAbits.LATA1
    #define mLED_3
    #define mLED_4

    #define mGetLED_1()         mLED_1
    #define mGetLED_2()         mLED_2
    #define mGetLED_3()         1
    #define mGetLED_4()         1

    #define mLED_1_On()         mLED_1 = 1;
    #define mLED_2_On()         mLED_2 = 1;
    #define mLED_3_On()
    #define mLED_4_On()

    #define mLED_1_Off()        mLED_1 = 0;
    #define mLED_2_Off()        mLED_2 = 0;
    #define mLED_3_Off()
    #define mLED_4_Off()

    #define mLED_1_Toggle()     mLED_1 = !mLED_1;
    #define mLED_2_Toggle()     mLED_2 = !mLED_2;
    #define mLED_3_Toggle()
    #define mLED_4_Toggle()

    /** SWITCH *********************************************************/
    #define mInitSwitch2()      TRISDbits.TRISD2=1;
    #define mInitSwitch3()      TRISDbits.TRISD3=1;
    #define mInitAllSwitches()  mInitSwitch2();mInitSwitch3();
    #define sw2                 PORTDbits.RD2
    #define sw3                 PORTDbits.RD3

    /** AN input ******************************************************/
    // AN2 is connected ot the An pin of the mikrobus connector
    #define mInitADC()         { TRISAbits.TRISA2=1;    \
                                 ANCON0bits.PCFG2 = 0;  \
                                 ADCON0bits.CHS=2;      \
                                 ADCON0bits.ADON=1;     \
                                 ADCON1=0x9E;}     // right just, 6Tad, Fosc/64

    /** I/O pin definitions ********************************************/
    #define INPUT_PIN 1
    #define OUTPUT_PIN 0

    #define T_INT       TRISDbits.TRISD4
    #define P_INT       PORTDbits.RD4

    #define T_PWM       TRISCbits.TRISC6
    #define P_PWM       PORTCbits.RC6

    #define T_RST       TRISBbits.TRISB2
    #define P_RST       PORTBbits.RB2

    #define T_SCL       TRISDbits.TRISD0
    #define P_SCL       PORTDbits.RD0
    #define T_SDA       TRISDbits.TRISD1

    #define T_TX        TRISDbits.TRISD6

    #define T_RX        TRISDbits.TRISD5

    #define T_CS        TRISBbits.TRISB3
    #define P_CS        PORTBbits.RB3

    #define T_SCK       TRISBbits.TRISB4
    #define T_MOSI      TRISCbits.TRISC7
    #define T_MISO      TRISBbits.TRISB5



#endif  // HARDWARE_PROFILE_H
