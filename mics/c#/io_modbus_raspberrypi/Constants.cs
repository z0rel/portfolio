using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IO1516.src
{
    class Constants
    {
        /// <summary>
        ///  Пин SPI обновления выходных данных сдвигового регистра
        /// </summary>
        public const int PIN_UPDATE_SHIFT_REGISTER_OUT_VALUES = 27;
        /// <summary>
        ///  Бит - Нужно обновить в УЧПУ регистр состояния входных модулей
        /// </summary>
        public const int BIT_OF_NEED_UPDATE_STATE_INPUT_REGISTER = 1 << 17;
        public const int BIT_OF_NEED_UPDATE_STATE_INPUT_REGISTER_INV = ~BIT_OF_NEED_UPDATE_STATE_INPUT_REGISTER;
        /// <summary>
        ///  Маска - нужно обновить в УЧПУ данные с входного модуля
        /// </summary>
        public const int BIT_OF_UPDATE_MASK = 1 << 17;
        /// <summary>
        ///  Маска - нужно обновить в УЧПУ данные с входного модуля - инвертированная
        /// </summary>
        public const int BIT_OF_UPDATE_MASK_INV = ~BIT_OF_UPDATE_MASK;

        /// <summary>
        ///  Номера пинов GPIO, подключенных к входам
        /// </summary>
        public static int[] OUTPUT_PIN_NUMBERS = {17, 18, 27, 22, 23, 24, 8, 5, 6, 12, 13, 19, 16, 26, 11, 21};
    }
}
