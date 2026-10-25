#include "tiny_ssd1315.h"

#include <cstring>

#include "pico/stdlib.h"

void GPGFX_TinySSD1315::init(GPGFX_DisplayTypeOptions options) {
	_options.displayType = options.displayType;
	_options.i2c = options.i2c;
	_options.spi = options.spi;
	_options.size = options.size;
	_options.address = options.address;
	_options.orientation = options.orientation;
	_options.inverted = options.inverted;
	_options.font = options.font;

	// SSD1315 GDDRAM is 128x64. Do not probe SH1106 RMW.
	this->screenType = 0;

	const uint8_t mux = (_options.size == SIZE_128x32) ? 31 : 63;
	const uint8_t comPins = (_options.size == SIZE_128x32) ? 0x02 : 0x12;
	const bool remapSeg =
		(_options.orientation == 2) || (_options.orientation == 3);
	const bool scanNormal =
		(_options.orientation == 1) || (_options.orientation == 3);

	// Solomon SSD1315 Rev 1.1: 128x64, internal charge pump + internal IREF
	uint8_t commands[] = {
		0x00,
		CommandOps::DISPLAY_OFF,
		CommandOps::SET_DISPLAY_CLOCK_DIVIDE,
		0x80,
		CommandOps::SET_MULTIPLEX,
		mux,
		CommandOps::SET_DISPLAY_OFFSET,
		0x00,
		CommandOps::SET_START_LINE,
		CommandOps::MEMORY_MODE,
		0x00,
		CommandOps::SET_LOW_COLUMN,
		CommandOps::SET_HIGH_COLUMN,
		(uint8_t)(remapSeg ? CommandOps::SEGMENT_REMAP_0
				   : CommandOps::SEGMENT_REMAP_127),
		(uint8_t)(scanNormal ? CommandOps::COM_SCAN_NORMAL
				     : CommandOps::COM_SCAN_REVERSE),
		CommandOps::SET_COM_PINS,
		comPins,
		CommandOps::SET_CONTRAST,
		0xCF,
		CommandOps::SET_PRECHARGE,
		0xF1,
		CommandOps::SET_VCOM_DETECT,
		0x20,
		CommandOps::SET_IREF,
		IREF_INTERNAL_30UA,
		CommandOps::CHARGE_PUMP,
		CHARGE_PUMP_7V5,
		CommandOps::DEACTIVATE_SCROLL,
		CommandOps::FULL_DISPLAY_ON_RESUME,
		(uint8_t)(_options.inverted ? CommandOps::INVERT_DISPLAY
					    : CommandOps::NORMAL_DISPLAY),
	};

	sendCommands(commands, sizeof(commands));
	sleep_ms(10);
	sendCommand(CommandOps::DISPLAY_ON);
	_isPowered = true;

	clear();
	drawBuffer(NULL);
}

void GPGFX_TinySSD1315::setPower(bool isPowered) {
	_isPowered = isPowered;
	if (isPowered) {
		sendCommand(CommandOps::CHARGE_PUMP);
		sendCommand(CHARGE_PUMP_7V5);
		sendCommand(CommandOps::DISPLAY_ON);
	} else {
		sendCommand(CommandOps::DISPLAY_OFF);
		sendCommand(CommandOps::CHARGE_PUMP);
		sendCommand(CHARGE_PUMP_OFF);
	}
}

void GPGFX_TinySSD1315::drawBuffer(uint8_t *pBuffer) {
	uint8_t buffer[MAX_SCREEN_SIZE + 1];
	buffer[0] = I2C_DATA_MODE;

	sendCommand(CommandOps::COLUMN_ADDRESS);
	sendCommand(0x00);
	sendCommand(0x7F);
	sendCommand(CommandOps::PAGE_ADDRESS);
	sendCommand(0x00);
	sendCommand(0x07);

	if (pBuffer == NULL) {
		memcpy(&buffer[1], frameBuffer, MAX_SCREEN_SIZE);
	} else {
		memcpy(&buffer[1], pBuffer, MAX_SCREEN_SIZE);
	}

	_options.i2c->write(_options.address, buffer, sizeof(buffer), false);
}
