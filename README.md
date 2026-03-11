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
west build -p always -b nrf54h20dk/nrf54h20/cpuapp
west flash
```

## Monitor serial output

The nRF54H20DK enumerates multiple CDC-ACM serial ports over USB. APP core logs
appear on one of the `/dev/ttyACM*` devices at **115200 baud**.

Install `pyserial` if needed:

```bash
pip install pyserial
```

Find the right port (plug in the board first):

```bash
ls /dev/ttyACM*
```

The first port (`/dev/ttyACM0`) is usually the APP core. If it shows no output,
try the next one (`/dev/ttyACM1`, etc.).

```bash
python3 -m serial.tools.miniterm /dev/ttyACM0 115200
```

Exit with `Ctrl-]`.

After flashing, reset the board (press the reset button) to see startup logs:

```
[00:00:00.000] Ultimate Flipboard starting
[00:00:00.001] Ready — BTN1=A+  BTN2=B+  BTN3=A−  BTN4=reset
[00:00:00.001] Score │ Team A:  0  │  Team B:  0
```

# Testing

Unit tests cover the score logic and run locally without any hardware.

## Run tests

Activate the toolchain (if not already done), then build and run:

```bash
cd /path/to/sdk-nrf/v3.2.3

west build -p always -b native_sim/native/64 --no-sysbuild \
  /path/to/ultimate-flipboard/tests/scoreboard

./build/zephyr/zephyr.exe
```

Expected output:

```
Running TESTSUITE scoreboard
===================================================================
START - test_dec_a_floors_at_zero
 PASS - test_dec_a_floors_at_zero in 0.000 seconds
...
SUITE PASS - 100.00% [scoreboard]: pass = 8, fail = 0, skip = 0, total = 8
PROJECT EXECUTION SUCCESSFUL
```

## Adding tests

Tests live in `tests/scoreboard/src/main.c`. Score logic lives in `src/scoreboard.c`
and `include/scoreboard.h` — it has no Zephyr or hardware dependencies, so new
functions are easy to test. Add a `ZTEST` block for each new behaviour:

```c
ZTEST(scoreboard, test_my_new_case)
{
    scoreboard_t sb;
    scoreboard_init(&sb);
    /* ... */
    zassert_equal(sb.score_a, expected);
}
```
