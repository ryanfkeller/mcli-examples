SHELL := /bin/bash 

.PHONY: 

BUILD_DIR := build
SKETCH := arduino_serial_mcli.ino
PORT := /dev/ttyACM0
BAUD := 115200

help:
	@echo "********************************"
	@echo "* Arduino INO Makefile *"
	@echo "********************************"
	@echo ""
	@echo "Available targets:"
	@echo "  build     -- Compiles the .ino file into a loadable application"
	@echo "Usage:"
	@echo "  make [target]"
	@echo "  For example, run 'make build' to build the application."


mkdirs:
	@mkdir -p $(BUILD_DIR)

build: mkdirs
	@echo "Building $(SKETCH)"
	arduino-cli compile \
		--fqbn arduino:avr:uno \
		--library "../libs/mcli/include/mcli" \
		--library "../libs/mcli/include/adapters" \
		--build-path $(BUILD_DIR) \
		$(SKETCH)

upload: 
	@echo "Uploading $(SKETCH)"
	arduino-cli upload \
		--fqbn arduino:avr:uno \
		--input-dir $(BUILD_DIR) \
		--port $(PORT)

monitor:
	@echo "Monitoring $(PORT) at $(BAUD) baud with screen"
	screen $(PORT) $(BAUD)

all: build upload monitor
