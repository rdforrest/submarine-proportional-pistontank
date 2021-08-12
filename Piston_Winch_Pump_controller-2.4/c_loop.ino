
void loop() {

  //Test of pulseIn for r/c input
  //duration1 = pulseIn(pin, HIGH);
  //Serial.println(duration1);

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

    Serial.print(F(" Hall Count= ")); Serial.println(count);
    Serial.print(F(" Input Pulse= ")); Serial.print(ip_pulse);
    Serial.print(F(" Error signal= ")); Serial.print(errsig);
    Serial.print(F(" Reverse/Off/Fwd,-1,0,1= ")); Serial.print(reverse);
    Serial.print(F(" Failsafe= ")); Serial.print(failsafe_count);

    // Hall switch counter, with increment/decrement depending on direction of rotation by RDF

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

  //Set value for ip_pulse. ( For a screw length of about 250 revs. 15 is stick trim factor)
  switch (mody) {
    case 0:   ip_pulse = (((pulseTime - 1005) / 11) - 5); break;
    case 1:   ip_pulse = (((pulseTime - 1005) / 7) - 15); break;
    //case 2:   ip_pulse = (((pulseTime - 1005) / 90) - 0); break;
    case 2:   ip_pulse = (((pulseTime - 1005) / 160) - 0); break;
  }

  //AnalogSmooth section not used and commented out

  //int analog = ip_pulse; // put ip_pulse into AnalogSmooth

  // Defaults to window size 10
  //AnalogSmooth as = AnalogSmooth();

  //int analogSmooth = as.smooth(analog);

  //ip_pulse = analogSmooth;

  // Set value for Hall switch count

  switch (mody) {
    case 0: count = count; break;
    case 1: count = 50; break;
    case 2: count = count; break;
  }

  // Set dead-band
  switch (mody) {
    case 0: band = 5; break;
    case 1: band = 20; break;
    case 2: band = 2; break;
  }

  errsig = count - ip_pulse;

  // If errsig is negative, reverse the motor

  if (errsig < 0) {
    digitalWrite(in1, LOW);   //Swap in1 & in2 to reverse motor
    digitalWrite(LED_BUILTIN, HIGH);   // turn the Built In LED on (HIGH is the voltage level)
    digitalWrite(in2, HIGH);
    digitalWrite(enA, HIGH);    //High runs motor
    reverse = -1;
  }

  // If absolute value of errsig is less than 30, stop the motor

  if ((errsig < band) && (errsig > - band)) // Set dead-band
  {
    digitalWrite(in1, HIGH);   //Swap in1 & in2 to reverse motor
    digitalWrite(in2, LOW);
    digitalWrite(enA, LOW);    //High runs motor
    reverse = 0;
  }

  //Set motor to forward
  if (errsig > 0) {
    digitalWrite(in1, HIGH);   //Swap in1 & in2 to reverse motor
    digitalWrite(LED_BUILTIN, LOW);   // turn the Built In LED off(HIGH is the voltage level)
    digitalWrite(in2, LOW);
    digitalWrite(enA, HIGH);    //High runs motor
    reverse = 1;
  }

  /*
    // Failsafe routine - set piston to empty, pump out
    // Only used for original 40Mhz receivers
    // if ((pulseFreq > 53) || (pulseFreq < 49) && (failsafe_count > 1000))
    // For LoRa RXs
    if (ip_pulse > 115) // Test for failsafe value
    {
      failsafe_count = ++failsafe_count; //Increment failsafe count by RDF
    }
    if ( failsafe_count > 1000) // Test for failsafe value
    {
      Serial.print("\n"); //newline
      Serial.print(F("Failsafe activated "));
      Serial.print("\n"); //newline
      Serial.print(F("ip_pulse = ")); Serial.print(ip_pulse); Serial.print(F(" failsafe_count = ")); Serial.print(failsafe_count);

      digitalWrite(in1, HIGH); //Swap in1 & in2 to reverse motor
      digitalWrite(in2, LOW);
      // digitalWrite(enA, HIGH); //High runs motor

      // delay(35000); //35 second delay to get piston tank empty.

      //Set delay
      switch (mody) {
        case 0: delay(35000); break;
        case 1: delay(500); break;
        case 2: delay(500); break;
      }

      //set rev counter to zero
      count = 0;

      digitalWrite(in1, HIGH);   //Swap in1 & in2 to reverse motor
      digitalWrite(in2, LOW);

      // Set Motor on or off during Failsafe. High runs motor
      switch (mody) {
        case 0: digitalWrite(enA, HIGH) ; break;
        case 1: digitalWrite(enA, HIGH); break;
        case 2: digitalWrite(enA, LOW); break;
      }

    }
    // Reset failsafe counter
    else {
      failsafe_count  = 0;
    }
  */

} //end of loop()
