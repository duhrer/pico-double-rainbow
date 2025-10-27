# Pico "Double Rainbow"

This project configures a Pico(2) microcontroller and some Neopixels to act as a
"double rainbow" of nested light rings. You can use it for a light fixture sized
setup, for a room-scale setup, or anything in between.

## Prerequisites

### Hardware

First, you need the right hardware:

1. An RP2040 or RP2035 microcontroller
2. Enough
   [NeoPixels](https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels)
   to make two rings, ideally 60 or more in total
4. An IR receiver and remote (see below)
5. Something to install it in or on (see below)

The NeoPixels are the biggest concern, and there a lot of ways to get there. The
biggest decision is whether to have two "strands" of lights or one. See the
[examples page](./examples/) for how I used both styles.  See below for the pins
used for each configuration.

I used complete unmodified rings and strings for my work. You could also cut
NeoPixel strips to size and arrange those. If you're doing that, you have the
option to use either mode depending on how many free GPIO pins you have and how
you want to be able to route the wiring for each ring.

#### "Double" NeoPixel Strand

The default mode uses a different GPIO pin for an "inner" and "outer" ring of
NeoPixels. The inner ring uses (GPIO 24) and is configured for 16 lights. The
"outer" ring uses GPIO26, and is configured for 60 lights. 

#### "Single" NeoPixel Strand

In this mode, a single larger strand of NeoPixels is arranged to form two rings,
with a band set aside to act as the "outer" band, an unlit "bridge" band, and
another band that acts as the "inner" ring. The single strand is connected to
GPIO24. The outer ring is configured to use the first 40 lights in the string.
The inner ring is configured to use the last 20 lights in the string.  The 6
lights between the two rings are unlit.

#### Infrared Receiver and Remote

Most of the functions used in this project are accessed using an IR receiver and
remote. I used [this IR
receiver](https://www.tinytronics.nl/en/communication-and-signals/wireless/infrared/ir-infrared-receiver-module-38khz-940nm)).
Connect the `ground` and `vcc` pins to the `ground` and `3v3` headers on your
microcontroller. Connect the `out` pin to the header for `GPIO5`. Note that this
component only works if it has enough of an unobstructed "line of sight" from
outside, so you'll need to design your mounting arrangement with that in mind.

I tested this project with [the remote from this
kit](https://www.tinytronics.nl/en/communication-and-signals/wireless/infrared/ir-sensor-module-with-remote-and-battery-with-ir-led)
and [the remote for the lights I
have](https://www.amazon.nl/-/en/Changing-Dimmable-Control-Colours-Decoration/dp/B06XYFZ4J5/ref=sr_1_1?sr=8-1).
This type of remote seems incredibly common in kits, and is also readily
available online.

#### Something to Install It In or On

This is a big topic, and you can do what makes sense for you, but if you want
serving suggestions, check out some [examples](./examples/) I created.

### Software

Once you have something to run the code on, you'll need to set up a build
environment. In the past, I have used:

1. The [Getting Started with Pico Guide](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
2. The [Pico VS Code extension](https://github.com/raspberrypi/pico-vscode)

Both of those are probably the easiest starting points, and if you hit upon
questions, there are lots of people working with them, so hopefully you can find
the guidance you need.

Personally, I use [distrobox](https://distrobox.it/) and [the docker container
created by `lukstep`](https://github.com/lukstep/raspberry-pi-pico-docker-sdk),
which I set up using commands like:

```
distrobox-create --image lukstep/raspberry-pi-pico-sdk -n pico-sdk
distrobox enter pico-sdk
```

## Changing the Configuration

A few things are hard coded that you might want to change:

1. The GPIO pins used for the neopixel strands
2. The number of lights in each "ring"
3. The GPIO pin used for the IR receiver
4. The default brightness
5. The default "mode"

You can do change these by editing the compiler directives (`define` statements)
in `src/common_defaults.h` and `src/rainbow.cpp`.

## Building and Installing

### VS Code and a PiProbe

If you have a
[PiProbe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html),
you should be able to use the debugger configuration in this project to build,
install (and debug) the code in this project.

First, you should check the paths in `.vscode/launch.json`and
`.vscode/settings.json` and update them as needed to match your system. Then,
you should be able to just hit the debugger icon and choose the configuration
defined in `.vscode/launch.json`.

The application will be built, deployed, and will pause execution at the
beginning of the `main()` function.

### Manual Build

To build the application from the command line, you can use commands like the
following, starting at the root the repository:

```
mkdir build
cd build
cmake ..
make -j16
```

The last command assumes you have sixteen cores, adjust as needed. Once the
build completes, there are two ways to install the application.

### Installing

This project builds two variants on the same project depending on the number of
NeoPixel "strings" you have (see above), you can either use:

1. `pico-double-rainbow`: For units with NeoPixels on two separate GPIO pins.
2. `pico-combined-rainbow`: For units with NeoPixels on a single GPIO pin.

The remaining examples here will use the `pico-double-rainbow` binary name.

If you don't have a PiProbe, reboot your Pico while holding the "Bootsel"
button, then copy or drag the generated UF2 file `pico-double-rainbow.uf2` onto
the USB drive that appears.

If you're lucky enough to have a board with a reset button, all of the binaries
in this project also support entering `bootsel` mode by pressing the reset
button twice. You then copy the `pico-double-rainbow.uf2` file to the USB drive
as described above.

If you have a PiProbe, you can install the program without resetting your Pico
using a command like:

```
sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program pico-double-rainbow.elf verify reset exit"
```

## Usage

Once you've put together the hardware and built and installed the binary, the
unit should start up in the default "rainbow" mode, in which the three colour
channels (red, green, and blue) are represented as bands that move at different
speeds around the rings. The bands are mixed wherever they overlap. The inner
and outer rings move in different directions.

If you want to use any other mode, you'll need to either change the defaults
(see above) or have an IR receiver and remote. Most of the functions on the
Practical Series II remote are supported, i.e. you can turn the lights off and
on, change the brightness level, and choose to either display a single colour or
to use one of the two "rainbow" modes. The first rainbow mode cycles through six
colours (the rainbow, basically, but I don't distinguish between indigo and
violet). The second is the default "double rainbow" mode described above.
