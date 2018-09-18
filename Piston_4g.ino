
/*
  Proportional Piston Tank Controller by David Forrest based on work by Eric Weber and Gabriel Staples. 18/9/2018
  My code changes are labelled RDF
  Hall switch (A1104 from Allegro) used to count revolutions and give proportional control.
  An error signal is generated from the Hall switch count and the r/c input pulse.
  This is fed to the H bridge and motor (It is a servo really)
  When the unit is first powered up it has 35 seconds to empty the piston tank and set the switch counter to zero.
  Loss of r/c signal (Failsafe) empties the piston tank and resets the switch counter.
  The Hall effect sensor is switched on when the south pole of a magnet comes close to the front tapering face of the sensor.
  Hall switch needs +5v supply.
  Nov 2017 Put pullup in software this seems to eliminate the need for a pullup resistor.
  Nov 2017 Built In LED now shows forward & reverse
  Aug 2018 Pin information added for cheap and easily available (43  Amps !?)IBT 2 using Double BTS7960 H-bridge driver circuit,
  Sept 2018 Library information given for RC Guy (Gabriel Staples) (Now seems to need  a $5 donation.)
 
  Notes by Gabriel Staples
  Pulse_reader_w_pin_change_interrupt_singleCh.ino
  -read in any pulsing signal on Arduino pin INPUT_PIN (defined below), to get its period (us) & freq (Hz), with a time resolution of 0.5us
  --you can read in any pulse, including standard PWM signals, Radio Control (RC) PWM signals, etc.
  -I am using some low-level AVR code, which requires using some built-in Arduino macros to do pin-mapping.
  -this code only reads in a single channel at a time, though it could be expanded to read in signals on every Arduino pin, digital and analog, simultaneously.
  --this would be lots of work, so for now I'll leave that up to you.
  -this code should be able to read in any pulse between approximately 10~20us and 35.79 minutes; I'll let you experiment
  to find the actual shortest pulse you can measure with it

  By Gabriel Staples
  http://www.ElectricRCAircraftGuy.com/
  -My contact info is available by clicking the "Contact Me" tab at the top of my website.
  Written: 28 Nov. 2013
  Updated: 21 March 2015

  http://www.electricrcaircraftguy.com/2014/02/Timer2Counter-more-precise-Arduino-micros-function.html
  David Forrest note: Seems to need $5 donation now(18/9/2018)
  https://github.com/ElectricRCAircraftGuy/eRCaGuy_Timer2_Counter
  
  Some References:
  -to learn how to manipulate some of the low-level AVR code, pin change interrupts, etc, these links will help
  --http://www.gammon.com.au/interrupts
  --ATmega328 datasheet: http://www.atmel.com/Images/Atmel-8271-8-bit-AVR-Microcontroller-ATmega48A-48PA-88A-88PA-168A-168PA-328-328P_datasheet.pdf
  --http://playground.arduino.cc/Main/TimerPWMCheatsheet
  --See the *many* very helpful links at bottom of this article: http://www.electricrcaircraftguy.com/2014/01/the-power-of-arduino.html
  --reference the Arduino source code, ex:
  ---C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\Arduino.h
  ---C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\wiring_digital.c
  ---C:\Program Files (x86)\Arduino\hardware\arduino\avr\variants\standard\pins_arduino.h
*/

/*
  Circuits:

  Option 1) To measure the pulses from an Arduino PWM pin, ex: pin 5 or 9, connect Pin 5 or 9 (a PWM output) to INPUT_PIN (the pulse reader input)
          -see the setup() function for commanding the PWM output to begin, so you can have something to read in

  Option 2) To measure an RC PWM servo-type signal coming from an RC Rx:
  -Power the Rx by connecting 5V to + on the Rx, and GND to - on the Rx
  -Connect the channel signal you want to measure on the Rx to the INPUT_PIN on the Arduino
*/
#include <eRCaGuy_Timer2_Counter.h>

//#include <avr/wdt.h>    // Include watch dog timer

//macros
#define fastDigitalRead(p_inputRegister, bitMask) ((*p_inputRegister & bitMask) ? HIGH : LOW)

