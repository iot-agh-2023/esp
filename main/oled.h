#include "driver/spi_master.h"

typedef struct {
	bool _valid; // Not using it anymore
	int _segLen; // Not using it anymore
	uint8_t _segs[128];
} PAGE_t;

typedef struct {
	int _address;
	int _width;
	int _height;
	int _pages;
	int _dc;
	spi_device_handle_t _SPIHandle;
	bool _scEnable;
	int _scStart;
	int _scEnd;
	int _scDirection;
	PAGE_t _page[8];
	bool _flip;
} SSD1306_t;

void init_display(void);

void display_text(int line, char const* text);