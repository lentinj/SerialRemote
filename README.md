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

The button on your board (if LUFA has configured one) will toggle the attached
PC awake / suspended.

Send characters at 9600 baud on the serial port. Attach serial transmit from a
FTDI or similar to ``PD2`` (or whichever is the serial receive pin). You could
use picocom to test, e.g:

    picocom -b9600 -fn /dev/ttyUSB0
    (press m for mute)

The firmware makes no attempt to buffer mutliple commands, it's assumed there's
a human the other end.

The full list of supported commands [can be seen in the source](SerialRemote.c#L202).

## Controlling from tasmota

The main reason for doing this is to be able to control a PC via. a
[tasmota enabled Sonoff](https://github.com/arendst/Tasmota).

Connect serial transmit pin from the ESP8266 to ``PD2`` (or whichever is the
serial receive pin).

Set baud rate with ``Baudrate 9600`` then send commands with ``SerialSend2 m``
for mute, e.g.

You can suspend/wake the computer in sync with the main relay:

    Rule1 1
    Rule1 ON Power1#state=1 DO SerialSend2 1 ENDON
          ON Power1#state=0 DO SerialSend2 0 ENDON

You could also use a remote to do play/pause:

    Rule1 1
    Rule1 ON IrReceived#Data=0x471 DO SerialSend2 " " ENDON
