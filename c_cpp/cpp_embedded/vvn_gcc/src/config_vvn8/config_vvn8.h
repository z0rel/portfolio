/*
 * IncFile1.h
 *
 * Created: 12.03.2022 11:58:13
 *  Author: artem
 */


#ifndef CONFIG_VNN8_H_
#define CONFIG_VNN8_H_


#include <stdint.h>

#include "../../interpolation/ConstrainedSpline.h"
#include "../../interpolation/Linear.h"

#include "mapping_values.h"

#include "internal/config_interpolation.h"
#include "internal/config_modbus.h"
#include "internal/config_password.h"
#include "internal/config_vvn8_packed.h"
#include "internal/config_vvn8_unpacked.h"


namespace config {

// typedef std::basic_string<std::string::value_type, std::char_traits<std::string::value_type>,
// Mallocator<std::string::value_type>> Mstring;

void first_init_eeprom();

extern ConfigVVN8 config_vvn8;

}  // namespace config


#endif /* INCFILE1_H_ */
