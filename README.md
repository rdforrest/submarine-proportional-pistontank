# submarine-proportional-pistontank
Proportional control of a piston tank for model submarines to improve control and trim over on/off control. 
Proportional Piston Tank Controller by David Forrest based on work by Eric Weber and Gabriel Staples.
Hall switch (A1104 from Allegro) is used to count revolutions and give proportional control.
The Hall effect sensor is switched on when the south pole of a magnet comes close to the front tapering face of the sensor. There is no problem with "bounce" etc.
An error signal is generated from the difference between the Hall switch count and the r/c input pulse.
This is processed and fed to the H bridge and motor (It is a servo really)
When the unit is switched on it has 35 seconds to empty the piston tank and set the switch counter to zero.
Loss of r/c signal (Failsafe) empties the piston tank and resets the switch counter.
The Hall switch needs +5v supply.
Nov 2017 Put pullup in software this seems to eliminate the need for a pullup resistor.
Nov 2017 Built In LED now shows forward & reverse
AnalogSmooth routine not actually used.
Note that this routine has only been tested on the "bench" so far, not in the "wet."
