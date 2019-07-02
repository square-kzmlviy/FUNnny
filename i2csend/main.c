#include<avr/io.h>
#define F_cpu = 1000000 UL
#include <util/delay.h>
#include "stdSerialCOM.h"
#include <avr/interrupt.h>
#define M 100UL

unsigned long Bias = 0b00000000000000001;
//unsigned long Bias = 0b 0 . 0000 0000 0000 0001;


/* debug�p���M�f�[�^ */
char in_data[8];

/* SHT_MSB */
unsigned char St_MSB;

/* SHT_LSB */
unsigned char St_LSB;

/* ���x�l�v�Z�p�o�b�t�@ */
int buff;

/* ���x�l�i���j */
unsigned char d_T;


/* St_MSB + St_LSB��16bit�f�[�^ */
unsigned long srh;


/* �i���m�F�ϐ� */
static char progress = 0;


/* LCD�A�h���X���M�m�F�t���O */
char flg_lcd=0;

/* �^�X�N�ߏd��� */
char ISR_cnt = 0;


/* main ISR���� */
char main_flg=0;//���C����ʉ�
/* �ϐ�progress�̕ψ� */
//----------------------
// 0 | lcd�������O
//----------------------
// 1 | lcd�������ς�
//----------------------
// 2 | lcd�\������
//----------------------
// 3 | SHT	�R�}���h���M�O
//----------------------
// 4 | SHT�@�R�}���h���M�ς�
//----------------------



