# Software for the robot

If you build your own robot, you need to set the fuses correctly. I did this with Atmel Studio.
The Production file for the fuses is "Fuses_Lightbug_v1.1.elf". 
If you can't use Atmel Studio, the fuses need to be the following values:
* Extended: 0xFE
* High: 0xD8
* Low: 0xDE
 
After setting the Fuses and installing the Arduino Leonardo Bootloader (Leonardo-prod-firmware-2012-12-10.hex)
you can program the board like any regular Arduino Leonardo.

There are 2 versions:
* one basic which just has the light following ability (software_lightbug_v1_0)
* one with added serial communication with simple commands to control the robot over bluetooth (software_lightbug_v1_1)
