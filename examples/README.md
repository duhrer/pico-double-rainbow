# Examples

This page covers two examples of how I used this code to create light
installations.  One is small scale, i.e. a light fixture.  The other is larger
scale, where the outer ring is 2 metres in diametre.

## Light Fixture

The smaller (and first) installation I made replaced the guts of a thrift store
light fixture with an [RP2040 Stamp Round
Carrier](https://www.solder.party/docs/rp2040-stamp/round-carrier/) and[a ring
of 60
Neopixels](https://www.tinytronics.nl/nl/verlichting/ringen-en-modules/ws2812b-digitale-5050-rgb-led-ring-60-leds).

Once I knew what I had and what I wanted to put in, I started by making
measurements and diagrams to figure out what would fit where. Here's the frosted
glass dome:

![The Frosted Glass Dome](./light-fixture/dome.svg)

There were two components to the original base, a polished outer shell, and an interior "cup".

![The outer shell](./light-fixture/base-ring.svg)

![The interior "cup"](./light-fixture/base-sleeve.svg)

There wasn't much room to install components in the original shell and cup, so I
decided to keep the frosted glass dome and build a new base out of layers of
laser-cut bamboo:

![A rendered "fit test" for the laser cut components](models/base-fit-test.png)

You can't see from this angle, but the final designs included mounting slots to
slide the unit onto screws mounted in the wall.

## Ceiling Installation

For this version, I used the [Pimoroni Plasma
2040](https://shop.pimoroni.com/products/plasma-2040?variant=39410354847827) and
[a 10 meter cable of
"stars"](https://shop.pimoroni.com/products/10m-addressable-rgb-led-star-wire?variant=41375620530259).

I used a single larger strand of NeoPixels to form two rings, with a band set
aside to act as the "outer" band, an unlit "bridge" band, and another band that
acts as the "inner" ring:

![The "rings"](./room-sized/light-string-layout.svg)

The case for the components was a small aluminium tin very reminescent of the
old Altoids tins, which I hacked various holes into for wall mounting, cables,
and to allow IR signals to reach the inside.
