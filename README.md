# MS5837-for-MSP430
Library to interface pressure sensor MS5837 to MSP430 efficiently, using I2C peripheral and interrupts.

This code was written to interface any MSP430's I2C communication port to a pressure sensor MS5837. The library is written in order to optimize the use of interrupts, allowing your program to either execute other tasks or sleep while waiting for the pressure sensor's response.

The code is run, as an example, on the Green Bubbles DocuScooter Motherboard. But it can be ported to any MSP430 equipped with an I2C peripheral easily. Read more about Green Bubbles DocuScooter Motherboard and download its design documents on [www.corentinaltepe.com/blog/docuscooter-motherboard](http://www.corentinaltepe.com/blog/docuscooter-motherboard/).

[Download Schematics (PDF, 114 kB)](http://www.corentinaltepe.com/wp-content/uploads/2016/10/20161010_MOTHERBOARD_SCHEMATICS_REV2.pdf)

[Download Eagle Files (ZIP, 184 kB)](http://www.corentinaltepe.com/wp-content/uploads/2016/10/20161010_MOTHERBOARD_EAGLES_REV2.zip)

Two additional components are needed for the library to work properly:
* computation of CRC-4: provided in this project with crc.c and crc.h. You are free to replace these files with your own implementation.
* precise time tracking with timeouts: since the sensor needs between 0.60 ms and 18.06 ms to run an analog to digital conversion (ADC). A precise measure of time allows an optimal use of the processor (no time spent waiting by executing dummy cycles). Again, this function is provided with clocks.c and clocks.h but you are free to replace these functions with your own implementation.
