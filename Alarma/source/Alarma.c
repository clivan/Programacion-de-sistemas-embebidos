#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include <string.h>
#include "fsl_tpm.h"
#include "keypad.h"
#include "LCD.h"

#define Red 0u
#define Yellow 4u
#define Green 3u
#define Buzzer 7u
#define Sense_1 5u
#define Sense_2 6u
#define Sense_3 10u
#define Sense_4 11u

typedef struct estado{
	uint16_t TIMER_MOD;
	uint8_t Alarma;
	uint8_t Rojo;
	uint8_t Amarillo;
	uint8_t Verde;
	uint8_t SensOn;
} state;

keypad k;
int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();

    tpm_config_t config;
    TPM_GetDefaultConfig(&config);
    config.prescale= kTPM_Prescale_Divide_128;
    TPM_Init(TPM0, &config);
    TPM_Init(TPM1, &config);
    
    keypad k;
    keypad_config config1;
    get_default_keypad_config(&config1);
    set_keypad_config(&k, &config1);
    char key=0, anterior;

    state FSM[8];
    FSM[0]=(state ){.Alarma=0u, .Rojo=0u, .Amarillo=0u, .Verde=1u, .SensOn=0u};
    FSM[1]=(state ){.Alarma=0u, .Rojo=0u, .Amarillo=0u, .Verde=1u, .SensOn=0u};
    FSM[2]=(state ){.Alarma=0u, .Rojo=0u, .Amarillo=0u, .Verde=0u, .SensOn=0u};
    FSM[3]=(state ){.Alarma=0u, .Rojo=0u, .Amarillo=1u, .Verde=0u, .SensOn=0u, .TIMER_MOD=4864u};
    FSM[4]=(state ){.Alarma=0u, .Rojo=0u, .Amarillo=1u, .Verde=0u, .SensOn=1u};
    FSM[5]=(state ){.Alarma=1u, .Rojo=1u, .Amarillo=0u, .Verde=0u, .SensOn=0u};
    FSM[6]=(state ){.Alarma=0u, .Rojo=0u, .Amarillo=1u, .Verde=0u, .SensOn=0u};
    FSM[7]=(state ){.Alarma=0u, .Rojo=0u, .Amarillo=1u, .Verde=0u, .SensOn=0u};

    uint8_t Sensor1=0;
    uint8_t Sensor2=0;
    uint8_t Sensor3=0;
    uint8_t Sensor4=0;
    uint32_t Mask= 1u<<8u;
	uint32_t Mask_Off = Mask;
    uint8_t Print_enable=0;
    uint8_t estado=0;
    uint8_t Timer_init=0;
	uint32_t TIMER_FLAG;
	uint8_t sens_state=0;
	uint8_t change_flag=0;
	uint8_t reg_state=0;
	uint8_t flag_wrong=0;
	int intentos=3;
	int Counter= 0;

	LCD_Activate(1u,1u,0u);

	char RPassword[12]={'A','C','A','B','A','D','A','1','3','7','9',0};
	char InPassword[12]={'0','0','0','0','0','0','0','0','0','0','0',0};
	char reset[12]={'0','0','0','0','0','0','0','0','0','0','0',0};

    while(1){
    	if(reg_state==0){

    		GPIO_WritePinOutput(GPIOC, Buzzer, FSM[estado].Alarma);
			GPIO_WritePinOutput(GPIOC, Red, FSM[estado].Rojo);
			GPIO_WritePinOutput(GPIOC, Yellow, FSM[estado].Amarillo);
			GPIO_WritePinOutput(GPIOC, Green, FSM[estado].Verde);
    	}else{
    		GPIO_WritePinOutput(GPIOC, Buzzer, FSM[5].Alarma);
			GPIO_WritePinOutput(GPIOC, Red, FSM[5].Rojo);
			GPIO_WritePinOutput(GPIOC, Yellow, FSM[estado].Amarillo);
			GPIO_WritePinOutput(GPIOC, Green, FSM[estado].Verde);
    	}


		TIMER_FLAG=TPM_GetStatusFlags(TPM0);

		sens_state=FSM[estado].SensOn;



		if(sens_state){
			if(!(GPIO_ReadPinInput(GPIOC, Sense_1))){
				Sensor1=1;

			}
			if(!(GPIO_ReadPinInput(GPIOC, Sense_2))){
				Sensor2=1;
			}
			if(!(GPIO_ReadPinInput(GPIOC, Sense_3))){
				Sensor3=1;
			}
			if(!(GPIO_ReadPinInput(GPIOC, Sense_4))){
				Sensor4=1;
			}
		}

		if(flag_wrong){
			LCD_Set(1u,1u,1u);
			LCD_Clear();
			Line(2);
			LCD_Write('I');
			LCD_Write('N');
			LCD_Write('C');
			LCD_Write('O');
			LCD_Write('R');
			LCD_Write('R');
			LCD_Write('E');
			LCD_Write('C');
			LCD_Write('T');
			LCD_Write('A');
			Line(1);
			LCD_Write('C');
			LCD_Write('L');
			LCD_Write('A');
			LCD_Write('V');
			LCD_Write('E');
			flag_wrong=0;
		}



		anterior=key;
		key=read_keypad(&k);

		switch(estado){
		case 0:											//Alarma desactivada, Verde On, Buzzer Off, Sensores Off, Pantalla <Alarma Desactivada>
			if(Print_enable == 0){
				Print_enable=1;
				LCD_Set(1u,1u,1u);
				LCD_Clear();
				Line(1);
				LCD_Write('A');
				LCD_Write('L');
				LCD_Write('A');
				LCD_Write('R');
				LCD_Write('M');
				LCD_Write('A');
				Line(2);
				LCD_Write('D');
				LCD_Write('E');
				LCD_Write('S');
				LCD_Write('A');
				LCD_Write('C');
				LCD_Write('T');
				LCD_Write('I');
				LCD_Write('V');
				LCD_Write('A');
				LCD_Write('D');
				LCD_Write('A');

			}

			if(key == 0 && anterior != 0){
				key=anterior;

				if(key == '*'){
					Print_enable=0;
					estado=1;
					DelayTPM();

				}
				if(key == '#'){					
					Print_enable=0;
					estado=6;
					DelayTPM();

				}
			key=0;
			}
		break;

		case 1:							      

			if(Print_enable==0){
				LCD_Set(1u,1u,1u);
				LCD_Clear();
				Line(1);
				LCD_Write('C');
				LCD_Write('L');
				LCD_Write('A');
				LCD_Write('V');
				LCD_Write('E');
				Print_enable=1;
			}




			if(key == 0 && anterior != 0){
				key=anterior;

				if(key != '#'){
					InPassword[Counter]=key;
					LCD_Write('*');
					printf("%c",key);
					Counter++;
					printf("\n%d\n",Counter);
					DelayTPM();

				}else if(key == '#'){  								//Si es presionado enter pero no se ha llenado los 11 campos
					if(Counter < 11){
						Print_enable=0;
						strcpy(InPassword,reset);
						Counter=0;
					}else if(Counter == 11){

						if(strcmp(InPassword,RPassword)==0){
							strcpy(RPassword,InPassword);
							strcpy(InPassword,reset);
							Print_enable=0;
							Counter=0;
							if(reg_state==1 && change_flag==0){
								estado=0;
								reg_state=0;
							}else if(reg_state==0 && change_flag==0){
								estado=2;
								reg_state=0;
							}else if(reg_state==0 && change_flag==1){
								Counter=0;
								estado=7;
								reg_state=0;
								change_flag=0;
								Print_enable=0;

							}
							DelayTPM();
							intentos=3;
							flag_wrong=0;

						}else{
							intentos=intentos-1;
							if(intentos==0){
								strcpy(InPassword,reset);
								estado=5; //Estado de alarma
							}else{

								flag_wrong=1;

								printf("Intentos restantes: %d \n",intentos);
								printf("Clave: ");
								strcpy(InPassword,reset);
								Counter=0;
							}
						}
					}else if(Counter > 11){
						printf("\nCaracteres sobrantes\n");
						printf("Clave: ");
						//strcpy(InPassword,reset);
						Print_enable=0;
						Counter=0;
					}

				}
				key=0;

			}




		break;

		case 2:							        
			LCD_Set(1u,1u,1u);
			LCD_Clear();
			Line(1);
			LCD_Write('A');
			LCD_Write('L');
			LCD_Write('A');
			LCD_Write('R');
			LCD_Write('M');
			LCD_Write('A');
			LCD_Write(' ');
			LCD_Write('A');
			LCD_Write('C');
			LCD_Write('T');
			LCD_Write('I');
			LCD_Write('V');
			LCD_Write('A');
			do{
				TPM_SetTimerPeriod(TPM1, 32u);
				TPM_StartTimer(TPM1, kTPM_SystemClock);
				GPIO_TogglePinsOutput(GPIOC, 1u<<0u);
				GPIO_TogglePinsOutput(GPIOC, 1u<<4u);
				GPIO_TogglePinsOutput(GPIOC, 1u<<3u);
				GPIO_TogglePinsOutput(GPIOC, 1u<<7u);
				while(!(TPM1->STATUS & Mask)){         //Wait
				}

				if(TPM1->STATUS & Mask){
					TPM1->STATUS &=Mask_Off;
					Counter=Counter+1;
					TPM_StopTimer(TPM1);
					TPM1->CNT=0;
				}

			}while(Counter<=6);
			Counter=0;
			estado=3;

		break;

		case 3:							        
			Print_enable=0;
			if(Timer_init==0){
					Timer_init=1;
					TPM_SetTimerPeriod(TPM0, FSM[3].TIMER_MOD);
					TPM_StartTimer(TPM0, kTPM_SystemClock);
				}

				if(TIMER_FLAG){
					TPM_ClearStatusFlags(TPM0, 1u<<8u);
					Timer_init= 0;
					TPM_StopTimer(TPM0);
					TPM0->CNT=0;
					estado= 4;
				}
		break;

		case 4:							        

			if(Print_enable==0){
				LCD_Set(1u,1u,1u);
				LCD_Clear();
				Line(1);
				LCD_Write('A');
				LCD_Write('L');
				LCD_Write('A');
				LCD_Write('R');
				LCD_Write('M');
				LCD_Write('A');
				LCD_Write(' ');
				LCD_Write('A');
				LCD_Write('C');
				LCD_Write('T');
				LCD_Write('I');
				LCD_Write('V');
				LCD_Write('A');
				Line(2);
				LCD_Write('S');
				LCD_Write('E');
				LCD_Write('N');
				LCD_Write('S');
				LCD_Write('O');
				LCD_Write('R');
				LCD_Write('E');
				LCD_Write('S');
				LCD_Write(' ');
				LCD_Write('A');
				LCD_Write('C');
				LCD_Write('T');
				LCD_Write('I');
				LCD_Write('V');
				LCD_Write('O');
				LCD_Write('S');
				Print_enable=1;
			}

			if(Sensor1 || Sensor2 || Sensor3 || Sensor4){
				Print_enable=0;
				estado=5;
			}


		break;

		case 5:							        

			reg_state=1;
			if(Print_enable==0){
				LCD_Set(1u,1u,1u);
				LCD_Clear();
				Line(1);
				LCD_Write('S');
				LCD_Write('E');
				LCD_Write('N');
				LCD_Write('S');
				LCD_Write('O');
				LCD_Write('R');
				LCD_Write(' ');
				printf("Sensor ");

				if(Sensor1==1){
					LCD_Write('1');
					LCD_Write(' ');
					printf("1");
					Sensor1=0;
				}
				if(Sensor2==1){
					LCD_Write('2');
					LCD_Write(' ');
					printf("2");
					Sensor2=0;
				}
				if(Sensor3==1){
					LCD_Write('3');
					LCD_Write(' ');
					Sensor3=0;
				}
				if(Sensor4==1){
					LCD_Write('4');
					LCD_Write(' ');
					Sensor4=0;
				}
				LCD_Write('A');
				LCD_Write('C');
				LCD_Write('T');
				LCD_Write('I');
				LCD_Write('V');
				LCD_Write('A');
				LCD_Write('D');
				LCD_Write('O');
				printf(" Activado");


				Line(2);
				LCD_Write('#');
				LCD_Write(' ');
				LCD_Write('I');
				LCD_Write('N');
				LCD_Write('T');
				LCD_Write('O');
				LCD_Write('D');
				LCD_Write('U');
				LCD_Write('C');
				LCD_Write('E');
				LCD_Write(' ');
				LCD_Write('C');
				LCD_Write('L');
				LCD_Write('A');
				LCD_Write('V');
				LCD_Write('E');
				Print_enable=1;
			}


			if(key=='#'){
				estado=1;
				Print_enable=0;
			}

		break;



		case 6:
			reg_state=0;
			if(Print_enable==0){
				LCD_Set(1u,1u,1u);
				LCD_Clear();
				Line(1);
				LCD_Write('C');
				LCD_Write('A');
				LCD_Write('M');
				LCD_Write('B');
				LCD_Write('I');
				LCD_Write('O');
				LCD_Write(' ');
				LCD_Write('C');
				LCD_Write('L');
				LCD_Write('A');
				LCD_Write('V');
				LCD_Write('E');
				Line(2);
				LCD_Write('S');
				LCD_Write('I');
				LCD_Write('=');
				LCD_Write('*');
				LCD_Write(' ');
				LCD_Write(' ');
				LCD_Write(' ');
				LCD_Write(' ');
				LCD_Write('N');
				LCD_Write('0');
				LCD_Write('=');
				LCD_Write('#');

				Print_enable=1;
			}

			if(key == 0 && anterior != 0){
				key=anterior;
				if(key=='*'){
					key=0;
					if(Print_enable==1){
						key=0;
						LCD_Set(1u,1u,1u);
						LCD_Clear();
						Line(1);
						LCD_Write('A');
						LCD_Write('C');
						LCD_Write('T');
						LCD_Write('U');
						LCD_Write('A');
						LCD_Write('L');
						Line(2);
						LCD_Write('C');
						LCD_Write('L');
						LCD_Write('A');
						LCD_Write('V');
						LCD_Write('E');
						change_flag=1;
						estado=1;

					}
				}else if(key=='#'){
					key=0;
					DelayTPM();
					estado=0;
					change_flag=0;
					Print_enable=0;

				}
			key=0;
			}
		break;

		case 7:
			if(Print_enable==0){
					LCD_Set(1u,1u,1u);
					LCD_Clear();
					Line(1);
					LCD_Write('N');
					LCD_Write('U');
					LCD_Write('E');
					LCD_Write('V');
					LCD_Write('A');
					Line(2);
					LCD_Write('C');
					LCD_Write('L');
					LCD_Write('A');
					LCD_Write('V');
					LCD_Write('E');
					change_flag=1;
					Print_enable=1;
			}

			if(key == 0 && anterior != 0){
				key=anterior;

				if(key != '#'){
					InPassword[Counter]=key;
					LCD_Write('*');
					printf("%c",key);
					Counter++;
					DelayTPM();
					printf("\n%d\n",Counter);

				}else if(key == '#'){
					strcpy(RPassword,InPassword);
					strcpy(InPassword,reset);
					Print_enable=0;
					key=0;
					DelayTPM();
					estado=0;
					Counter=0;
					change_flag=0;
				}

			key=0;
			}
		break;
		}

    }
}
