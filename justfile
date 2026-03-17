# Set shell for command execution
set shell := ['bash', '-uc']

# Arduino Nano settings
fqbn := "arduino:avr:nano:cpu=atmega328old"
port := "/dev/ttyUSB0"
sketch := "blink"

# List available commands
default:
    @just --list

# Install arduino-cli and AVR core
setup:
    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh
    arduino-cli core update-index
    arduino-cli core install arduino:avr

# Compile the sketch
build:
    arduino-cli compile --fqbn {{fqbn}} {{sketch}}/

# Upload the sketch to the board
flash:
    arduino-cli upload -p {{port}} --fqbn {{fqbn}} {{sketch}}/

# Compile and upload
deploy: build flash

# Monitor serial output
monitor:
    arduino-cli monitor -p {{port}} -c baudrate=9600
