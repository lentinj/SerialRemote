# SerialRemote: Serial-based remote control of connected PC

Firmware for AVR-based devices to emulate a USB multimedia keyboard, allowing
the following actions to be performed over a serial link:

* Wake PC from suspend
* Suspend / Hibernate keys
* Multimedia keys: play/pause/volume up/etc (e.g. for operation via. an infra-red remote)
* Keyboard keys: space

The aim is to control a lounge PC via. a tasmota-enabled sonoff device.

Uses the LUFA library, so supports anything LUFA supports, currently only
tested on Minimus at90usb162 devices.

Should work independently of the OS used on the attached PC, works at least
with Windows 10 and Linux.

## Building and installing

    apt install gcc-avr avr-libc dfu-programmer avrdude

Clone the code and make sure LUFA is downloaded with:

    git submodule update --init

Then edit ``makefile`` to suit your board. This is a standard LUFA makefile,
``MCU``, ``ARCH``, ``BOARD`` and ``F_CPU`` are particular values to pay
attention to.

To build and flash to an attached chip in DFU mode:

     make && make dfu

## Enabling wake from suspend 

Under linux, you need to tweak the appropriate ``power/wakeup`` file, e.g.:

    echo enabled > /sys/devices/pci0000:00/0000:00:1d.0/usb4/4-1/4-1.8/4-1.8.3/power/wakeup

Under windows you can find a "Allow this device to wake the computer" option in
Device Manager.

## Controlling

Commands can come from several sources:

### Button

For an Arduino Pro Micro, PB6 / pin 10 is configured as a suspend/wake toggle
in [Buttons.h](Board/Buttons.h).

For other ``BOARD``s, their ``BUTTONS_BUTTON1`` is used if available.

### Infra-red

Commands can also be sent over IR, the main reason for doing this is to be able
to control a PC via. a [tasmota enabled Sonoff](https://github.com/arendst/Tasmota).

The input from the infrared receiver is expected to be on PD1 / pin 2, and PD0
/ pin 3 is kept high to power the receiver to make wiring easy.

It listens for NEC-encoded commands with address 0x42, these can be sent with
[tasmota configured for IR transmitting](https://tasmota.github.io/docs/IR-Remote/):

* Power on:  ``IRsend {"Protocol":"NEC","Bits":32,"Data":0x42bd807f}``
* Power off: ``IRsend {"Protocol":"NEC","Bits":32,"Data":0x42bd00ff}``
* Space " ": ``IRsend {"Protocol":"NEC","Bits":32,"Data":0x42bd04fb}``

The [encode.py] script can generate strings for other commands.

### Serial

Send characters at 9600 baud on the serial port. Attach serial transmit from a
FTDI or similar to ``PD2`` (or whichever is the serial receive pin). You could
use picocom to test, e.g:

    picocom -b9600 -fn /dev/ttyUSB0
    (press m for mute)

The firmware makes no attempt to buffer mutliple commands, it's assumed there's
a human the other end.

The full list of supported commands [can be seen in the source](SerialRemote.c#L202).

Note: Whilst you could connect a Sonoff directly over the serial link this is
**not recommended** as the Sonoff's ground may not be isolated from mains.
