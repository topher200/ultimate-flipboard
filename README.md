# Ultimate Flipboard

A flipdots-based scoreboard that keeps track of ultimate scores, including a MMP/WMP tracker.

Uses a [flipdots board](https://flipdots.com/en/products-services/flip-dot-boards-xy5/) from Flipdots.com.

## Hardware

- **Microcontroller**: Arduino Nano (ATmega328P, old bootloader)
- **Display**: Flipdots 7×28 panel via RS485

## Setup

Install arduino-cli and the AVR core:

```bash
just setup
```

## Usage

```bash
just build    # compile the sketch
just flash    # upload to the board
just deploy   # compile + upload
just monitor  # serial monitor
```

The Nano connects as `/dev/ttyUSB0` (FTDI FT232R). Override the port or sketch with:

```bash
just port=/dev/ttyUSB1 flash
just sketch=my_other_sketch deploy
```

## Other branches

We've tried to build this project for a few different microcontrollers. Other branches in this repo are:

- nrf54h20 - nRF54H20DK development board
  - Configured to send test frames to the flipdots display. A signal is sent but
  is not working.
