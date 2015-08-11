# energenie-bitbang
Using an FTDI FT232R chip in BitBang mode to control a 433MHz Energenie power socket via a simple 433MHz transmitter.


This has been tested on Ubuntu 15.04 x64 and Ubuntu 14.04.2 LTS x32

Requirements
------------

An Energenie or compatible power socket.

A 433MHz transmitter such as the ubiquitous FS1000A (eBay, ~ £1)

A USB to TTL FT232RL adapter (eBay, ~ £2)

Compile
-------

sudo apt-get install libftdi-dev libftdi1 libusb-dev

git clone https://github.com/richud/energenie-bitbang.git

cd energenie-bitbang

gcc energenie.c -lftdi -o energenie

Hardware Setup
--------------
Attach Ground <> Ground

Any bitbang pin to DATA and any to VCC. (Set in energenie.c file in the #define lines.)

Usage
-----

./energenie code [serial]

e.g.

sudo ./energenie 123456 FMJ8095F

Programing it with the above code will mean 123456 switches on, 123455 off.

