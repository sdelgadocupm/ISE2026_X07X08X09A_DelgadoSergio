#include "Driver_SPI.h"
#include "Arial12x12.h" 
#include "lcd.h"
 
//SPI
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
static TIM_HandleTypeDef htim7;

//LCD
static unsigned char buffer[512];
static uint16_t positionL1=0;
static uint16_t positionL2=256;

void LCD_reset(void){
	
	/* Configuracion driver SPI */
	
	SPIdrv->Initialize(NULL);
	SPIdrv->PowerControl(ARM_POWER_FULL);
	SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8),20000000);
	
	/* Configuracion pines GPIO */
	
	GPIO_InitTypeDef GPIO_InitStructA;
	GPIO_InitTypeDef GPIO_InitStructD;
	GPIO_InitTypeDef GPIO_InitStructF;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitStructA.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructA.Pull = GPIO_PULLUP;
	GPIO_InitStructA.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructA.Pin = GPIO_PIN_6;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructA);
		
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	GPIO_InitStructD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructD.Pull = GPIO_PULLUP;
	GPIO_InitStructD.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructD.Pin = GPIO_PIN_14;
	
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructD);
	
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
	GPIO_InitStructF.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructF.Pull = GPIO_PULLUP;
	GPIO_InitStructF.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructF.Pin = GPIO_PIN_13;
	
	HAL_GPIO_Init(GPIOF, &GPIO_InitStructF);
	
	/* Inicializacion RESET, CS, A0 */
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
	
	/* Generacion señal de reset */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	delay(1);	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	delay(1000);
}


void delay(uint32_t n_microsegundos){
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 83;
	htim7.Init.Period = (n_microsegundos)-1;
	
	__HAL_RCC_TIM7_CLK_ENABLE();
	
	HAL_TIM_Base_Init(&htim7);	

	HAL_TIM_Base_Start(&htim7);

	while(__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE) == 0);
	
	HAL_TIM_Base_Stop(&htim7);
	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
	
	__HAL_TIM_SET_COUNTER(&htim7, 0);
}


void LCD_wr_data(unsigned char data){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	ARM_SPI_STATUS stat;
	SPIdrv->Send(&data, sizeof(data));
	
	do{
		stat = SPIdrv->GetStatus();
	}while(stat.busy);	
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

}

void LCD_wr_cmd(unsigned char cmd){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
	ARM_SPI_STATUS stat;
	SPIdrv->Send(&cmd, sizeof(cmd));
	
	do{
		stat = SPIdrv->GetStatus();
	}while(stat.busy);	
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	
	
}

void LCD_Init(void){
   LCD_wr_cmd(0xAE);
   LCD_wr_cmd(0xA2);

   LCD_wr_cmd(0xA0);
   LCD_wr_cmd(0xC8);
   LCD_wr_cmd(0x22);
   LCD_wr_cmd(0x2F);
   LCD_wr_cmd(0x40);
   LCD_wr_cmd(0xAF);
   LCD_wr_cmd(0x81);
   LCD_wr_cmd(0x17);
   LCD_wr_cmd(0xA4);
   LCD_wr_cmd(0xA6);
}

void LCD_update(void){
	int i;
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB0); // Página 0
	for(i=0;i<128;i++){
		LCD_wr_data(buffer[i]);
	}
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB1); // Página 1
	for(i=128;i<256;i++){
		LCD_wr_data(buffer[i]);
	}
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2); //Página 2
	for(i=256;i<384;i++){
		LCD_wr_data(buffer[i]);
	}
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3); // Pagina 3
	for(i=384;i<512;i++){
		LCD_wr_data(buffer[i]);
	}
}

void symbolToLocalBuffer_L1(uint8_t symbol){
	
	uint8_t i, value1, value2;
	uint16_t offset=0;
	
	offset=25*(symbol - ' ');
	
	for(i=0;i<12;i++){
		value1=Arial12x12[offset+i*2+1];
		value2=Arial12x12[offset+i*2+2];
		
		buffer[i+positionL1]=value1;
		buffer[i+128+positionL1]=value2;
		
	}	
	
	positionL1=positionL1+Arial12x12[offset];
}

void symbolToLocalBuffer_L2(uint8_t symbol){
	
	uint8_t i, value1, value2;
	uint16_t offset=0;
	
	offset=25*(symbol - ' ');
	
	for(i=0;i<12;i++){
		value1=Arial12x12[offset+i*2+1];
		value2=Arial12x12[offset+i*2+2];
		
		buffer[i+positionL2]=value1;
		buffer[i+128+positionL2]=value2;
		
	}	
	
	positionL2=positionL2+Arial12x12[offset];
}

void printLCD(char text[], uint8_t line){
	int i=0;
	positionL1=0;
	positionL2=256;
	
	if(line == 1){
		LCD_clean1();
	}else if(line == 2){
		LCD_clean2();
	}

	do{	
		if(line == 1){
			symbolToLocalBuffer_L1(text[i]);
		}else if(line == 2){
			symbolToLocalBuffer_L2(text[i]);
		}
		i++;
	}while(text[i]!=NULL);

	LCD_update();
}


void LCD_clean1(void){
	for(int i=0;i<256;i++){
		buffer[i]=0x00;
	}
	LCD_update();
}

void LCD_clean2(void){
	for(int i=256;i<512;i++){
		buffer[i]=0x00;
	}
	LCD_update();
}

void LCD_clean(void){
	for(int i=0;i<512;i++){
		buffer[i]=0x00;
	}
	LCD_update();
}


