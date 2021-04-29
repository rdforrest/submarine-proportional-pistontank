
/*
  12 Aug 2020. Using mode switch setting (mody)  to select mode of working.
  mody = 0: Piston tank proportional control using limit switches. With Failsafe
  mody =1:  Pump - on/off/reverse with no proportional control. With Failsafe
  mody =2:  Winch- on/off/reverse with no proportional control No limit switches. No Failsafe.
  Make sure that you set mody to mode you want.
  Also trying to use PulseIn for r/c pulse input - but seems to be too slow - so any improvement needs to use interrupts?
  Proportional Piston Tank Controller or winch controller by David Forrest based on work by Eric Weber and Gabriel Staples. 18/9/2018
  Use a genuine Arduino Nano for reliability. (I have had problems with clones,)
  The software can be used for 3 easily available (eg eBay) H bridge controllers:
  (a)IBT-2 Double BTS7960 H-bridge driver circuit using Motor Driver Auto BTS7960 43A H-Bridge PWM Drive. 43 Amps
  (b)L298N DC Stepper Motor Driver Module Dual H Bridge Control Board for Double H bridge drive. Drive current 2A(MAX single bridge)
  (c)L293D Module For Arduino L293D motor shield. 600mA OUTPUT CURRENT CAPABILITY PER CHANNEL. 1.2A PEAK OUTPUT CURRENT (non repetitive)PER CHANNEL. Often called Deek-Robot
  (Check these L293D modules carefully. I had a batch of these where the L293D chip had been reversed causing overheating. Had me scratching my head for a day or two!)
  Look at listing for appropriate pin connections.
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
  Hardware notes:

  Option 1) To measure the pulses from an Arduino PWM pin, ex: pin 5 or 9, connect Pin 5 or 9 (a PWM output) to INPUT_PIN (the pulse reader input)
          -see the setup() function for commanding the PWM output to begin, so you can have something to read in

  Option 2) To measure an RC PWM servo-type signal coming from an RC Rx:
  -Power the Rx by connecting 5V to + on the Rx, and GND to - on the Rx
  -Connect the channel signal you want to measure on the Rx to the INPUT_PIN on the Arduino
*/

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

*/
