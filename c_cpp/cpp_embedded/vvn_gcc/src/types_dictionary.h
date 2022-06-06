/*
 * types_dictionary.h
 *
 * Created: 01.03.2022 7:12:15
 *  Author: artem
 */ 


#ifndef TYPES_DICTIONARY_H_
#define TYPES_DICTIONARY_H_

#include <avr/eeprom.h>
#include "types_dictionary_int.h"

extern EepromFirstInitializationKey EEMEM eeprom_first_initialization_key;

namespace Config {
    class ConfigVVN8;
    extern ConfigVVN8 config_vvn8;
}




#endif /* TYPES_DICTIONARY_H_ */