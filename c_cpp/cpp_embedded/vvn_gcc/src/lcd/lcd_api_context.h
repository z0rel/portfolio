/*
 * lcd_api_context.h
 *
 * Created: 18.03.2022 11:40:40
 *  Author: artem
 */


#ifndef LCD_API_CONTEXT_H_
#define LCD_API_CONTEXT_H_


namespace LCD_driver {

union LCDApiContext {
    uint8_t c;
    uint8_t command;
    uint8_t data;

    struct ContextLcdString {
        const char *str;
        uint8_t i;
        uint8_t c;
        inline void set(const char *_str) { this->str = _str; }
    } lcdString;

    struct ContextLcdStringRestricted {
        const char *str;
        uint8_t i;
        uint8_t len;
        uint8_t c;
    } stringRestricted;

    struct ContextLcdStringPrgmem {
        const char *str PROGMEM;
        uint8_t i;
        uint8_t c;
    } stringPrgmem;

    struct ContextLcdStringXY {
        uint8_t row;
        uint8_t pos;
        ContextLcdStringRestricted lcd_string_ctx;
    } stringXY;

    struct ContextLcdCharXY {
        uint8_t row;
        uint8_t pos;
        char c;
        uint8_t cmd;

        inline void set(uint8_t _row, uint8_t _pos, uint8_t _c)
        {
            this->row = _row;
            this->pos = _pos;
            this->c   = _c;
        }
    } charXY;
};

}  // namespace LCD_driver


#endif /* LCD_API_CONTEXT_H_ */
