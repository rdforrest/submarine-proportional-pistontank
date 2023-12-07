
/*
  Name:       Piston_Winch_Pump_controller-2.7
  Date:       21 July 2021
  Brief:
  Author:     David Forrest

***********************************************************************************************
  Revisions:
  Date          rev,    who      what
  1/9/2022      2.3     RDF      Changed count in mody2 option to 1 from 5
  21/07/2021    2.3     RDF      Changed count in mody2 option
  12 May 2021.                   Using mode switch setting (mody)  to select mode of working.
  29 May 2021.                   Failsafe for 40Mhz receivers commnented out.
  21 Aug 2023.  2.6     RDF      Changing winch parameters.
  06 Dec 2023.  2.6     RDF      Seeing if logic level convertor for L293D Module may be unnecessary. (Makes device smaller.)
  Mody configuration value
  mody configures unit for different purposes:
  mody = 0: Piston tank proportional control using limit switches. Empties piston for 35 seconds on power-up. Limit switched needed.
  mody =1:  Pump - on/off/reverse with no proportional control.
  mody =2:  Winch - proportional. On power-on, starting in low-stick position, winds in for 20 seconds. No limit switches but needs slipping clutch.
  Make sure that you set mody to mode you want.
  
  Also trying to use PulseIn for r/c pulse input - but seems to be too slow - so any improvement needs to use interrupts?
  Proportional Piston Tank Controller or winch controller by David Forrest based on work by Eric Weber and Gabriel Staples. 18/9/2018
  Use a genuine Arduino Nano for reliability. (I have had problems with clones,)
  The software can be used for 3 easily available (eg eBay) H bridge controllers:
  (a)IBT-2 Double BTS7960 H-bridge driver circuit using Motor Driver Auto BTS7960 43A H-Bridge PWM Drive. 43 Amps
  (b)L298N DC Stepper Motor Driver Module Dual H Bridge Control Board for Double H bridge drive. Drive current 2A(MAX single bridge)
  (c)L293D Module For Arduino L293D motor shield. 600mA OUTPUT CURRENT CAPABILITY PER CHANNEL. 1.2A PEAK OUTPUT CURRENT (non repetitive)PER CHANNEL. Often called Deek-Robot
  (Check these L293D modules carefully. I had a batch of these where the L293D chip had been reversed causing overheating. Had me scratching my head for a day or two!)
  These modules  are described in an article in Everyday Practical Electronics Sept 2020 (Copies in my H Brdge folder.)
  Look at listing for appropriate pin connections.
  My code changes are labelled RDF
  
  Hall switch (A1104 from Allegro) used to count revolutions and give proportional control.
  An error signal is generated from the Hall switch count and the r/c input pulse.
  This is fed to the H bridge and motor (It is a servo really)
  When the unit is first powered up it has 35 seconds to empty the piston tank and set the switch counter to zero.
  Loss of r/c signal (Failsafe) empties the piston tank and resets the switch counter.
  The Hall effect sensor (A1101, 3 pin DIP package) is switched on when the south pole of a magnet comes close to the front tapering face of the sensor.
  Hall switch needs +5v supply. Connections (with tapering face upwards) left hand pin +5v, Middle pin GND, Right hand pin OUTPUT.
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
