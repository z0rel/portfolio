/*
 * config_meas_common.h
 *
 * Created: 09.04.2022 5:07:59
 *  Author: artem
 */


#ifndef CONFIG_MEAS_COMMON_H_
#define CONFIG_MEAS_COMMON_H_


namespace config {

// | Обозначение     |  Диапазон измерения     | Номера градуировочных жидкостей и значения произведения  |
// | исполнения      |  вискозиметра           | кинематической вязкости на квадрат плотности, Па*с*кг/м3 |
// | вискозиметра    |  Па*с*кг/м3             |     1 |     2 |     3 |     4 |     5 |     6            |
// | --------------- | ----------------------- | ----- | ----- | ----- | ----- | ----- | ---------------- |
// | 5Д1.560.024-011 |     1..    20           |   1.1 |     4 |     8 |    12 |    16 |    19.1          |
// | 5Д1.560.024-021 |    10..   200           |    12 |    40 |    80 |   120 |   160 |   190            |
// | 5Д1.560.024-031 |   100..  2000           |   120 |   400 |   800 |  1200 |  1600 |  1900            |
// | 5Д1.560.024-041 |  1000.. 20000           |  1200 |  4000 |  8000 | 12000 | 16000 | 19000            |
// | 5Д1.560.024-051 | 10000..100000           | 12000 | 19000 | 40000 | 60000 | 80000 | 95500            |

// Внимание! Нумерация жидкостей в таблице 5.1 приведена только для поведения
// градуировки (см. далее по тексту). При проведении ПСИ и ГПИ пользоваться таблицей 7  раздела 4  5Д1.560.024 ТУ.

enum class VVN_TYPE : uint8_t {
    //           | Обозначение     |         Диапазон                  |  Диапазон           | Кi - Коэффициент          |
    //           | исполнения      |         измерений                 |  измерений          | преобразования            |
    //           | вискозиметра    |         динамической              |  с учётом           | измеряемой                |
    //           |                 |         вязкости                  |  плотности          | величины                  |
    //           |                 |         Па * с                    |  жидкости           | в выходной сигнал         |
    //           |                 |                                   |  Па * с * кг/м3     | тока: Па * с * кг/м3 / мА |
    //           | --------------- | --------------------------------- | ------------------- | ------------------------- |
    VVN8_011,  // | 5Д1.560.024-011 | от     1 * 10-3 до      20 * 10-3 | от     1 до      20 |    1.25                   |
    VVN8_021,  // | 5Д1.560.024-021 | от    10 * 10-3 до     200 * 10-3 | от    10 до     200 |   12.5                    |
    VVN8_031,  // | 5Д1.560.024-031 | от   100 * 10-3 до   2 000 * 10-3 | от   100 до   2 000 |  125                      |
    VVN8_041,  // | 5Д1.560.024-041 | от  1000 * 10-3 до  20 000 * 10-3 | от  1000 до  20 000 | 1250                      |
    VVN8_051  //  | 5Д1.560.024-041 | от 10000 * 10-3 до 100 000 * 10-3 | от 10000 до 100 000 | 6250                      |
};

enum class VVN_MULTIPLIER_VISCOSITY : uint32_t {
    VVN8_011_MUL = 100000,
    VVN8_021_MUL = 10000,
    VVN8_031_MUL = 1000,
    VVN8_041_MUL = 100,
    VVN8_051_MUL = 10
};


/// Получить позицию плавающей точки - десятичный множитель целочисленной вязкости по типу устройства
inline uint32_t get_viscosity_multiplier_by_vvn_type(VVN_TYPE device_type)
{
    switch (device_type) {
    case VVN_TYPE::VVN8_011:
        return static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_011_MUL);
    case VVN_TYPE::VVN8_021:
        return static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_021_MUL);
    case VVN_TYPE::VVN8_031:
        return static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_031_MUL);
    case VVN_TYPE::VVN8_041:
        return static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_041_MUL);
    case VVN_TYPE::VVN8_051:
        return static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_051_MUL);
    }
    return 1;
}


inline constexpr uint32_t c_get_viscosity_multiplier_by_vvn_type(VVN_TYPE device_type)
{
    return (
	  (device_type == VVN_TYPE::VVN8_011) 
	  ? static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_011_MUL)
      : (
      (device_type == VVN_TYPE::VVN8_021) 
	  ?  static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_021_MUL)
	  : (
      (device_type == VVN_TYPE::VVN8_031) 
	  ?  static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_031_MUL)
	  : (
      (device_type == VVN_TYPE::VVN8_041) 
	  ? static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_041_MUL)
	  : (
      (device_type == VVN_TYPE::VVN8_051) 
	  ? static_cast<uint32_t>(VVN_MULTIPLIER_VISCOSITY::VVN8_051_MUL)
	  : 1
      )))));
}

enum class VVN_MULTIPLIER_DENCITY : uint32_t { MULTIPLIER = 10000 };

enum class VVN_MULTIPLIER_TEMPERATURE : uint32_t { MULTIPLIER = 10000 };

enum class VVN_MULTIPLIER_CURRENT : uint32_t { MULTIPLIER = 1000 };

enum class VVN_MULTIPLIER_VOLTAGE : uint32_t { MULTIPLIER = 10000 };

enum class LoggingTimeout : uint8_t {
    SEC_5,
    SEC_10,
    SEC_20,
    SEC_30,
    SEC_40,
    SEC_50,
    SEC_60,
    MIN_1,
    MIN_2,
    MIN_3,
    MIN_4,
    MIN_5,
    MIN_6,
    MIN_7,
    MIN_8,
    MIN_9,
    MIN_10
};

/// Тип измеренной вязкости (Па * с * кг / м3) * VVN_MULTIPLIER_VISCOSITY
typedef double TypeViscosityMeasured;

/// Тип динамической вязкости (Па * с) * VVN_MULTIPLIER_VISCOSITY
typedef double TypeViscosityDynamics;

// Тип измерения температуры (в градусах Цельсия) * VVN_MULTIPLIER_TEMPERATURE
typedef double TypeTemperature;

// Тип измерения плотности * VVN_MULTIPLIER_DENCITY
typedef double TypeDensity;

// Тип юстировки напряжения * VVN_MULTIPLIER_VOLTAGE
typedef double TypeVoltage;

// Тип юстировки тока: 4.000 ma - 20.000 ma
typedef double TypeCurrentAlignment;


}  // namespace config


#endif /* CONFIG_MEAS_COMMON_H_ */
