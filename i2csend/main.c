﻿#include<avr/io.h>
#define F_cpu = 1000000 UL
#include <util/delay.h>
#include "stdSerialCOM.h"
#include <avr/interrupt.h>
#define M 100UL
//Gitテスト

unsigned long Bias = 0b00000000000000001;
//unsigned long Bias = 0b 0 . 0000 0000 0000 0001;


/* debug用送信データ */
char in_data[8];

/* SHT_MSB */
unsigned char St_MSB;

/* SHT_LSB */
unsigned char St_LSB;

/* 温度値計算用バッファ */
int buff;

/* 温度値（℃） */
unsigned char d_T;


/* St_MSB + St_LSBの16bitデータ */
unsigned long srh;


/* 進捗確認変数 */
static char progress = 0;


/* LCDアドレス送信確認フラグ */
char flg_lcd=0;

/* タスク過重回避 */
char ISR_cnt = 0;


/* main ISR同期 */
char main_flg=0;//メインを通過
/* 変数progressの変位 */
//----------------------
// 0 | lcd初期化前
//----------------------
// 1 | lcd初期化済み
//----------------------
// 2 | lcd表示完了
//----------------------
// 3 | SHT	コマンド送信前
//----------------------
// 4 | SHT　コマンド送信済み
//----------------------



/* 割込み TWI割込み */
ISR(TWI_vect)
{
	/***************************************************************************/
	/***************************************************************************/

	/* LCD初期化データ */
	char i2c_init[] = {0x38,0x39,0x14,0x70,0x52,0x6c,0x38,0x0c,0x01};

	/* LCD表示データ */
	char input_data[] = {0b10000000,(0x40|0b10000000),0b01000000,'0','0','0'};

	/* LCD表示データ＿カウント */
	char init = 0;



	/***************************************************************************/
	/***************************************************************************/
	convert_to_binary_number_serialconnect(progress,in_data,'p');
	convert_to_binary_number_serialconnect(TWSR,in_data,'s');
	/* main同期 */
	if(main_flg == 1)
	{
		/* ステータスレジスタ分岐 上位5bitの確認*/
		
		switch(TWSR & 0xF8)
		{
			/* 開始条件送信 */
			case 0x08:
			
				convert_to_binary_number_serialconnect(progress,in_data,'8');
				/* LCD初期化前 か　LCD初期化済 */
				if((progress == 0) || (progress == 1) )
				{
					/* SLA+W AQM0802のアドレス */
					TWDR = 0x7C;
					/* フラグ落とし 送信TWINT TWEN TWIE*/
					TWCR = 0b10000101;
					/* ACK待ち割込み立ち上がりでとる */
					flg_lcd = 1;
				}

				/* SHT通信アドレス送信前 */
				else if(progress == 3)
				{
					/* SLA+W AQM0802のアドレス W指定*/
					TWDR = 0b10001000;
					TWCR = 0b10000101;

					/* ACK待ち割込み立ち上がりでとる */
				}

				/* SHT通信アドレス送信後（センサ値受信前） */
				else if(progress == 4)
				{
					/* SLA+W AQM0802のアドレス r指定*/
					TWDR = 0b10001001;
					TWCR = 0b10000101;

					/* ACK待ち割込み立ち上がりでとる */
				}
				break;

			/* 開始条件送信 */
			case 0x18:

			/* LCD初期化前 */
			if(progress==0)
			{
				while(init != 9)
				{
					convert_to_binary_number_serialconnect(TWSR,in_data,'I');
					/* 初期化データの格納 */
					TWDR =  i2c_init[init];
					TWCR = 0b10000101;
					/* ACK応答待ち */
					while(!(TWCR & 0b10000000)	);

					/* 仕様書指定0x52後のディレイ */
					if(i2c_init[init] == 0x52)_delay_ms(200);
					init++;
				}
				init = 0;

				/* 次の操作へ　→lcd初期化済み */
				progress = 1;

				/* TWI通信終了　TWINT TWSTO TWEN TWIE  */
				TWCR = 0b10010101;
				/* STOが立つまで待つ */
				while(!(TWCR & (1<<TWSTO))	);

			}

			/* LCD初期化済　かつ　アドレス送信済み　 */
			else if(progress==1 && flg_lcd == 1)
			{
				convert_to_binary_number_serialconnect(progress,in_data,'S');
				/* 桁表示　計算用変数定義 */
				unsigned int b;

				/* 表示データ格納変数 */
				unsigned char LCD_3;
				unsigned char LCD_2;
				unsigned char LCD_1;
				/* テストデータ */
				
				
				LCD_3 = d_T/100;
				LCD_2 = (d_T - (LCD_3 * 100) ) / 10;
				LCD_1 = (d_T - (LCD_3 * 100) ) % 10;
				

				/* 表示データの出力 */
				//0b10000000,コマンドデータ
				//(0x40|0b10000000),座標DB7=1
				//0b01000000,表示データRS=1
				//'0',文字型0+ 整数型変数で表示
				//'0',文字型0+ 整数型変数で表示
				//'0',文字型0+ 整数型変数で表示

				while(init != 6)
				{
					//convert_to_binary_number_serialconnect(TWSR,in_data,'O');
					/* 3桁目 */
					if(init==3)
					{
						/* 3桁目表示 */
						TWDR = input_data[init]+(d_T/100);
						b = d_T/100;
						d_T = d_T - (b*100);
					}

					/* 2桁目 */
					else if(init==4)
					{
						TWDR = input_data[init]+(d_T/10);
					}

					/* 3桁目 */
					else if(init==5)
					{
						TWDR = input_data[init]+(d_T%10);
					}

					/* 上記以外の桁 */
					else
					{
						TWDR = input_data[init];
						
					}
					TWCR = 0b10000101;
					while(!(TWCR & 0b10000000)	);
					convert_to_binary_number_serialconnect(progress,in_data,TWDR);
					init++;
				}
				
				init = 0;

				/* 次の操作へ　→SHT	コマンド送信前 */
				progress = 3;
				ISR_cnt = 0;

				/* 通信終了 */
				TWCR = 0b10010101;
				/* STOが立つまで待つ */
				while(!(TWCR & (1<<TWSTO))	);
			}

			/* SHT	コマンド送信前 */
			else if(progress==3)
			{
				/* コマンドコード送信 */

				/* クロスストレッチ有効 */
				TWDR = 0x2c;
				TWCR = 0b10000101;
				while(!(TWCR & 0b10000000)	);

				/* 繰り返しレベル中 */
				TWDR = 0x0d;
				TWCR = 0b10000101;
				while(!(TWCR & 0b10000000)	);

				/* *************** */

				/* 次の操作へ　→コマンド送信済み*/
				ISR_cnt = 0;
				progress = 4;

				/* 通信終了 */
				TWCR = 0b10010101;
				/* STOが立つまで待つ */
				while(!(TWCR & (1<<TWSTO))	);
			}

			break;

			case 0x40:

				/* クロスストレッチ */

				/* 温度データ受信　MSB,LSB,CRC */

				/* MSB受信待ち（ACKなし） */
				while(!(TWCR & 0b10000000)	);
				convert_to_binary_number_serialconnect(TWDR,in_data,'$');
				/* ACK応答 */
				TWCR = 0b11000101;

				/* LSB受信待ち */
				while(!(TWCR & 0b10000000)	);
				convert_to_binary_number_serialconnect(TWDR,in_data,'+');
				St_MSB = TWDR;
				
				/* ACK応答 */
				TWCR = 0b11000101;

				/* CRC受信待ち（ACKなし） */
				while(!(TWCR & 0b10000000)	);
				convert_to_binary_number_serialconnect(TWDR,in_data,'-');
				St_LSB = TWDR;
				
				/* ACK応答 */
				TWCR = 0b11000101;

				/* ************************** */
				
				
				
				buff = (St_MSB*256+St_LSB);
				d_T = -45 + ((175 * (buff*Bias)) >> 16);
				convert_to_binary_number_serialconnect(d_T,in_data,'<');
				
				
				

				/* 湿度データ受信　MSB,LSB,CRC */

				/* MSB受信待ち（ACKなし） */
				while(!(TWCR & 0b10000000)	);
				/* ACK応答 */
				convert_to_binary_number_serialconnect(d_T,in_data,'=');
				TWCR = 0b11000101;

				/* LSB受信待ち */
				while(!(TWCR & 0b10000000)	);
				convert_to_binary_number_serialconnect(d_T,in_data,'/');
				/* ACK応答 */
				TWCR = 0b11000101;

				/* CRC受信待ち（ACKなし） */
				while(!(TWCR & 0b10000000)	);
				/* ACK応答 */
				TWCR = 0b10000101;//nack応答
				while(!(TWCR & 0b10000000)	);

				/* ************************** */

				/* 次の操作へ　→LCD初期化済み（LCD再描画） */
				

				/* 通信終了 */
				TWCR = 0b10010101;
				/* STOが立つまで待つ */
				while(!(TWCR & (1<<TWSTO))	);
				
				ISR_cnt = 0;
				progress = 1;

				break;

			default:
			break;

		}

	}

	main_flg = 0;

}




