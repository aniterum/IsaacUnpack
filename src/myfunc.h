/*
 * myhunc.h
 *
 *  Created on: 14 нояб. 2014 г.
 *      Author: user
 */

#ifndef MYFUNC_H_
#define MYFUNC_H_

struct HEADER;

typedef struct HEADER {
	char header[7];				//Заголовок файла, должен быть ARCH000
	unsigned char compFlg;  	//Флаг упаковки, 1 - сжат, 0 - зашифрован
	uint tailOffset;     		//Сдвиг к хвосту файла, где есть данные о блоках
	unsigned short int recCnt;	//Количество записей в хвосте
};

struct TAIL_DATA;

typedef struct TAIL_DATA {
	uint chk;			//Контрольная сумма
	uint key;			//Ключ для дешифровки
	uint offset;		//Сдвиг относительно начала файла
	uint dataLen;		//Длина данных
	uint _un;			//Неизвестно
};

#define swap(a,b)\
a ^= b;\
b ^= a;\
a ^= b;

/**
 * Расшифровываем блок данных указанным ключом
 */
void decodeBlock(char *buf, size_t size, int _key){
	uint key = ((_key ^ 0xF9524287) | 0x1);
	uint s = 0;
	int i;
	for (i=0;  i < size; i += 4){
		*(uint*)(buf + i) ^= key;

		s = key & 0xf;

		if ((s -= 2) == 0)
		{
			// 1 2 3 4
			// 4 3 2 1
			swap(*(buf+i), *(buf + i+3));
			swap(*(buf+i+1), *(buf + i+2));

		} else if ((s -= 7) == 0) {
			// 1 2 3 4
			// 2 1 4 3
			swap(*(buf+i), *(buf + i+1));
			swap(*(buf+i+2), *(buf + i+3));

		} else if ((s -= 4) == 0) {
			// 1 2 3 4
			// 3 4 1 2

			swap(*(buf+i), *(buf + i+2));
			swap(*(buf+i+1), *(buf + i+3));

		}

		s = key;
		s <<= 8;
		s ^= key;
		key = s;
		key >>= 9;
		s ^= key;
		key = s;
		key <<= 0x17;
		key ^= s;


	}
}


#endif /* MYHUNC_H_ */
