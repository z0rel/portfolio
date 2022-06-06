/*
 * display_context.h
 *
 * Created: 16.03.2022 14:07:26
 *  Author: artem
 */ 


#ifndef DISPLAY_CONTEXT_H_
#define DISPLAY_CONTEXT_H_

#include "../src/lcd/lcd_api_context.h"

namespace coroutines {

class DisplayContext {
public:
	/// Флаг частичной инвалидации
    bool valid = false;
	/// Флаг полной перезаписи экрана (для ускорения записи)
    bool valid_full = false;
	uint8_t i;

	LCD_driver::LCDApiContext ctx;

    void invalidate() { this->valid = false; }
	bool isValid() const { return this->valid; }

    void invalidateFull() { this->valid = true; this->valid_full = false; }
	bool isValidFull() const { return this->valid_full; }
};

extern DisplayContext displayContext;

}

#endif /* DISPLAY_CONTEXT_H_ */