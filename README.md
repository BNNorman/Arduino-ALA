# Arduino-ALA
Extensions to ALA and FastGPIO libraries.

This folder contains all the files relating to my extended ALA library development.

##ExtAlaLedRgb

The extended library is called ExtAlaLedRgb.cpp and it's corresponding header.

I have added the ability to drive APA102 LED strips.

Using this extended version enables you to define sections of a single LED strip and run different sequences of animation on each section. I have posted a video on YouTube showing an APA102 led strip running two different animation sequences on different sections of the same strip. (https://www.youtube.com/watch?v=hsvYEGXiAFw)

##ExtFastGPIO

Using a 144 APA102 Led strip the RAM required is too much for a 2K device so I used a MEGA2560. In order to drive the APA102 as fast as possible I modified FastGPIO (renamed to ExtFastGPIO to add the pin definitions for the 2560.)

On the 2560 ExtFastGPIO Works ok with the exception of pins 42-49 (PORT L) and comms pins 14-15 (PORT J) and pins 16-17 (PORT H). If you try to use those pins the Arduino compiler throws an error "impossible constraint in 'asm'". There are plenty of other pins on the 2560 which can be used.

##Colours.h

I have included a header which defines the standard web colours as listed at http://www.w3schools.com/
