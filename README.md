# IoT_Beer_Garden_Lamp
IoT Controlled Beer Garden Lamp with support for NeoPixel LEDs or simple White LED strip.

# Warning
This design has not been compliance tested and will not be. It is published purely for inspiration to others. I take no responsibility for damages caused directly or indirectly. I accept no responsibility for any damage to any equipment that results from the use of this design and its components. IT IS ENTIRELY AT YOUR OWN RISK!

# Overview
I always wanted a nice German Beer Garden lamp in the garden at home and what better way to install such a lamp than to connect it to the cloud. As I'm a big fan of Franziskaner Wheat Beer, naturally this was my lamp to use. It is designed for a normal mains power bulb, but I wanted to use controllable LEDs so low voltage is more realistic.

### Prost
Here is what the complete lamp looks like:
![Prost](/Images/Prost.JPG)

### Construction
I really wanted an old fashioned cast-iron lamp pole, but I found this too hard to source where I live so I settled on a treated wooden pole anchored in the ground using a "H" wooden pole holder. Some G clamps were used to set the pole in a straight position while the concrete sets.
![Concrete](/Images/ConcretedPole.jpg)

To make this more decorative, I used a RAL 7016 anthracite grey coloured retaining stone as a sleeve for the concrete pole base.
![Sleeve](/Images/FinishedFoundation.JPG)

In order to hold the actual Beer Garden lamp I constructed a mounting bracket using two galvanised "L" brackets bolted together with a small section of the lamp holder. Two "L" brackets were used to ensure rigidity during harsh weather conditions. So far it has been stable. The cable (in conduit) runs on not visible the side of the pole and feeds under the bracket into the lamp.
![Bracket](/Images/LampBracket.jpg)

### Hardware
The hardware is centred around an ESP-01. In order to get the current to drive the LEDs (either WS2812 or White LED strip) I used an old laptop power supply. As the Beer Garden lamp is positioned next to me garden shed, the power supply is housed inside and only low voltage is supplied to the Beer Garden lamp. Both LED array and ESP hardware is housed in the actual lamp. I used a simple Buck Regulator to drop to 5 volts (WS2812) or 12 volts (White LED strip). A Low Dropout Regulator is used for the ESP-01 3.3 volt needs.

Connection to the LED whether it is WS2812 or White LED strip is documented in the code. In the case of the White LED strip, a N-CH FET is used with a NPN BJT in an open collector (with strong pull-up) arrangement to make sure the FET is operating as close to saturation as possible - the 3.3 volt output of the ESP is on the lower end of the Vgs requirements.

### Software
The software is fairly straight forward as it is based around the Arduino Framework. I used Atom with PlatformIO as the development environment. There are several helper functions which I reuse in most of my ESP projects (IoT).

### HTML Front End
This is not my strong point... :) It is basic and does what I need it to do. I'm open for suggestions and support to make this look much (much) nicer.
