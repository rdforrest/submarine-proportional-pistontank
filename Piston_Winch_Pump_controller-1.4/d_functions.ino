//// Use macro instead
// boolean fastDigitalRead(volatile byte* p_inputRegister,byte bitMask)
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
