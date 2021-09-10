/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.
````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````
  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC18F47K42
        Driver Version    :  2.00
 */

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
 */

/*
  Name:       Piston_Winch_Pump_controller-2.3
  Date:       23 Aug 2021
  Brief:      
  Author:     David Forrest

 ***********************************************************************************************
  Revisions:
  Date        rev,    who      what
  21/07/2021  2.3     RDF      Changed count in mody2 option 
  23/08/2021  1.0     RDF      First version for PIC 18F
  9/09/2021   1.3     RDF      Code tidied up and simplified
 *  
  26 Aug 2021.  PIC 18f47k42 uses SMT timer for r/c pulse. External interrupt for Hall switch counter. Has USB inout to PC for ease of setting up. (baud rate 9600)
  12 May 2021. Using mode switch setting (mody)  to select mode of working.
  29 May 2021. Failsafe for 40Mhz receivers removed (left in Arduino version).
  mody configures unit for different purposes:
  mody = 0: Piston tank proportional control using limit switches. About 160 revs to fill.
  mody = 1:  Pump - on/off/reverse with no proportional control.
  mody = 2:  Winch - proportional starting in mid-stick position. No limit switches.
  Make sure that you set mody to mode you want.
  Proportional Piston Tank Controller or winch controller by David Forrest based on work by Eric Weber and Gabriel Staples. 18/9/2018
 The software can be used for 3 easily available (e.g eBay) H bridge controllers:
  (a)IBT-2 Double BTS7960 H-bridge driver circuit using Motor Driver Auto BTS7960 43A H-Bridge PWM Drive. 43 Amps
  (b)L298N DC Stepper Motor Driver Module Dual H Bridge                                                                    Control Board for Double H bridge drive. Drive current 2A(MAX single bridge)
  (c)L293D Module For Arduino L293D motor shield. 600mA OUTPUT CURRENT CAPABILITY PER CHANNEL. 1.2A PEAK OUTPUT CURRENT (non repetitive)PER CHANNEL. Often called Deek-Robot
  (Check these L293D modules carefully. I had a batch of these where the L293D chip had been reversed causing overheating. Had me scratching my head for a day or two!)
Hall switch (A1104 from Allegro) used to count revolutions and give proportional control.
  An error signal is generated from the Hall switch count and the r/c input pulse.
  This is fed to the H bridge and motor (It is a servo really)
  When the unit is first powered up it has 35 seconds to empty the piston tank and set the switch counter to zero.
  Loss of r/c signal (Failsafe) empties the piston tank and resets the switch counter.
  The Hall effect sensor is switched on when the south pole of a magnet comes close to the front tapering face of the sensor.
  Hall switch needs +5v supply.
 * 470 ml Piston Tank - Full is 161 revs
 
Pins for PIC 18f47k42 version connections are:
A0 is in1
A1 is in2
A2 is enA
E2 for LED showing reverse
C1 is r/c pulse input
B0 is Hall switch input
B2 & B3 are are USB connection to PC
 */

#include "mcc_generated_files/mcc.h"

// Global variables (RDF)
int mody = 0; // Set mode. Piston tank =0, pump control = 1, winch =2
int band = 0; // Sets the dead-band for proportional control
signed int ip_pulse = 0; // Variable from TX pulse 
signed int errsig = 0; // Error signal for servo action (Hall count minus PWM input)
int count = 0; // Counts from Hall switch
int reverse = 1;
int failsafe_count = 1;
float analog;
uint32_t Width = 3;
uint32_t Period = 0;
uint32_t Timer = 0;


//Callback routine for Hall counter

void counterHall_callback(void) {
    // On interrupt from Hall switch add or subtract 1 from Hall Counter
    if (reverse > 0) {
        ++count;
    } else {
        --count;
    }
}

/*
 // (a)IBT-2 Double BTS7960 H-bridge
 // Summary of pin connections when using IBT-2 Double BTS7960 H-bridge by RDF
 // Connect pins 3,4 on IBT-2 together and connect to D3 on Nano. (These pins turn the motor on)
 // Pin 1 to D4 on Nano. (These pins set the direction of rotation of the motor.)
 // Pin 2 to D2 on Nano. ( This pin sets the direction of rotation of the motor.)
 // R_EN & L_EN linked together and connected to D3
 // VCC pin 7 to +5v on Arduino
 // Hall switch input to D8
 // RX input to A5
 // M+ and M- on screw terminals to motor
 // B+ and  B- to +12v and GND

 //(b)L298N DC Stepper Motor Driver Module Dual H Bridge. 600mA.
 // Test.connect motor controller pins to Arduino digital pins
 // motor one
 // https://tronixlabs.com.au/news/tutorial-l298n-dual-motor-controller-module-2a-and-arduino
 // L298M module for Arduino. Often called Deek-Robot
 // Logic voltage is 5v so can connect directly to an Arduino
 // Link pins on EN1
 // GND to GND on Arduino
 // VCC to +5v on Arduino
 // Hall switch input to D8
 // RX input to A5
 // A+ and A- on screw terminals to motor
 // en1 to D3; // Front pin
 // in1 to D2;
 // in2 to D4;

 //(c)L293D Module For Arduino L293D motor shield. 600mA. 1.2A PEAK OUTPUT CURRENT
 // Link pins on EN2
 // EN1 to D3; // Front pin
 // IN1 to D2;
 // IN2 to D4;
 // GND to GND on Arduino
 // VCC to +5v on Arduino
 // A+ and A- on screw terminals to motor
 // VIN and GND to +12v and GND respectively
 // Logic voltage is 3v so needs interface (eg logic level convertor) to connect to an Arduino and prevent overheating.

                         Main application
 */
