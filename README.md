Ultimate Flipboard

A flipdots-based scoreboard that keeps track of ultimate scores, including a MMP/WMP tracker.

Uses a [flipdots board](https://flipdots.com/en/products-services/flip-dot-boards-xy5/) from Flipdots.com.

# Installation

## Install nRF SDK

Follow [Nordic's instructions](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/device_guides/nrf54h/ug_nrf54h20_gs.html) for installing the SDK for an nRF54H20.

- install `nrfutil`
- install `sdk-manager`
- install toolchain
- clone `sdk-nrf` repo with `west`
- install nrfConnect for Desktop
  - install Serial Terminal
- download and install `bicr.hex`
- download and install the IronSide SE binaries
- `west build` and `west flash`

## Activate the toolchain environment

Before running `west`, activate the nRF toolchain in your shell:

```bash
source <(nrfutil toolchain-manager env --as-script --ncs-version v3.2.3)
```

Run this in each new terminal session before building. Then from this repo:

```bash
west build -p always -b nrf54h20dk/nrf54h20/cpuapp --sysbuild
west flash
```

## Monitor serial output

The nRF54H20DK enumerates multiple CDC-ACM serial ports over USB. APP core logs
appear on one of the `/dev/ttyACM*` devices at **115200 baud**.

Install `picocom` if needed:

```bash
sudo apt install picocom
```

Find the right port (plug in the board first):

```bash
ls /dev/ttyACM*
```

The first port (`/dev/ttyACM0`) is usually the APP core. If it shows no output,
try the next one (`/dev/ttyACM1`, etc.).

```bash
picocom /dev/ttyACM0 -b 115200
```

Exit with `Ctrl-A Ctrl-X`.

After flashing, reset the board (press the reset button) to see startup logs:

```
[00:00:00.000] Ultimate Flipboard starting
[00:00:00.001] Ready — BTN1=A+  BTN2=B+  BTN3=A−  BTN4=reset
[00:00:00.001] Score │ Team A:  0  │  Team B:  0
```
