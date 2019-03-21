# submarine-proportional-pistontank
Proportional control of a piston tank for model submarines 
Proportional Piston Tank Controller by David Forrest based on work by Eric Weber and Gabriel Staples. 24/10/2017
  My code changes are labelled RDF. This version New_Piston4e (5/12/2017)
  Hall switch (A1104 from Allegro) used to count revolutions and give proportional control.
  An error signal is generated from the Hall switch count and the r/c input pulse.
  This is fed to the H bridge and motor (It is a servo really)
  When the unit is switched on it has 35 seconds to empty the piston tank and set the switch counter to zero.
  Loss of r/c signal (Failsafe) empties the piston tank and resets the switch counter.
  The Hall effect sensor is switched on when the south pole of a magnet comes close to the front tapering face of the sensor.
  Hall switch needs +5v supply.
  Nov 2017 Put pullup in software this seems to eliminate the need for a pullup resistor.
  Nov 2017 Built In LED now shows forward & reverse
  AnalogSmooth routine not actually used.
  Aug 2018. Pinout details added for cheap, easily available (43A??) H bridge - IBT-2
  Sept 2018 . RC Guy library details added and AnalogSmooth references removed.
  March 2019. Software still needs work before use in a working model sub - failsafe issues.
