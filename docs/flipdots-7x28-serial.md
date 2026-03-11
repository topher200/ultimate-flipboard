# Flipdots 7x28 Serial Protocol Documentation

This document describes the serial (RS485) communication protocol for controlling a single **7×28 AlfaZeta flipdots display panel**.

## Frame Format

The serial protocol uses a fixed frame structure:

```
[0x80] [Command] [Address] [Data...] [0x8F]
```

- **0x80**: Frame start byte
- **Command**: Operation command (see below)
- **Address**: Device address (0x01-0xFE for specific device, 0xFF for broadcast)
- **Data**: Variable-length payload (28 bytes for 7×28 displays)
- **0x8F**: Frame end byte

## Commands for 7×28 Display

| Command | Data Bytes | Refresh? | Description |
|---------|------------|----------|-------------|
| 0x82    | 0          | YES      | Global refresh — refresh all connected displays |
| 0x83    | 28         | YES      | 7×28 display update with immediate refresh |
| 0x84    | 28         | NO       | 7×28 display update (buffered, wait for refresh) |

For a single 7×28 panel, we typically use **command 0x83** (refresh=YES) for simplicity, as it updates the display immediately without requiring a separate refresh command.

## Data Format

The 7×28 display requires **28 data bytes**, one byte per vertical column (x-coordinate 0-27).

### Bit Mapping

Each byte represents a 7-dot vertical column:
- **Bit 0 (LSB)**: Top dot (y=0)
- **Bit 1**: Second dot (y=1)
- **Bit 2**: Third dot (y=2)
- **Bit 3**: Fourth dot (y=3)
- **Bit 4**: Fifth dot (y=4)
- **Bit 5**: Sixth dot (y=5)
- **Bit 6**: Bottom dot (y=6)
- **Bit 7 (MSB)**: **Must be set to 0** (ignored by hardware)

### Coordinate System

- **X-axis**: 0-27 (left to right, 28 columns)
- **Y-axis**: 0-6 (top to bottom, 7 rows)

To set pixel at (x, y):
```c
buf[x] |= (1u << y);   // Turn dot ON (white/visible)
buf[x] &= ~(1u << y);  // Turn dot OFF (black/hidden)
```

## Example Frames

### All Dots Black (All Hidden)

For a 7×28 display at address 0x01:
```
80 83 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 8F
```

For broadcast (address 0xFF):
```
80 83 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 8F
```

### All Dots White (All Visible)

For a 7×28 display at address 0x01:
```
80 83 01 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 7F 8F
```

Each `0x7F` byte sets bits 0-6 (all 7 dots visible), with bit 7 = 0.

### Single Pixel Example

To light up only the top-left pixel (x=0, y=0):
```c
uint8_t buf[28] = {0};
buf[0] = 0x01;  // Bit 0 set, all others 0
// Frame: 80 83 01 01 00 00 ... (28 bytes total) ... 8F
```

To light up the bottom-right pixel (x=27, y=6):
```c
uint8_t buf[28] = {0};
buf[27] = 0x40;  // Bit 6 set (0x40 = 0b01000000)
// Frame: 80 83 01 00 00 ... (27 zeros) ... 40 8F
```

## RS485 Hardware Setup

### Connection

- Connect the flipdots panel to your controller via RS485
- Ensure proper termination resistors if using a bus topology
- Check DIP switch settings on the panel for address configuration

### Serial Parameters

Typical settings (verify with your panel documentation):
- **Baud rate**: 9600 or 19200 (check panel DIP switches)
- **Data bits**: 8
- **Parity**: None
- **Stop bits**: 1
- **Flow control**: None

### Refresh=YES vs Refresh=NO (buffered updates)

For 7×28 panels there are two update commands:

- `0x83` — **Refresh=YES**: the panel updates its dots as the DATA bytes are received  
- `0x84` — **Refresh=NO**: the panel stores the DATA bytes in internal memory and does **not** update the visible dots until it receives a global refresh sequence

The global refresh sequence is:

```text
80 82 FF 8F
```

- `0x80` — frame start  
- `0x82` — “format / refresh all displays” command (no data bytes)  
- `0xFF` — broadcast address (all panels)  
- `0x8F` — frame end  

This allows you to:

1. Send `0x84` frames (Refresh=NO) to one or more panels to stage new content  
2. When all content is staged, send the `0x82` refresh frame once so that all panels update their displays **synchronously**

In this project we use `0x83` (Refresh=YES) for simplicity, but `0x84` plus a later `0x82` is useful if you need synchronized updates across multiple 7×28 panels.
