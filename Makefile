SKETCH=sketch/sketch.ino
BOARD=esp8266:esp8266:nodemcu
PORT=/dev/ttyUSB0

verify:
	arduino --board $(BOARD) --verify $(SKETCH)

upload:
	arduino --board $(BOARD) --upload $(SKETCH) --port $(PORT)