/* ������ TWI������ */
ISR(TWI_vect)
{
	/***************************************************************************/
	/***************************************************************************/

	/* LCD�������f�[�^ */
	char i2c_init[] = {0x38,0x39,0x14,0x70,0x52,0x6c,0x38,0x0c,0x01};

	/* LCD�\���f�[�^ */
	char input_data[] = {0b10000000,(0x40|0b10000000),0b01000000,'0','0','0'};

	/* LCD�\���f�[�^�Q�J�E���g */
	char init = 0;



	/***************************************************************************/
	/***************************************************************************/
	convert_to_binary_number_serialconnect(progress,in_data,'p');
	convert_to_binary_number_serialconnect(TWSR,in_data,'s');
	/* main���� */
	if(main_flg == 1)
	{
		/* �X�e�[�^�X���W�X�^���� ���5bit�̊m�F*/
		
		switch(TWSR & 0xF8)
		{
			/* �J�n�������M */
			case 0x08:
			
				convert_to_binary_number_serialconnect(progress,in_data,'8');
				/* LCD�������O ���@LCD�������� */
				if((progress == 0) || (progress == 1) )
				{
					/* SLA+W AQM0802�̃A�h���X */
					TWDR = 0x7C;
					/* �t���O���Ƃ� ���MTWINT TWEN TWIE*/
					TWCR = 0b10000101;
					/* ACK�҂������ݗ����オ��łƂ� */
					flg_lcd = 1;
				}

				/* SHT�ʐM�A�h���X���M�O */
				else if(progress == 3)
				{
					/* SLA+W AQM0802�̃A�h���X W�w��*/
					TWDR = 0b10001000;
					TWCR = 0b10000101;

					/* ACK�҂������ݗ����オ��łƂ� */
				}

				/* SHT�ʐM�A�h���X���M��i�Z���T�l��M�O�j */
				else if(progress == 4)
				{
					/* SLA+W AQM0802�̃A�h���X r�w��*/
					TWDR = 0b10001001;
					TWCR = 0b10000101;

					/* ACK�҂������ݗ����オ��łƂ� */
				}
				break;

			/* �J�n�������M */
			case 0x18:

			/* LCD�������O */
			if(progress==0)
			{
				while(init != 9)
				{
					convert_to_binary_number_serialconnect(TWSR,in_data,'I');
					/* �������f�[�^�̊i�[ */
					TWDR =  i2c_init[init];
					TWCR = 0b10000101;
					/* ACK�����҂� */
					while(!(TWCR & 0b10000000)	);

					/* �d�l���w��0x52��̃f�B���C */
					if(i2c_init[init] == 0x52)_delay_ms(200);
					init++;
				}
				init = 0;

				/* ���̑���ց@��lcd�������ς� */
				progress = 1;

				/* TWI�ʐM�I���@TWINT TWSTO TWEN TWIE  */
				TWCR = 0b10010101;
				/* STO�����܂ő҂� */
				while(!(TWCR & (1<<TWSTO))	);

			}

			/* LCD�������ρ@���@�A�h���X���M�ς݁@ */
			else if(progress==1 && flg_lcd == 1)
			{
				convert_to_binary_number_serialconnect(progress,in_data,'S');
				/* ���\���@�v�Z�p�ϐ���` */
				unsigned int b;

				/* �\���f�[�^�i�[�ϐ� */
				unsigned char LCD_3;
				unsigned char LCD_2;
				unsigned char LCD_1;
				/* �e�X�g�f�[�^ */
				
				
				LCD_3 = d_T/100;
				LCD_2 = (d_T - (LCD_3 * 100) ) / 10;
				LCD_1 = (d_T - (LCD_3 * 100) ) % 10;
				

				/* �\���f�[�^�̏o�� */
				//0b10000000,�R�}���h�f�[�^
				//(0x40|0b10000000),���WDB7=1
				//0b01000000,�\���f�[�^RS=1
				//'0',�����^0+ �����^�ϐ��ŕ\��
				//'0',�����^0+ �����^�ϐ��ŕ\��
				//'0',�����^0+ �����^�ϐ��ŕ\��

				while(init != 6)
				{
					//convert_to_binary_number_serialconnect(TWSR,in_data,'O');
					/* 3���� */
					if(init==3)
					{
						/* 3���ڕ\�� */
						TWDR = input_data[init]+(d_T/100);
						b = d_T/100;
						d_T = d_T - (b*100);
					}

					/* 2���� */
					else if(init==4)
					{
						TWDR = input_data[init]+(d_T/10);
					}

					/* 3���� */
					else if(init==5)
					{
						TWDR = input_data[init]+(d_T%10);
					}

					/* ��L�ȊO�̌� */
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

				/* ���̑���ց@��SHT	�R�}���h���M�O */
				progress = 3;
				ISR_cnt = 0;

				/* �ʐM�I�� */
				TWCR = 0b10010101;
				/* STO�����܂ő҂� */
				while(!(TWCR & (1<<TWSTO))	);
			}

			/* SHT	�R�}���h���M�O */
			else if(progress==3)
			{
				/* �R�}���h�R�[�h���M */

				/* �N���X�X�g���b�`�L�� */
				TWDR = 0x2c;
				TWCR = 0b10000101;
				while(!(TWCR & 0b10000000)	);

				/* �J��Ԃ����x���� */
				TWDR = 0x0d;
				TWCR = 0b10000101;
				while(!(TWCR & 0b10000000)	);

				/* *************** */

				/* ���̑���ց@���R�}���h���M�ς�*/
				ISR_cnt = 0;
				progress = 4;

				/* �ʐM�I�� */
				TWCR = 0b10010101;
				/* STO�����܂ő҂� */
				while(!(TWCR & (1<<TWSTO))	);
			}

			break;

			case 0x40:

				/* �N���X�X�g���b�` */

				/* ���x�f�[�^��M�@MSB,LSB,CRC */

				/* MSB��M�҂��iACK�Ȃ��j */
				while(!(TWCR & 0b10000000)	);
				convert_to_binary_number_serialconnect(TWDR,in_data,'$');
				/* ACK���� */
				TWCR = 0b11000101;

				/* LSB��M�҂� */
				while(!(TWCR & 0b10000000)	);
				convert_to_binary_number_serialconnect(TWDR,in_data,'+');
				St_MSB = TWDR;
				
				/* ACK���� */
				TWCR = 0b11000101;

				/* CRC��M�҂��iACK�Ȃ��j */
				while(!(TWCR & 0b10000000)	);
				convert_to_binary_number_serialconnect(TWDR,in_data,'-');
				St_LSB = TWDR;
				
				/* ACK���� */
				TWCR = 0b11000101;

				/* ************************** */
				
				
				
				buff = (St_MSB*256+St_LSB);
				d_T = -45 + ((175 * (buff*Bias)) >> 16);
				convert_to_binary_number_serialconnect(d_T,in_data,'<');
				
				
				

				/* ���x�f�[�^��M�@MSB,LSB,CRC */

				/* MSB��M�҂��iACK�Ȃ��j */
				while(!(TWCR & 0b10000000)	);
				/* ACK���� */
				convert_to_binary_number_serialconnect(d_T,in_data,'=');
				TWCR = 0b11000101;

				/* LSB��M�҂� */
				while(!(TWCR & 0b10000000)	);
				convert_to_binary_number_serialconnect(d_T,in_data,'/');
				/* ACK���� */
				TWCR = 0b11000101;

				/* CRC��M�҂��iACK�Ȃ��j */
				while(!(TWCR & 0b10000000)	);
				/* ACK���� */
				TWCR = 0b10000101;//nack����
				while(!(TWCR & 0b10000000)	);

				/* ************************** */

				/* ���̑���ց@��LCD�������ς݁iLCD�ĕ`��j */
				

				/* �ʐM�I�� */
				TWCR = 0b10010101;
				/* STO�����܂ő҂� */
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
	
	TWBR  = 0b11111111;//SCL ���g��
	TWSR  = 0b00000000;//1.9kHz
	
	
	UCSR0A = 0b00000000;
	UCSR0B = 0b00011000;
	UCSR0C = 0b00000110;
	UBRR0 = 25;
	int buf1;
	char Data_name=' ';
	char flag = 0;
	
	sei();
	/* main��ʉ� */
	main_flg = 1;

	/* �ʐM�J�n TWINT TWSTA TWEN TWIE*/
	TWCR = 0b10100101;//�t���O�����@�J�n�@twi�L��
	while(!(TWCR & 0b10000000)	);
	
	while (1){
		/* main��ʉ� */
		main_flg = 1;
		
		if(progress == 1 && ISR_cnt == 0){
			convert_to_binary_number_serialconnect(progress,in_data,'P');
			TWCR = 0b10100101;//�t���O�����@�J�n�@twi�L��
			ISR_cnt = 1;
			
		}
		
		else if(progress == 3 && ISR_cnt == 0){
			convert_to_binary_number_serialconnect(progress,in_data,'P');
			TWCR = 0b10100101;//�t���O�����@�J�n�@twi�L���@MSB_0x2C LSB_0x0D
			ISR_cnt = 1;
			
		}
		
		else if(progress == 4 && ISR_cnt == 0){
			convert_to_binary_number_serialconnect(progress,in_data,'P');
			TWCR = 0b10100101;//�t���O�����@�J�n ����l��M
			ISR_cnt = 1;
			
			
		}

	}
	
	
}