//Global Variables & defines
const byte INPUT_PIN = A5; //RDF changed to A5 .you can change this to ANY digital or analog pin, ex: 10, 8, A0, A5, etc,
//EXCEPT A6 and A7 (which exists on the Nano and Pro Mini, for example, and are NOT capable of digital operations)
byte input_pin_bitMask;
volatile byte* p_input_pin_register;

//volatile variables for use in the ISR (Interrupt Service Routine)
volatile boolean output_data = false; //the main loop will try to output data each time a new pulse comes in, which is when this gets set true
volatile unsigned long pulseCounts = 0; //units of 0.5us; the input signal high pulse time
volatile unsigned int pd = 0; //units of 0.5us; the pulse period (ie: time from start of high pulse to start of next high pulse)

// Simple counter from below (RDF)

int ip_pulse = 0;  // Variable from TX pulse by RDF

float errsig = 0; // Error signal for servo action (Hall count minus PWM input)

//https://quarkstream.wordpress.com/2009/12/11/arduino-4-counting-events/

int state = LOW;

int lastState = LOW;

int count = 0;

int reverse = 15;

int failsafe_count = 1;

//Digital input pin 8 for Hall switch input.

unsigned long duration;

float analog = errsig ;

void setup() {
  // put your setup code here, to run once:

  pinMode(8, INPUT_PULLUP);// Put Pullup on Hall switch input

  state = digitalRead(8);

  pinMode(INPUT_PIN, INPUT_PULLUP); //use INPUT_PULLUP to keep the pin from floating and jumping around when nothing is connected

  //configure timer2
  timer2.setup();

  //prepare for FAST digital reads on INPUT_PIN, by mapping to the input register (ex: PINB, PINC, or PIND), and creating a bitMask
  //using this method, I can do digital reads in approx. 0.148us/reading, rather than using digitalRead, which takes 4.623us/reading (31x speed increase)
  input_pin_bitMask = digitalPinToBitMask(INPUT_PIN);
  p_input_pin_register = portInputRegister(digitalPinToPort(INPUT_PIN));

  configurePinChangeInterrupts();

  //start PWM output (to read in its pulses, for use in Circuit 1 as described above)
  pinMode(5, OUTPUT);
  pinMode(9, OUTPUT);
  analogWrite(5, 10); //976.5625Hz, with high pulses of ~1/976.5625 x 10/256 = 40us; Connect pin 5 to INPUT_PIN and open serial monitor & you will see approximately this
  analogWrite(9, 128); //490.20Hz, with high pulses of ~1/490.2 x 128/256 = ~1020us; Connect pin 9 to INPUT_PIN and open serial monitor & you will see approximately this
  /*
    PWM Notes:
    -PWM on pins 5 & 6 occurs at 976.5625Hz; see here: http://playground.arduino.cc/Main/TimerPWMCheatsheet
    -PWM on pins 9 & 10 occurs at 490.20Hz
  */


  Serial.begin(9600);   //RDF changed from 115200
  Serial.print(F("Begin waiting for pulses on pin ")); Serial.print(INPUT_PIN);
  Serial.println(F(".\nData will be printed after each pulse is received."));

  // Test.connect motor controller pins to Arduino digital pins
  // motor one
  // https://tronixlabs.com.au/news/tutorial-l298n-dual-motor-controller-module-2a-and-arduino

  int enA = 3;
  int in1 = 2;
  int in2 = 4;
  
  // When using IBT-2 Double BTS7960 H-bridge driver circuit, use following pinouts:
  // Connect pins 3,4 on IBT-2 together and connect to D3 on Nano
  // Pin 1 to D4 on Nano
  // Pin 2 to D2 on Nano

  // set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  //Test. This will be used for piston tank initial setup

  digitalWrite(in1, HIGH); //Swap in1 & in2 to reverse motor
  digitalWrite(in2, LOW);
  digitalWrite(enA, HIGH); //High runs motor

  delay(35000); //35 second delay to get piston tank empty from the start.

  //set rev counter to zero

  count = 0;

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup code finishes here
}

