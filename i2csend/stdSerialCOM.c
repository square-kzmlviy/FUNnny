/*
 * stdSerialCOM.c
 *
 * Created: 2019/06/09 1:19:47
 * Author : nhs80412
 */ 

#include <avr/io.h>
//#include "stdSerialCOM.h"
/***1.data 2.beffer 3.uniquechar ************************/
void convert_to_binary_number_serialconnect(unsigned char data, char *math,unsigned char data_name)
{
	char cnt;
	char ans;
	char buf[9] = "00000000";
	char j;
	char bin[3] = "0b";
	ans = 0;
	cnt = 0;
	for (j = 0; j <= 7; j++)
	{
		*(math+j) = 0;
	}

	while (data != 0)//ÅŒã‚Ì‰‰ŽZ‚ª‚Oor1
	{
		*(math + cnt) = data % 2;//—]‚è‚ðo‚·
		//printf("bit[%d] %% 2=%d\n", cnt,*(math + cnt));
		data = data / 2;//ŽÀÛ‚ÉŠ„‚é
		//printf("data / 2=%d\n", data);
		cnt++;//address‰ÁŽZ

	}
	//*(math + cnt) = ans;
		while (!(UCSR0A & 0b00100000));
		UDR0 = data_name;
	for (j = 0; j <= 7; j++)
	{
		*(buf + j) = *( math + (7 - j) ) + '0';
		//printf("buf[%d] = math[%d]\n", j, 7 - j);
	}
	for (j = 0; j <= 1; j++) {
		while (!(UCSR0A & 0b00100000));
		UDR0 = *(bin + j);
		//printf("%c", *(bin + j));
	}
	for (j = 0; j <= 7; j++)
	{
		while (!(UCSR0A & 0b00100000));
		UDR0 = *(buf + j);
		//printf("%c", *(buf + j) );
	}
	
	while(!(UCSR0A & 0b00100000));
	UDR0 = '\n';
	while(!(UCSR0A & 0b00100000));
	UDR0 = '\r';

}


