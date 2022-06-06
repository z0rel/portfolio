/*
 * lcd_commands.h
 *
 * Created: 16.03.2022 6:52:32
 *  Author: artem
 */


#ifndef LCD_COMMANDS_H_
#define LCD_COMMANDS_H_


#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00


#define LCD_COMMAND_GO_TO_2_LINE 0xC0

#define LCD_COMMAND_DISPLAY_OFF (LCD_CURSOROFF | LCD_BLINKOFF)
#define LCD_COMMAND_DISPLAY_ON (LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF)

#define LCD_COMMAND_CURSOR_OFF__BLINK_OFF (LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF)
#define LCD_COMMAND_CURSOR_ON__BLINK_OFF (LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKOFF)

#define LCD_COMMAND_CURSOR_ON__BLINK_ON (LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON)
#define LCD_COMMAND_CURSOR_OFF__BLINK_ON (LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKON)

#define LCD_COMMAND_SCROLL_DISPLAY_LEFT (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT)
#define LCD_COMMAND_SCROLL_DISPLAY_RIGHT (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT)

// This is for text that flows Left to Right
#define LCD_COMMAND_TEXT_FLOW_LEFT_TO_RIGHT(FLAGS) (LCD_ENTRYLEFT | LCD_ENTRYMODESET | (FLAGS))

// This is for text that flows Right to Left
#define LCD_COMMAND_TEXT_FLOW_RIGHT_TO_LEFT(FLAGS) (LCD_ENTRYMODESET | (FLAGS))

// This will 'right justify' text from the cursor
#define LCD_COMMAND_TEXT_AUTOSCROLL_ON(FLAGS) (LCD_ENTRYSHIFTINCREMENT | LCD_ENTRYMODESET | (FLAGS))

// This will 'left justify' text from the cursor
#define LCD_COMMAND_TEXT_AUTOSCROLL_OFF(FLAGS) (LCD_ENTRYMODESET | (FLAGS))


#endif /* LCD_COMMANDS_H_ */
