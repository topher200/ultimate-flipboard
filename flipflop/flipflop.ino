/*
 * Flipdots black/white flip-flop test
 *
 * Toggles a 7×28 AlfaZeta flipdot panel between all-white and all-black
 * every 300 ms over RS-485 at 9600 baud.
 *
 * Wiring:
 *   Arduino Nano pin 3 (TX) → RS-485 breakout RXD
 *   Arduino Nano GND        → RS-485 breakout GND
 *   RS-485 breakout A/B     → Flipdot panel
 */

#include <SoftwareSerial.h>

#define RS485_TX_PIN 3
#define RS485_RX_PIN 2  /* unused, but SoftwareSerial requires it */

#define FLIP_INTERVAL_MS 300

/* Protocol constants */
#define FRAME_START 0x80
#define FRAME_END   0x8F
#define CMD_REFRESH 0x83
#define ADDR_BROADCAST 0xFF
#define DISPLAY_WIDTH 28

SoftwareSerial rs485(RS485_RX_PIN, RS485_TX_PIN);

static bool is_white = false;

void sendFrame(bool white) {
  uint8_t data = white ? 0x7F : 0x00;

  rs485.write(FRAME_START);
  rs485.write(CMD_REFRESH);
  rs485.write(ADDR_BROADCAST);
  for (int i = 0; i < DISPLAY_WIDTH; i++) {
    rs485.write(data);
  }
  rs485.write(FRAME_END);
}

void setup() {
  Serial.begin(9600);
  rs485.begin(9600);

  Serial.println("Flipdots flip-flop test starting");
}

void loop() {
  is_white = !is_white;
  sendFrame(is_white);
  Serial.println(is_white ? "WHITE" : "BLACK");
  delay(FLIP_INTERVAL_MS);
}
