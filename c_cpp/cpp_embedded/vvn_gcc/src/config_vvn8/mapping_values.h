/*
 * mapAdcToDouble.h
 *
 * Created: 28.03.2022 10:19:33
 *  Author: artem
 */


#ifndef MAPPING_VALUES_H_
#define MAPPING_VALUES_H_

#include <math.h>
#include <stdint.h>

#include "internal/config_meas_common.h"


namespace adc {


enum class ADC_SCALERS : int32_t {
    temp      = 10000,
    dens      = 10000,
    viscosity = 10000,
    current32 = 100000,
    voltage32 = 100000,
};


int32_t pack_temp(double value);

inline constexpr int32_t c_pack_temp(double value) { return static_cast<int32_t>(round(value * static_cast<double>(ADC_SCALERS::temp))); }


double unpack_temp(int32_t value);

inline constexpr double c_unpack_temp(int32_t value) { return static_cast<double>(value) / static_cast<double>(ADC_SCALERS::temp); }


int32_t pack_dens(double value);

inline constexpr int32_t c_pack_dens(double value) { return static_cast<int32_t>(round(value * static_cast<double>(ADC_SCALERS::dens))); }


double unpack_dens(int32_t value);

inline constexpr double c_unpack_dens(int32_t value) { return static_cast<double>(value) / static_cast<double>(ADC_SCALERS::dens); }


int32_t pack_viscosity(double value);

inline constexpr int32_t c_pack_viscosity(double value, config::VVN_TYPE device_type)
{
    return static_cast<int32_t>(round(value * static_cast<double>(c_get_viscosity_multiplier_by_vvn_type(device_type))));
}


double unpack_viscosity(int32_t value);

inline constexpr double c_unpack_viscosity(int32_t value, config::VVN_TYPE device_type)
{
    return static_cast<double>(value) / static_cast<double>(c_get_viscosity_multiplier_by_vvn_type(device_type));
}


int32_t pack_current32(double value);

inline constexpr int32_t c_pack_current32(double value) { return static_cast<int32_t>(round(value * static_cast<double>(ADC_SCALERS::current32))); }


double unpack_current32(int32_t value);

inline constexpr double c_unpack_current32(int32_t value) { return static_cast<double>(value) / static_cast<double>(ADC_SCALERS::current32); }


int32_t pack_voltage32(double value);

inline constexpr int32_t c_pack_voltage32(double value) { return static_cast<int32_t>(round(value * static_cast<double>(ADC_SCALERS::voltage32))); }

double unpack_voltage32(int32_t value);


} // namespace adc


#endif /* MAPPING_VALUES_H_ */
