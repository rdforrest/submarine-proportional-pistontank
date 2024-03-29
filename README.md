# submarine-proportional-pistontank
Proportional control of a piston tank for model submarines 
Proportional Piston Tank Controller by David Forrest based on work by Eric Weber and Gabriel Staples. 24/10/2017
  My code changes are labelled RDF.
  Hall switch (A1104 from Allegro) used to count revolutions and give proportional control.
  An error signal is generated from the Hall switch count and the r/c input pulse.
  This is fed to the H bridge and motor (It is a servo really)
  When the unit is switched on it has 35 seconds to empty the piston tank and set the switch counter to zero.
  Loss of r/c signal (Failsafe) empties the piston tank and resets the switch counter.
  The Hall effect sensor is switched on when the south pole of a magnet comes close to the front tapering face of the sensor.
  Hall switch needs +5v supply.
  March 2020 now tested in model sub and working well.
  Latest version can also be used for simple sail-winch and pump control.
  
  August 2021. 18f47k42 - piston.X This latest version for a PIC 18f47k42 is untested in a model sub but looks promising and was developed in 3 days (using the MCC feature) as      opposed to 3 months for the original version using Assembler! (I am experimenting with the 18f because of a useful series of articles by Mike Hibbett in Practical Electronics.) I was looking at a PIC version because I was concerned about the reliability of Nano clones but the latest versions seem OK.
  
  Sept 2021. Piston_Winch_Pump_controller-2.4 The latest version of the Arduino version works well in my model sub testing.
