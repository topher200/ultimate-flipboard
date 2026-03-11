# nRF54H20DK UART/RS485 Configuration for Flipdots

Hardware wiring and configuration notes for connecting the nRF54H20DK to the flipdots display via RS485.

## Hardware Connections

Connect your nRF54H20DK board to an RS485 breakout board:

1. **TX Pin** (nRF → RS485): Connect the nRF board's TX pin to the RS485 breakout's **RXD** input
2. **RX Pin** (RS485 → nRF): Connect the nRF board's RX pin to the RS485 breakout's **TXD** output
3. **GND**: Connect ground between both boards
4. **RS485 A/B**: Connect the breakout's differential pair (A/B terminals) to your flipdots display
5. **Power**: Power the RS485 breakout (commonly 5V) according to its datasheet

Most RS485 breakout boards (e.g., MAX485-based boards) handle differential signaling automatically. Some boards expose **DE/RE** (Driver Enable/Receiver Enable) pins — for simple one-direction transmit to the flipdots display, these are often tied together and driven high, or managed automatically by the board.

## Pin Selection

The overlay file (`boards/nrf54h20dk_nrf54h20_cpuapp.overlay`) contains example pins:

- **TX**: Example uses `P0.02` (change `NRF_PSEL(UART_TX, 0, 2)` to match your pin)
- **RX**: Example uses `P0.03` (change `NRF_PSEL(UART_RX, 0, 3)` to match your pin)

Choose any available GPIOs on port 0:

- Avoid pins already used for buttons, LEDs, or other peripherals
- Check the nRF54H20DK pinout or schematic for conflicts

## Baud Rate

Set the `current-speed` property in the overlay to match your flipdots panel:

- `9600` — common default for flipdots panels
- `19200` — some panels support this higher rate

The baud rate must match the DIP switch configuration on the flipdots controller.

## Troubleshooting

If you see `Flipdots UART device not found` or `UART device not ready` in logs:

- Verify the overlay file exists at `boards/nrf54h20dk_nrf54h20_cpuapp.overlay`
- Check that pin assignments in the overlay match your actual wiring
- Ensure `status = "okay"` is set on `&uart1` in the overlay
