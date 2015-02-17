/*
 ============================================================================
 Name        : main.c
 Author      : A
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "myfunc.h"

int main(int argc, char *argv[]) {
	char* ARCHIVE = "/home/user/Документы/Isaac_Unpack/packed/graphics.a";
	char* WRITE_TO = "/home/user/Документы/Isaac_Unpack/graphics/png%04i.png";
	char* DIR = "/home/user/Документы/Isaac_Unpack/graphics";
	char* _header = "ARCH000";

	if (argc != 3)
		puts("Usage:\nIsaacUnpack archive_file [save_to_directory] [files_mask]\nExample: IsaacUnpack /home/user/music.a /home/user/music music%02i.ogg");

	if( access( ARCHIVE, 0 ) == -1 ){
		printf("Archive file does not exists!");
		return(EXIT_SUCCESS);
	}

	if (mkdir(DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
		puts("Directory does not exists, create new one");

	puts("Start processing");
	puts(ARCHIVE);

	FILE *inFile;
	inFile = fopen(ARCHIVE, "rb");

	//Получаем данные о заголовке
	struct HEADER header;
	fread(&header, sizeof(header), 1, inFile);

	if (strcmp(header.header, _header) != 0) {
		printf("This is not ARCH000 file");
		return EXIT_SUCCESS;
	}

	//Если флаг сжатия 0 - перед нами шифрованный файл, начинаем расшифровку
	if (header.compFlg == 0) {

		printf("Header: %s\nCompressed flag: %i\nTail offset: %i\nRecords count: %i\n",
				header.header, header.compFlg, header.tailOffset, header.recCnt);

		//Загружаем данные из окончания файла
		struct TAIL_DATA tails[header.recCnt];
		fseek(inFile, header.tailOffset, SEEK_SET);
		size_t sz = sizeof(tails[0]);

		int i;
		for (i = 0; i < header.recCnt; i++)
			fread(&tails[i], sz, 1, inFile);

		size_t write_to_len = strlen(WRITE_TO);
		write_to_len = write_to_len + (255 - (write_to_len % 255));
		char outFileName[write_to_len];

		//Основной цикл расшифровки
		int t;
		for (t = 0; t < header.recCnt; t++) {

			char *buf;
			int memoryNeeded = tails[t].dataLen + (sizeof(uint) - (tails[t].dataLen % sizeof(uint)));
			buf = malloc(memoryNeeded);

			if (buf == NULL) {
				puts("Out of memory\n");
				fclose(inFile);
				return 0;
			}

			fseek(inFile, tails[t].offset, SEEK_SET);
			fread(buf, memoryNeeded, 1, inFile);

			decodeBlock(buf, memoryNeeded, tails[t].key);

			FILE *outFile;
			sprintf(outFileName, WRITE_TO, t);
			outFile = fopen(outFileName, "wb");
			fwrite(buf, tails[t].dataLen, 1, outFile);
			fclose(outFile);
			free(buf);
		}

		fclose(inFile);

	} else {
		puts("File is Compressed");
		printf("Header: %s\nCompressed flag: %i\nTail offset: %i\nRecords count: %i\n",
				header.header, header.compFlg, header.tailOffset, header.recCnt);
	}


	return EXIT_SUCCESS;
}