void main(void) {
    // Initialise the device
    SYSTEM_Initialize();
    INT0_SetInterruptHandler(counterHall_callback);
    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Put your setup code here, to run once:

    __delay_ms(500); // short delay for printing
    // Initialise the piston tank. Set all the motor control pins to outputs
    LATAbits.LATA0 = 0; // LATA0  is in1
    LATAbits.LATA1 = 1; // LATA1 is in2
    LATAbits.LATA2 = 1; // LATA2 is enA
    LATEbits.LATE2 = 0; // Installed LED, shows reverse

    printf(" \r\n"); // New line
    printf("Start up delay. Begin waiting for r/c pulses on input pin. \r\n");
    printf("Data will be printed after pulses are received. \r\n");

    //Set start up delay
    switch (mody) {
        case 0: for (int j = 0; j <= 5; j++) { // Blink LED and then 35 second delay to empty piston tank and then blink installed LED again.
                __delay_ms(100);       
                LATEbits.LATE2 = 1;
                __delay_ms(100);
                LATEbits.LATE2 = 0;
            }
            __delay_ms(35000);

            for (int j = 0; j <= 5; j++) {
                __delay_ms(100);
                LATEbits.LATE2 = 1;
                __delay_ms(100);
                LATEbits.LATE2 = 0;
            }
            break;
        case 1: __delay_ms(500);
            break;
        case 2: __delay_ms(500);
            break;
    }

    //Set Hall switch counter to initial value

    switch (mody) {
        case 0: count = 0;
            break;
        case 1: count = 50;
            break;
        case 2: count = 5;
            break;
    }

    // Initialisation code finishes here  and Main loop starts.

    while (1) {

        // Bring in r/c input from SMT module
        Width = SMT1CPW;
        Period = SMT1CPR;
        Timer = SMT1TMR;

        // Calculations here

        //Set value for input_pulse. For 0 to 300 ip_pulse and screw length of about 160 revs.(This is for 470 ml piston tank.)
        switch (mody) {
                // scale() // Scale function doesn't seem to be available in XC8 compiler    so divide range of 255 by actual range and subtract minimum actual value.
            case 0: ip_pulse = ((Width - 150)*2); // For LoRa RX. 169 is minimum stick input. 226 maximum.      
                //case 0: ip_pulse = (((Width - 0) / 0.5) - 240); // For 40Mhz RX
                break;
            case 1: ip_pulse = ((Width - 169)*4.5);
                break;
                //case 2:   ip_pulse = (((pulseTime - 1005) / 90) - 0); break;
            case 2: ip_pulse = ((Width - 169)*4.5);
                break;
        }

        // Set value for Hall switch count
        switch (mody) {
            case 0: count = count;
                break;
            case 1: count = 50;
                break;
            case 2: count = count;
                break;
        }

        // Set dead-band
        switch (mody) {
            case 0: band = 5;
                break;
            case 1: band = 30;
                break;
            case 2: band = 2;
                break;
        }
        // Calculate error signal
        errsig = count - ip_pulse;

        // If errsig is negative, reverse the motor
        if (errsig < 0) {
            LATAbits.LATA0 = 1; // LATA0  is in1. Swap in1 & in2 to reverse motor
            LATAbits.LATA1 = 0; // LATA1 is in2
            LATAbits.LATA2 = 1; // LATA2 is enA. 1 runs motor
            reverse = +1;

        }
        // If absolute value of errsig is less than deadband, stop the motor
        if ((errsig < band) && (errsig > -band)) // Set dead-band
        {
            LATAbits.LATA0 = 1; // LATA0  is in1. Swap in1 & in2 to reverse motor
            LATAbits.LATA1 = 0; // LATA1 is in2
            LATAbits.LATA2 = 0; // LATA2 is enA. High runs motor
            reverse = 0;
        }

        //Set motor to forward
        if (errsig > 0) {
            LATAbits.LATA0 = 0; // LATA0  is in1. Swap in1 & in2 to reverse motor
            LATAbits.LATA1 = 1; // LATA1 is in2
            LATAbits.LATA2 = 1; // LATA2 is enA. 1 runs motor
            reverse = -1;
        }

        LATEbits.LATE2 = reverse; // Installed LED, shows reverse    

        printf("Input pulse =%d   Hall counter =%d   Error signal=%d   Reverse =%d \r\n", ip_pulse, count, errsig, reverse);
        __delay_ms(200);

    }
}
/**
 End of File
 */