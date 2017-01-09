PORT=/dev/ttyUSB0

all: build

build:
	platformio run

upload:
	platformio run --target upload

upload-fs:
	platformio run --target buildfs && platformui --target uploadfs

serial:
	platformio device monitor -b 115200
