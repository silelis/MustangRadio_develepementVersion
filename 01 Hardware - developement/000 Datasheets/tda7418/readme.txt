https://developer.mbed.org/forum/mbed/topic/311/?page=1#comment-1594



Iv

You need to programm all control bytes for the first time, like this:

   I2caddr = &H88                                            'address
   I2cdata(1) = &H20                                        'REG0 AUTO INC
   I2cdata(2) = &H01                                        '0-INPUT SELECTOR
   I2cdata(3) = &H80                                        '1-LOUDNESS OFF
   I2cdata(4) = &H80                                        '2-*VOLUME 0DB
   I2cdata(5) = &HCF                                        '3-TREBLE CENTER 15KHZ, 0DB
   I2cdata(6) = &HCF                                        '4-MIDDLE Q=1 0DB
   I2cdata(7) = &H8F                                        '5-BASS Q=1 0DB
   I2cdata(8) = &H0E                                        '6-BASS 200HZ MIDDLE 1,5KHZ
   I2cdata(9) = &HA0                                        '7-*  LEFT FRONT
   I2cdata(10) = &HA0                                       '8-*  LEFT REAR
   I2cdata(11) = &HA0                                       '9-*  RIGHT REAR
   I2cdata(12) = &HA0                                       '10-* RIGHT FRONT
   I2cdata(13) = &H80                                       '11-* SUBWOOFER
   I2cdata(14) = &H01                                       '12-SOFT MUTE SOFT STEP OFF
   I2cdata(15) = &H00                                       '13-IN OPERATION

   I2cstart
   I2csend I2caddr , I2cdata(1) , 15                        'PGM CHIP
   I2cstop

This is a BASCOM sample, but I used this sequence and is working fine

Harald