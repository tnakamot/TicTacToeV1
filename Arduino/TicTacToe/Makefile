# This is the makefile for WSL2.
#
# Override PORT variable with an appropriate COM port
# where Arduino Nano (CH340) is connected.
PORT      := COM4

# Baud rate of the arduino program
BAUD      := 57600

SRC_DIR   := $(dir $(lastword $(MAKEFILE_LIST)))
BUILD_DIR := ${SRC_DIR}build
FQBN      := arduino:avr:nano:cpu=atmega328old

SRC_PATH   := $(shell wslpath -w ${SRC_DIR})
BUILD_PATH := $(shell mkdir -p ${BUILD_DIR} && wslpath -w ${BUILD_DIR})
ARDUINO_CLI_BIN := ./arduino-cli.exe
ARDUINO_CLI_WIN64_ZIP := arduino-cli_latest_Windows_64bit.zip
ARDUINO_CLI_WIN64_URL := https://downloads.arduino.cc/arduino-cli/${ARDUINO_CLI_WIN64_ZIP}

SERIAL_MONITOR := cmd.exe /C "C:\Program Files (x86)\SimplySerial\ss.exe" -baud:${BAUD} -com:${PORT}

arduino-cli.exe:
	wget ${ARDUINO_CLI_WIN64_URL}
	unzip ${ARDUINO_CLI_WIN64_ZIP} $@
	rm ${ARDUINO_CLI_WIN64_ZIP}

.PHONY: compile
compile: ${ARDUINO_CLI_BIN}
	${ARDUINO_CLI_BIN} compile --verbose --output-dir "${BUILD_PATH}" --fqbn ${FQBN} "${SRC_PATH}"
	@echo "Generated binary files in ${BUILD_DIR}"

.PHONY: upload
upload: compile
	${ARDUINO_CLI_BIN} upload --verbose --port ${PORT} --fqbn ${FQBN} --input-dir "${BUILD_PATH}"

.PHONY: monitor
monitor: upload
	${SERIAL_MONITOR}

.PHONY: clean
clean:
	rm -rf ${BUILD_DIR}

.DEFAULT_GOAL := monitor
