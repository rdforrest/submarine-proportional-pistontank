
#include <eRCaGuy_Timer2_Counter.h>

//#include <avr/wdt.h>    // Include watch dog timer

//macros
#define fastDigitalRead(p_inputRegister, bitMask) ((*p_inputRegister & bitMask) ? HIGH : LOW)

//Global Variables & defines
const byte INPUT_PIN = A5; //RDF changed to A5.you can change this to ANY digital or analog pin, ex: 10, 8, A0, A5, etc,
//EXCEPT A6 and A7 (which exists on the Nano and Pro Mini, for example, and are NOT capable of digital operations)
byte input_pin_bitMask;
volatile byte* p_input_pin_register;

//volatile variables for use in the ISR (Interrupt Service Routine)
volatile boolean output_data = false; //the main loop will try to output data each time a new pulse comes in, which is when this gets set true
volatile unsigned long pulseCounts = 0; //units of 0.5us; the input signal high pulse time
volatile unsigned int pd = 0; //units of 0.5us; the pulse period (ie: time from start of high pulse to start of next high pulse)

// Global variables (RDF)

int mody = 0; // Set mode. Piston tank =0, pump control = 1, winch =2

int band = 0; // Sets the dead-band for proportional control

int ip_pulse = 0;  // Variable from TX pulse by RDF

float errsig = 0; // Error signal for servo action (Hall count minus PWM input)

int enA = 3; // Name digital pins
int in1 = 2; // Name digital pins
int in2 = 4; // Name digital pins

int H_test = 0; // H Bridge test. If set to 1, Run motor for 20 seconds, then reverse, then stop.

//Test for Pulsein - not used
int pin = 19;
unsigned long duration1;


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

  //Print operation mode (mody)
  switch (mody) {
    case 0: Serial.println(F(".\nPiston mode(mody=0)")); break;
    case 1: Serial.println(F(".\nPump mode(mody=1)")); break;
    case 2: Serial.println(F(".\nWinch mode(mody=2)")); break;
  }

  Serial.print(F("Start up delay. Begin waiting for pulses on pin ")); Serial.print(INPUT_PIN);
  Serial.println(F(".\nData will be printed after each pulse is received."));

  pinMode(pin, INPUT);

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
  // Logic voltage is 5v so can connect directly to an Arduino. Dec 2023 comment - maybe not necessary to use convertor.
  // Link pins on EN1
  // GND to GND on Arduino
  // VCC to +5v on Arduino. Dec 2023 comment - maybe not necessary so don't connect.
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
  // VCC to +5v on Arduino. Dec 2023 comment - maybe not necessary so don't connect.
  // A+ and A- on screw terminals to motor
  // VIN and GND to +12v and GND respectively
  // Logic voltage is 3v (Dec 2023 comment- opinions seem to vary about this. 5v may be OK) so needs interface (eg logic level convertor) to connect to an Arduino and prevent overheating.  Dec 2023 comment - maybe not necessary to use convertor. 

  // set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  digitalWrite(in1, HIGH); //Swap in1 & in2 to reverse motor
  digitalWrite(in2, LOW);
  digitalWrite(enA, HIGH); //High runs motor

  //Set start up delay
  switch (mody) {
    case 0: delay(35000); break;
    case 1: delay(500); break;
    case 2: delay(20000); break;
  }

  //Set Hall switch counter to initial value

  switch (mody) {
    case 0: count = 0; break;
    case 1: count = 50; break;
    case 2: count = 1; break;
  }

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup code finishes here
}
