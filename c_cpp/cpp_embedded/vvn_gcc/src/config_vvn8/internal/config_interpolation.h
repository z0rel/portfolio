/*
 * config_interpolation.h
 *
 * Created: 09.04.2022 5:18:41
 *  Author: artem
 */ 


#ifndef CONFIG_INTERPOLATION_H_
#define CONFIG_INTERPOLATION_H_


#include "config_meas_common.h"


namespace config {


enum class InterpolationPointsSize : uint8_t { value = 6 };


class InterpolationConfig {
  public:
    struct InterpItem {
        /// Значения отфильтрованного значения напряжения АЦП
        int32_t adc_voltage_value;
        /// Значения кодов кинематической вязкости
        int32_t kinematic_viscosity_value;
    };

    /// Значения функции интерполяции {*1000, *10000}
    InterpItem interpolationPoints[static_cast<uint8_t>(InterpolationPointsSize::value)]{
        {adc::c_pack_voltage32(0.65), adc::c_pack_viscosity(19.7, VVN_TYPE::VVN8_011) },
	    {adc::c_pack_voltage32(0.95), adc::c_pack_viscosity(40.89, VVN_TYPE::VVN8_011)},
		{adc::c_pack_voltage32(1.392), adc::c_pack_viscosity(80.72, VVN_TYPE::VVN8_011)},
        {adc::c_pack_voltage32(1.725), adc::c_pack_viscosity(118.75, VVN_TYPE::VVN8_011)},
		{adc::c_pack_voltage32(2.003), adc::c_pack_viscosity(159.39, VVN_TYPE::VVN8_011)},
		{adc::c_pack_voltage32(2.253), adc::c_pack_viscosity(191.22, VVN_TYPE::VVN8_011)}};

    void load();
    void save();
};


}



#endif /* CONFIG_INTERPOLATION_H_ */