/*
 * mapping_values.cpp
 *
 * Created: 29.03.2022 6:28:53
 *  Author: artem
 */

#include "mapping_values.h"

#include "internal/config_meas_common.h"


namespace adc {


int32_t pack_dens(double value) { return static_cast<int32_t>(round(value * static_cast<double>(ADC_SCALERS::dens))); }


double unpack_dens(int32_t value)
{
    double val = static_cast<double>(value);
    val /= static_cast<double>(ADC_SCALERS::dens);
    return val;
}


int32_t pack_temp(double value) { return static_cast<int32_t>(round(value * static_cast<double>(ADC_SCALERS::temp))); }


double unpack_temp(int32_t value)
{
    double val = static_cast<double>(value);
    val /= static_cast<double>(ADC_SCALERS::temp);
    return val;
}


int32_t pack_viscosity(double value, config::VVN_TYPE device_type)
{
    return static_cast<int32_t>(round(value * static_cast<double>(config::get_viscosity_multiplier_by_vvn_type(device_type))));
}


double unpack_viscosity(int32_t value, config::VVN_TYPE device_type)
{
    double val = static_cast<double>(value);
    val /= static_cast<double>(config::get_viscosity_multiplier_by_vvn_type(device_type));
    return val;
}


int32_t pack_current32(double value)
{
    return static_cast<int32_t>(round(value * static_cast<double>(ADC_SCALERS::current32)));
}


double unpack_current32(int32_t value)
{
    double val = static_cast<double>(value);
    val /= static_cast<double>(ADC_SCALERS::current32);
    return val;
}


int32_t pack_voltage32(double value)
{
    return static_cast<int32_t>(round(value * static_cast<double>(ADC_SCALERS::voltage32)));
}

double unpack_voltage32(int32_t value)
{
    double val = static_cast<double>(value);
    val /= static_cast<double>(ADC_SCALERS::voltage32);
    return val;
}


}  // namespace adc
