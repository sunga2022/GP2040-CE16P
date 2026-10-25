#ifndef _GPGFX_TINYSSD1315_H_
#define _GPGFX_TINYSSD1315_H_

#include "tiny_ssd1306.h"

// 0.96" 128x64 OLED, Solomon SSD1315, I2C 0x3C/0x3D
class GPGFX_TinySSD1315 : public GPGFX_TinySSD1306 {
	public:
		GPGFX_TinySSD1315() {}
		~GPGFX_TinySSD1315() {}

		void init(GPGFX_DisplayTypeOptions options);
		void setPower(bool isPowered);
		void drawBuffer(uint8_t *pBuffer);

		std::vector<uint8_t> getDeviceAddresses() const override {
			return {0x3C, 0x3D};
		}

	private:
		enum CommandOps : uint8_t {
			SET_LOW_COLUMN = 0x00,
			SET_HIGH_COLUMN = 0x10,
			MEMORY_MODE = 0x20,
			COLUMN_ADDRESS = 0x21,
			PAGE_ADDRESS = 0x22,
			DEACTIVATE_SCROLL = 0x2E,
			SET_START_LINE = 0x40,
			SET_CONTRAST = 0x81,
			CHARGE_PUMP = 0x8D,
			SEGMENT_REMAP_0 = 0xA0,
			SEGMENT_REMAP_127 = 0xA1,
			FULL_DISPLAY_ON_RESUME = 0xA4,
			NORMAL_DISPLAY = 0xA6,
			INVERT_DISPLAY = 0xA7,
			SET_MULTIPLEX = 0xA8,
			SET_IREF = 0xAD,
			DISPLAY_OFF = 0xAE,
			DISPLAY_ON = 0xAF,
			COM_SCAN_NORMAL = 0xC0,
			COM_SCAN_REVERSE = 0xC8,
			SET_DISPLAY_OFFSET = 0xD3,
			SET_DISPLAY_CLOCK_DIVIDE = 0xD5,
			SET_PRECHARGE = 0xD9,
			SET_COM_PINS = 0xDA,
			SET_VCOM_DETECT = 0xDB,
		};

		// I2C control: Co=0, D/C#=1 — GDDRAM write
		static const uint8_t I2C_DATA_MODE = 0x40;

		// Charge pump: 0x14 = 7.5V enable, 0x10 = disable
		static const uint8_t CHARGE_PUMP_7V5 = 0x14;
		static const uint8_t CHARGE_PUMP_OFF = 0x10;

		// A[4]=1 internal IREF, A[5]=1 30uA / max ISEG 240uA
		static const uint8_t IREF_INTERNAL_30UA = 0x30;
};

#endif
