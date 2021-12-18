#pragma once

#include <cstdint>

#define __DEBUG_PRINT__
#undef __DEBUG_PRINT__

/* общие утилы для пакета */
constexpr uint32_t POLY = 0x82f63b78;
static inline uint32_t crc32c(uint32_t crc, const unsigned char* buf, size_t len) {
	crc = ~crc;
	while (len--) {
		crc ^= *buf++;
		for (int k = 0; k < 8; k++)
			crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
	}
	return ~crc;
}
union uint32_to_char {
	/* union для конверта инта в массив байтов */
	uint32_t value;
	uint8_t chars[4];
};
static string strAlign(uint32_t value) {
	/* Общая функция для выравнивания числа без знака 32 бита */
	string buffer = "         0";
	for (uint32_t i = 9; i >= 0 && value; --i, value /= 10) {
		buffer[i] = value % 10 + 48;
	}
	return buffer;
}

static void prt(const uint8_t* data, uint32_t msize = 4) {
	/* выводит от указателя data четыре символа */
	for (int j = 0; j < 4 && j < msize; ++j)
		cout << data[j];
}

static void prntArr(uint8_t* data, uint32_t blocksCnt, uint32_t backSize) {
	/* вывод для каждого блока первых и последних 4-рёх символов */
	cout << "bound:    0         1         2         3         4     "
		<< "    5         6         7         8         9";
	for (uint32_t i = 0; i < blocksCnt; ++i) {
		if (i % 10 == 0)
			cout << endl << strAlign(i).substr(5, 5);
		cout << "  ";
		
		if (i == (blocksCnt - 1)) {
			prt(&data[i * data_size], backSize);
			if (backSize > 4)
				prt(&data[i * data_size + backSize - 4]);
			else
				prt(&data[i * data_size + backSize], backSize);
		}
		else {
			prt(&data[i * data_size], data_size);
			prt(&data[i * data_size + data_size - 4], data_size);
		}
	}
	cout << endl;

}

static void deepCopyPD(const uint8_t* from, uint8_t* to, const uint32_t size) {
	for (uint32_t i = 0; i < size; ++i, ++to, ++from)
		(*to) = (*from);
}
