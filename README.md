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
