# submarine-proportional-pistontank
Proportional control of a piston tank for model submarines.  
Proportional Piston Tank Controller by David Forrest based on work by Eric Weber and Gabriel Staples.
  My code changes are labelled RDF. 
  Hall switch (A1104 from Allegro) used to count revolutions and give proportional control.
  An error signal is generated from the Hall switch count and the r/c input pulse.
  This is fed to the H bridge and motor (It is a servo really)
  When the unit is switched on it has 35 seconds to empty the piston tank and set the switch counter to zero.
  Loss of r/c signal (Failsafe) empties the piston tank and resets the switch counter.
  The Hall effect sensor is switched on when the south pole of a magnet comes close to the front tapering face of the sensor.
  The Hall switch needs +5v supply.
  Nov 2017 Put pullup in software this seems to eliminate the need for a pullup resistor.
  Nov 2017 Built In LED now shows forward & reverse
  AnalogSmooth routine not actually used.
  Note that this routine has only been tested on the "bench" so far, not in the "wet."