void loop() {
  // put your main code here, to run repeatedly:

  //local variables
  static float pulseTime = 0; //us; the most recent input signal high pulse time
  static float pd_us = 0; //us; the most recent input signal period between pulses
  static float pulseFreq = 0; //Hz, the most recent input signal pulse frequency

  if (output_data == true) //if a pulse just came in
  {
    //turn off interrupts, grab copies of volatile data, and re-enable interrupts
    noInterrupts();
    output_data = false; //reset
    unsigned long pulseCountsCopy = pulseCounts; //0.5us units
    unsigned long pdCopy = pd; //0.5us units
    interrupts();

    //do calculations
    pulseTime = pulseCountsCopy / 2.0; //us
    pd_us = pdCopy / 2.0; //us
    pulseFreq = 1000000.0 / pd_us; //Hz

    //failsafe_count = ++failsafe_count; //Increment failsafe count by RDF
    //if ( pulseTime > 1000 ) {
    //  failsafe_count = 0;  // set counter to zero if pulses are > 1000
    // }

    //print values
    //(optionally, add extra code here to not print after EVERY pulse is received, as this can result in serial data coming in excessively fast when pulses come in at a high freq)
    //Serial.print(F("pulsetime(us) = ")); Serial.print(pulseTime);
    //Serial.print(F(", pd_us(us) = ")); Serial.print(pd_us);
    //Serial.print(F(", pulseFreq(Hz) = ")); Serial.println(pulseFreq);

    Serial.print(F("  Hall Count=")); Serial.println(count);
    Serial.print(F("IP Pulse=")); Serial.print(ip_pulse);
    //Serial.print(F("Error signal = ")); Serial.print(errsig);
    //Serial.print(F("reverse = ")); Serial.print(reverse);
    //Serial.print(F("Failsafe = ")); Serial.print(failsafe_count);

    // Hall switch counter, with increment/decrement depending on direction of rotation by RDF

    int enA = 3;
    int in1 = 2;
    int in2 = 4;

    if (state == HIGH && lastState == LOW) {

      //  if (reverse > 0 ) { ++count; } else { --count; }
      if (reverse > 0 ) {
        --count;
      } else {
        ++count;
      }
    }

    lastState = state;

    state = digitalRead(8);
  }

  // Calculations here

  ip_pulse = (((pulseTime - 1005) / 7) - 20); // For a screw length of about 250 revs. 25 is stick trim factor

  //AnalogSmooth section not used and commented out

  //int analog = ip_pulse; // put ip_pulse into AnalogSmooth

  // Defaults to window size 10
  //AnalogSmooth as = AnalogSmooth();

  //int analogSmooth = as.smooth(analog);

  //ip_pulse = analogSmooth;

  errsig = count - ip_pulse;

  // Testing motor output pins

  int enA = 3;
  int in1 = 2;
  int in2 = 4;

  // If errsig is negative, reverse the motor

  if (errsig < 0) {
    digitalWrite(in1, LOW);   //Swap in1 & in2 to reverse motor
    digitalWrite(LED_BUILTIN, HIGH);   // turn the Built In LED on (HIGH is the voltage level)
    digitalWrite(in2, HIGH);
    digitalWrite(enA, HIGH);    //High runs motor
    reverse = -7;
  }

  // If absolute value of errsig is less than 20, stop the motor

  if ((errsig < 10) && (errsig > -10))
  {
    digitalWrite(in1, HIGH);   //Swap in1 & in2 to reverse motor
    digitalWrite(in2, LOW);
    digitalWrite(enA, LOW);    //High runs motor
    reverse = 2;
  }

  //Set motor to forward
  if (errsig > 0) {
    digitalWrite(in1, HIGH);   //Swap in1 & in2 to reverse motor
    digitalWrite(LED_BUILTIN, LOW);   // turn the Built In LED off(HIGH is the voltage level)
    digitalWrite(in2, LOW);
    digitalWrite(enA, HIGH);    //High runs motor
    reverse = 8;
  }

  // If Failsafe triggered - set piston to empty
  if ((pulseFreq > 53) || (pulseFreq < 49) && (failsafe_count > 1000))
  {

    failsafe_count = ++failsafe_count; //Increment failsafe count by RDF

  //Failsafe routine (like piston tank initial setup.)

  digitalWrite(in1, HIGH); //Swap in1 & in2 to reverse motor
  digitalWrite(in2, LOW);
  digitalWrite(enA, HIGH); //High runs motor

  delay(35000); //35 second delay to get piston tank empty.

  //set rev counter to zero

  count = 0;

    digitalWrite(in1, HIGH);   //Swap in1 & in2 to reverse motor
    digitalWrite(in2, LOW);
    digitalWrite(enA, HIGH);    //High runs motor
    Serial.print(F("Failsafe = ")); Serial.print(pulseFreq);
  }

  //else {failsafe_count  = 0;
  //}

} //end of loop()

