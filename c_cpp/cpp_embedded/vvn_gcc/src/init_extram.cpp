/*
 * init_extram.cpp
 *
 * Created: 02.03.2022 3:41:36
 *  Author: artem
 */ 

#include "../arduino/pins_arduino.h"
#include "../freertos/portable.h"


const HeapRegion_t xHeapRegions[] = {
    {reinterpret_cast<uint8_t *>(0x1100), 1 + 0x7FF7 - 0x1100}, {reinterpret_cast<uint8_t *>(0x8000), 1 + 0x90FF - 0x8000}, {nullptr, 0} /* Завершает массив. */
};


void init_extram() {
    MCUCR &= ~_BV(SRE); // External SRAM/XMEM Disable

	// Подтяжка (PULL-UP) портов AD7:0  может быть активирована  быть активированы, записью единицы в соответствующий порту регистр. 
	// Для снижения энергопотребления в спящем режиме рекомендуется отключить подтягивание к единице, записав регистр порта в ноль перед переходом в спящий режим. 
	// Интерфейс XMEM также поддерживает шину на линиях AD7:0. 
	// Хранитель шины можно отключить и включить программно, как описано в разделе
	// «Регистр управления внешней памятью B — XMCRB» на стр. 32. Когда он
	// включён, хранитель шины будет обеспечивать заданный логический уровень
	// (ноль или единицу) на шине AD7:0, в то время как эти линии в противном
	// случае были бы переведены в третье состояние состояния интерфейсом XMEM.
	//
    // Поскольку внешняя память отображается после внутренней памяти, как
    // показано на рис. 11, внешняя память не адресуется при адресации первых
    // 4352 байт пространства данных. Может показаться, что первые 4352 байта
    // внешней памяти недоступны (адреса внешней памяти от 0x0000 до 0x10FF).
    //
    // Однако при подключении внешней памяти меньше 64 Кбайт, например 
	// 32 Кбайт, к этим ячейкам легко получить доступ, просто обратившись с
	// адреса от 0x8000 до 0x90FF. Поскольку бит A15 адреса внешней памяти не  
    // подключен к внешней памяти, адреса от 0x8000 до 0x90FF будут
    // отображаться как адреса от 0x0000 до 0x10FF для внешней памяти.
    // Адресация выше адреса 0x90FF не рекомендуется, так как это будет
    // адресовать ячейку внешней памяти, к которой уже обращаются по другому
    // (более низкому) адресу. 



	
	// Для прикладного программного обеспечения внешняя
    // память объемом 32 Кбайт будет отображаться как одно линейное адресное  
    // пространство размером 32 Кбайт от 0x1100 до 0x90FF. Это показано на рис.
    // 17. Конфигурация памяти B относится к режиму совместимости с ATmega103,
    // конфигурация A — к несовместимому режиму.

    // TODO: extmem linekr  gcc prog 0x1100 до 0x90FF (32768b) -> physical 0x0 -> 0x7FFF
	// реальный маппинг: program 0x8000 до 0x90FF -> physical 0x0000 до 0x10FF
	//                   program 0x1100 до 0x7FFF -> physical 0x1100 до 0x7FFF
	// TODO: 7FF8-7FFF - нелинкуемые данные, часы. => 0x8000 до 0x90FF -> нужно будет размещать вручную в отдельной секции


    // Когда устройство установлено в режиме совместимости с ATmega103,
    // внутреннее адресное пространство составляет 4096 байт. Это означает, что
    // к первым 4096 байтам внешней памяти можно получить доступ по адресам от
    // 0x8000 до 0x8FFF. Для прикладного программного обеспечения внешняя
    // память объемом 32 Кбайт будет отображаться как одно линейное адресное
    // пространство размером 32 Кбайт от 0x1000 до 0x8FFF."
    // Включить хранитель шины и подтяжку портов

    // SRW10: Wait-state Select Bit
    MCUCR &= ~_BV(SRW10); //In ATmega103 compatibility mode, writing SRW10 to one enables the wait-state and one extra cycle is added during read/write strobe as shown in Figure 14. 

    // SRL2, SRL1, SRL0: Wait-state Sector Limit
	// 0 0 0 Lower sector = N/A
    // Upper sector = 0x1100 - 0xFFFF
    // TODO: extmem linekr  gcc prog 0x1100 до 0x90FF (32768b) -> physical 0x0 -> 0x7FFF
	// реальный маппинг: program 0x8000 до 0x90FF -> physical 0x0000 до 0x10FF
	//                   program 0x1100 до 0x7FFF -> physical 0x1100 до 0x7FFF
    XMCRA &= ~(_BV(SRL2) | _BV(SRL1) | _BV(SRL0));


    // Bit 3..2 – SRW01, SRW00: Wait-state Select Bits for Lower Sector
    // The SRW01 and SRW00 bits control the number of wait-states for the lower sector of the external memory address space, see Table 4.
	// SRW11 and SRW10 bits control the number of wait-states for the upper sector of the external memory address space, see Table 4.
	// When the entire SRAM address space is configured as one sector (SRL2 = 0 SRL1 = 0 SRL0 = 0), the wait-states are configured by the SRW11 and SRW10 bits.

	// Длительность одного такта процессора равна:
	// (1000 * 1000 * 1000) * 1 / (73728*100) = 138 nS
	// Согласно графикам, подходит вариант "Wait one cycle during read/write strobe" SRW11 = 0 SRW10 = 1
    XMCRA &= ~_BV(SRW11);
    XMCRA |= _BV(SRW10);

    // Сброс настроек для нижнего банка внешней памяти, который по конфигурации - не используется
    XMCRA &= ~(_BV(SRW01) | _BV(SRW00));

    // XMBK: External Memory Bus-keeper Enable
	// Writing XMBK to one enables the bus keeper on the AD7:0 lines. When the
	// bus keeper is enabled, it will ensure a defined logic level (zero or
	// one) on AD7:0 when they would otherwise be tri-stated. Writing XMBK to
	// zero disables the bus keeper. XMBK is not qualified with SRE, so even if
	// the XMEM interface is disabled, the bus keepers are still activated as
	// long as XMBK is one.
    XMCRB |=  _BV(XMBK);

    // Release PC7: XMM2 = 0 XMM1 = 0 XMM0 = 1
    XMCRB &= ~(_BV(XMM2) | _BV(XMM1));
    XMCRB |= _BV(XMM0);

    MCUCR |= _BV(SRE); // External SRAM/XMEM Enable

    // TODO: настроить PC7 в OUTPUT 0 для работы Memory Chip Enable

    // Настроить регионы кучи динамической памяти
    vPortDefineHeapRegions(xHeapRegions);
}



extern const HeapRegion_t xHeapRegions[];


bool extram_check_part(uint8_t *lower_limit, uint8_t *upper_limit)
{
   unsigned char temp;
   uint8_t *i;
   __asm__ __volatile__("cli");
   for(i = lower_limit; i < upper_limit; i++)
   {
       temp = *(unsigned char*)i;
       *(unsigned char*)i = 0x00;
       if (*(unsigned char*)i != 0x00) {
          break;
	   }

       *(unsigned char*)i = 0xFF;
       if (*(unsigned char*)i != 0xFF) {
          break;
	   }

       *(unsigned char*)i = temp;
       if (*(unsigned char*)i != temp) {
          break;
	   }
   }
   __asm__ __volatile__("sei");
   return (i == upper_limit) ? true : false;
}


bool extram_check() {
	bool ok = true;
	ok = ok && extram_check_part(xHeapRegions[0].pucStartAddress, xHeapRegions[0].pucStartAddress + xHeapRegions[0].xSizeInBytes);
	ok = ok && extram_check_part(xHeapRegions[1].pucStartAddress, xHeapRegions[1].pucStartAddress + xHeapRegions[0].xSizeInBytes);
	return ok;
}