int main(void)
{
	
	DDRB  = 0b00000000;//
	PORTB = 0B00000010;//	PB1 PUR_ON
	
	TWBR  = 0b11111111;//SCL 周波数
	TWSR  = 0b00000000;//1.9kHz
	
	
	UCSR0A = 0b00000000;
	UCSR0B = 0b00011000;
	UCSR0C = 0b00000110;
	UBRR0 = 25;
	int buf1;
	char Data_name=' ';
	char flag = 0;
	
	sei();
	/* mainを通過 */
	main_flg = 1;

	/* 通信開始 TWINT TWSTA TWEN TWIE*/
	TWCR = 0b10100101;//フラグ下げ　開始　twi有効
	while(!(TWCR & 0b10000000)	);
	
	while (1){
		/* mainを通過 */
		main_flg = 1;
		
		if(progress == 1 && ISR_cnt == 0){
			convert_to_binary_number_serialconnect(progress,in_data,'P');
			TWCR = 0b10100101;//フラグ下げ　開始　twi有効
			ISR_cnt = 1;
			
		}
		
		else if(progress == 3 && ISR_cnt == 0){
			convert_to_binary_number_serialconnect(progress,in_data,'P');
			TWCR = 0b10100101;//フラグ下げ　開始　twi有効　MSB_0x2C LSB_0x0D
			ISR_cnt = 1;
			
		}
		
		else if(progress == 4 && ISR_cnt == 0){
			convert_to_binary_number_serialconnect(progress,in_data,'P');
			TWCR = 0b10100101;//フラグ下げ　開始 測定値受信
			ISR_cnt = 1;
			
			
		}

	}
	
	
}