////Use macro instead
//boolean fastDigitalRead(volatile byte* p_inputRegister,byte bitMask)
//{
//  return (*p_inputRegister & bitMask) ? HIGH : LOW;
//}

void pinChangeIntISR()
{
  //local variables
  static boolean pin_state_new = LOW; //initialize
  static boolean pin_state_old = LOW; //initialize
  static unsigned long t_start = 0; //units of 0.5us
  static unsigned long t_start_old = 0; //units of 0.5us

  pin_state_new = fastDigitalRead(p_input_pin_register, input_pin_bitMask);
  if (pin_state_old != pin_state_new)
  {
    //if the pin state actualy changed, & it was not just noise lasting < ~2~4us
    pin_state_old = pin_state_new; //update the state
    if (pin_state_new == HIGH)

    {
      t_start = timer2.get_count(); //0.5us units
      pd = t_start - t_start_old; //0.5us units, the incoming pulse period
      t_start_old = t_start; //0.5us units; update
    }
    else //pin_state_new == LOW
    {
      unsigned long t_end = timer2.get_count(); //0.5us units
      pulseCounts = t_end - t_start; //0.5us units
      output_data = true;
    }
  }
}

//Interrupt Service Routines (ISRs) for Pin Change Interrupts
//see here: http://www.gammon.com.au/interrupts

//PCINT0_vect is for pins D8 to D13
ISR(PCINT0_vect)
{
  pinChangeIntISR();
}

//PCINT1_vect is for pins A0 to A5
ISR(PCINT1_vect)
{
  pinChangeIntISR();
}

//PCINT2_vect is for pins D0 to D7
ISR(PCINT2_vect)
{
  pinChangeIntISR();
}

void configurePinChangeInterrupts()
{
  //Pin Change Interrupt Configuration
  //see ATmega328 datasheet, ex: pgs. 73-75
  //also see: C:\Program Files (x86)\Arduino\hardware\arduino\avr\variants\standard\pins_arduino.h for the macros used here
  //1st, set flags on the proper Pin Change Mask Register (PCMSK)
  volatile byte* p_PCMSK = (volatile byte*)digitalPinToPCMSK(INPUT_PIN); //pointer to the proper PCMSK register
  *p_PCMSK = _BV(digitalPinToPCMSKbit(INPUT_PIN));
  //2nd, set flags in the Pin Change Interrupt Control Register (PCICR)
  volatile byte* p_PCICR = (volatile byte*)digitalPinToPCICR(INPUT_PIN); //pointer to PCICR
  *p_PCICR |= _BV(digitalPinToPCICRbit(INPUT_PIN));

  //  //ex: to use digital pin 8 as the INPUT_PIN:
  //  //turn on PCINT0_vect Pin Change Interrupts (for pins D8 to D13); see datasheet pg. 73-75.
  //  //1st, set flags on the proper Pin Change Mask Register
  //  PCMSK0 = 0b00000001; //here I am setting Bit0 to a 1, to mark pin D8's pin change register as on; for pin mapping see here: http://arduino.cc/en/Hacking/PinMapping168
  //  //2nd, set flags in the Pin Change Interrupt Control Register
  //  PCICR |= 0b00000001; //here I am turning on the pin change vector 0 interrupt, for PCINT0_vect, by setting the right-most bit to a 1
}

