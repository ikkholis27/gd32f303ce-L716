/*
********************************************************************
**  Copyright (C) 2022 Fibocom. All Rights Reserved.
**  Filename    : main.c
**  Author      : Frank.Zhou
**  Date        : 2022.06.07
**  Description : MCU test
********************************************************************
*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "gd32f30x.h"
#include "usart.h"

#define DEBUGMODE 0
#define MCU_SOFT_VERSION  "MCU_V1.0"

#define LED_GPIO_PIN (GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)

static char uart0_rxbuf[2000], uart0_rxbuf1[2000] = {0};
static volatile int rx_cnt = 0;
static volatile int len_rxmeter = 0;
static volatile char uart0_rec_flag = 0;
static volatile char sim_flag = 0; // AT=0; READY=1; CSQ=2; MIPCALL M2MPNAMR=3; MIPCALL?=4; MIPOPEN=5

static char meter_buf[4000] = {0};
static volatile int len_meter_buf = 0;
static char uart1_temp[4000], uart1_tx[2000] = {0};

// Uart1 Vars
int uart1_index = 0, socket_id=0, stx=0, etx=0;
static char uart1_rxbuf[4000] = {0};
static volatile char rx1_flag = 0; // AT=0; READY=1; MIPCALL=2; MIPOPEN=3; - MIP

void delay(uint32_t t)
{
    int i,j;
    for(i=0; i<t; i++) {
        for(j=0; j<=500000; j++){
					__NOP();
				}
    }
}

int uart1_send_buffer(char *str, int len)
{
    char *p = str;
	int x=0;
    while(x<len) {
        usart_data_transmit(USART1, *p);
        p++;
			x++;
    }
    return 0;
}

int uart0_send_string(char *str)
{
    char *p = str;

    while(*p) {
        usart_data_transmit(USART0, *p);
        p++;
    }

    return 0;
}

void uart0_handle(void)
{
    static uint8_t at_start_flag = 0;

    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)){
        char ch = (char)usart_data_receive(USART0);
        uart0_rxbuf1[rx_cnt] = ch;
        rx_cnt++;
        if (ch==3) {
//					memcpy(uart1_rxbuf1,uart0_rxbuf1,2000);
					len_rxmeter=rx_cnt;
					rx_cnt=0;
					sim_flag=7;
        }
        else if (ch==2) {
					rx_cnt = 1;
        }
				if(rx_cnt>2000){
					rx_cnt=0;
				}
    }	
}

void uart1_handle(void){
	if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE)){
        uint8_t ch = usart_data_receive(USART1);
				uart1_rxbuf[uart1_index]= ch;
				uart1_index++;
			if(uart1_index>3){
				if(uart1_rxbuf[uart1_index-1]=='Y' && uart1_rxbuf[uart1_index-2]=='D' && uart1_rxbuf[uart1_index-3]=='A'){
					sim_flag=1;
					uart1_index=0;
					memset(uart1_rxbuf,0,4000);
				}
				else if(uart1_rxbuf[uart1_index-1]=='?' && uart1_rxbuf[uart1_index-2]=='L' && uart1_rxbuf[uart1_index-3]=='L' && uart1_rxbuf[uart1_index-4]=='A'){
					sim_flag=23;					
				}
				else if(uart1_rxbuf[uart1_index-1]=='=' && uart1_rxbuf[uart1_index-2]=='L' && uart1_rxbuf[uart1_index-3]=='L' && uart1_rxbuf[uart1_index-4]=='A'){
					sim_flag=11;
				}
				else if(uart1_rxbuf[uart1_index-1]=='N' && uart1_rxbuf[uart1_index-2]=='E' && uart1_rxbuf[uart1_index-3]=='P'){
					sim_flag=13;					
				}
				else if(uart1_rxbuf[uart1_index-1]==':' && uart1_rxbuf[uart1_index-2]=='P' && uart1_rxbuf[uart1_index-3]=='C' && uart1_rxbuf[uart1_index-4]=='T' && uart1_rxbuf[uart1_index-5]=='R'){
					sim_flag=14;					
				}
/*				if(ch=='>'){
					printf("uart1: %c %d\r\n",ch, ch);
					memset(uart1_rxbuf,0,4000);
					memset(uart1_tx,0,2000);
					memset(uart1_temp,0,2000);
					uart1_index=0;
					sim_flag=17;
				}*/
				if(sim_flag==11 && uart1_rxbuf[uart1_index-1]=='\n' && uart1_rxbuf[uart1_index-2]=='\r' && uart1_rxbuf[uart1_index-3]=='R'){
					sim_flag=1;
					uart1_index=0;					
					memset(uart1_rxbuf,0,4000);
				}
				else if(sim_flag==11 && uart1_rxbuf[uart1_index-1]=='\n' && uart1_rxbuf[uart1_index-2]=='\r' && uart1_rxbuf[uart1_index-3]=='K'){
					sim_flag=2;
					uart1_index=0;					
					memset(uart1_rxbuf,0,4000);
				}
				if(sim_flag==13 && uart1_rxbuf[uart1_index-1]=='\n'){
					sim_flag=3;
					uart1_index=0;					
					memset(uart1_rxbuf,0,4000);
				}
				else if(sim_flag==23 && uart1_rxbuf[uart1_index-1]=='\n'){
					sim_flag=3;
					uart1_index=0;					
					memset(uart1_rxbuf,0,4000);
				}
				else if(sim_flag==14 && uart1_rxbuf[uart1_index-1]=='3' && uart1_rxbuf[uart1_index-2]=='0'){
					memcpy(uart1_temp,uart1_rxbuf,uart1_index);
					len_meter_buf=uart1_index-1;
					etx=uart1_index-1;
					sim_flag=24;
					uart1_index=0;
					memset(uart1_rxbuf,0,4000);
				}
				if(uart1_index>=4000){
					uart1_index=0;
				}
				else if(uart1_rxbuf[uart1_index-1]=='\n'){
					uart1_index=0;
					memset(uart1_rxbuf,0,4000);
				}
			}
      if(DEBUGMODE)   usart_data_transmit(USART0, ch);
			if(uart1_index>=4000){
					uart1_index=0;
					memset(uart1_rxbuf,0,4000);				
			}
    }
}
void send_data_server(){
	memset(uart1_tx,0,2000);
	int i=0;
	for(i=0;i<len_rxmeter;i++){
		usart_data_transmit(USART1, uart0_rxbuf1[i]);
	}
}
void prepare_data_server(){
	memset(uart1_tx,0,2000);
	sprintf(uart1_tx,"AT+MIPSEND=%d,%d\r\n",socket_id,len_rxmeter);
	uart1_send_string(uart1_tx);
	delay(10);
	send_data_server();
	sim_flag=17;
}
void extract_data(int idx){
	if(DEBUGMODE)printf("Extract Data\r\n");
	int i=0, j=0, len=0;
		for(j=0;j<len_meter_buf;j++){
				meter_buf[j] = meter_buf[idx+j];
			if(j>0){
				if(meter_buf[j]==','){
					if(meter_buf[j-1]!='0') socket_id = meter_buf[j-1];
					else len = j+1;
					}
				if(meter_buf[j-1]=='0'&& meter_buf[j]=='2') stx = j-1;
				if(meter_buf[j-1]=='0'&& meter_buf[j]=='3') etx = j;				
			}
		}
		socket_id = socket_id - 48;
		stx = len;
		len_meter_buf = etx - stx+1;
		if(DEBUGMODE) printf("1.stx etx len: %d %d %d %d meter_buf %c %c\r\n",stx,etx,len_meter_buf, socket_id, meter_buf[stx+1], meter_buf[etx]);
		j=0;
		for(j=0;j<len_meter_buf;j++){
			if(j>0){
			if(meter_buf[j-2]==',' && meter_buf[j-1]=='0' && meter_buf[j]=='2'){
					stx = j-1;
				}
			if(meter_buf[j-1]=='0' && meter_buf[j]=='3'){
				etx = j-1;
				}				
			}
		}
		j=0;
		for(j=0;j<len_meter_buf;j++){
				meter_buf[j]=meter_buf[stx+j];
				if(meter_buf[j-1]=='0' && meter_buf[j]=='3'){
					stx=0;
					etx = j;
				}				
		}
		len_meter_buf = etx - stx + 1;
		if(DEBUGMODE) printf("2.stx etx len: %d %d %d %d meter_buf %c %c\r\n",stx,etx,len_meter_buf, socket_id, meter_buf[stx+1], meter_buf[etx]);
//		sim_flag=4;
}
void print_data_meterbuf(){
	int i=0, j=0, len=0;
	for(i=0;i<len_meter_buf;i++){
	   usart_data_transmit(USART0, meter_buf[i]);		
	}
}
void send_to_meter(){
	int i=0, j=0, len=0;
	for(i=0;i<len_meter_buf;i++){
	   usart_data_transmit(USART0, uart0_rxbuf[i]);		
	}
}
/*
uint8_t hex2int(char *hex) {
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++; 
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        // shift 4 to make space for new digit, and add the 4 bits of the new digit 
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}*/
uint8_t hex2int(char hex) {
    uint32_t val = 0;
			// get current character then increment
			uint8_t byte = hex; 
			// transform hex character to the 4bit equivalent number, using the ascii table indexes
			if (byte >= '0' && byte <= '9') byte = byte - '0';
			else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
			else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
			// shift 4 to make space for new digit, and add the 4 bits of the new digit 
			val = (val << 4) | (byte & 0xF);
    return val;
}
void convert_data_meter(void){
  memset(uart0_rxbuf, 0, 2000);
	//memcpy(uart0_rxbuf,meter_buf,2000);
	int i=0;
	uint8_t lsb=0, msb=0, tempint=0;
	if(DEBUGMODE)	printf("\r\ndata ");
	for(i=0;i<len_meter_buf;i++){
		tempint=hex2int(meter_buf[i]);
		meter_buf[i] = tempint;
		if(DEBUGMODE)	printf("%d ", meter_buf[i]);
	}
		if(DEBUGMODE)	printf("\r\n");
	len_meter_buf = len_meter_buf/2;
	i=0;	
	if(DEBUGMODE)	printf("\r\ndata after convert ");
	for(i=0;i<len_meter_buf;i++){
		msb = meter_buf[2*i];
		lsb = meter_buf[2*i+1];
		uart0_rxbuf[i] = (msb << 4) | (lsb & 0xF);
		if(DEBUGMODE)	printf("%X ", uart0_rxbuf[i]);
	}
		if(DEBUGMODE)	printf("\r\n");

  if(DEBUGMODE)printf("\r\n3. %d %d %d %d %d %d %d %d %d\r\n",
		meter_buf[0],meter_buf[1],meter_buf[len_meter_buf*2-1], meter_buf[len_meter_buf], 
	uart0_rxbuf[0], uart0_rxbuf[1], uart0_rxbuf[2],uart0_rxbuf[len_meter_buf-1], len_meter_buf);
/*	if(DEBUGMODE){
		printf("Click Hex Now\r\n");
		delay(100);
	}*/
	send_to_meter();
/*	if(DEBUGMODE){
		delay(100);
	}*/
	if(DEBUGMODE)printf("\r\nFB Meter Send Now\r\n");
}
void copy_uart1_to_meter(void){
	memcpy(meter_buf,uart1_temp,4000);
	int i=0, j=0, len=0;
	for(i=3;i<len_meter_buf;i++){
		if(meter_buf[i-1]==':' && meter_buf[i-2]=='P' && meter_buf[i-3]=='C'){
			len=i+1;
		}
	}
	len_meter_buf = len;
	extract_data(len);
	if(DEBUGMODE) print_data_meterbuf();
	convert_data_meter();
	if(DEBUGMODE) printf("\r\n4.stx etx len: %d %d %d %d meter_buf %d %d\r\n",stx,etx,len_meter_buf, socket_id, meter_buf[0], meter_buf[len_meter_buf-1]);
	//send_to_meter();
	sim_flag=4;
}

void mcu_version(int arg)
{
    if(DEBUGMODE) printf("%s\r\n", MCU_SOFT_VERSION);
    if(DEBUGMODE) printf("OK\r\n");
}

void wdt_on(int arg)
{
//    if(DEBUGMODE) printf("OK\r\n");
    delay(50);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    gpio_bit_reset(GPIOA, GPIO_PIN_0);
}

void L716_reset(int arg)
{
    gpio_bit_set(GPIOB, GPIO_PIN_1);
    delay(50);
    gpio_bit_reset(GPIOB, GPIO_PIN_1);
    if(DEBUGMODE) printf("OK\r\n");
}

void led_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);

    gpio_bit_reset(GPIOC, LED_GPIO_PIN); //all led off
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_GPIO_PIN);
    gpio_bit_set(GPIOC, LED_GPIO_PIN); //all led on
}

void led_on(int led_num)
{
    gpio_bit_set(GPIOC, BIT(GPIO_PIN_SOURCE_13+led_num));
    if(DEBUGMODE) printf("OK\r\n");
}

void led_off(int led_num)
{
    gpio_bit_reset(GPIOC, BIT(GPIO_PIN_SOURCE_13+led_num));
    if(DEBUGMODE) printf("OK\r\n");
}

typedef struct {
   char *cmd;
   void (*hanlder)(int arg);
   int arg;
} odm_cmds_t;

const odm_cmds_t odm_cmds[] =
{
    {"AT+GTMCUVER",         mcu_version,    0},

    {"AT+GTMCUGPIO=2,1",    led_on,         0},
    {"AT+GTMCUGPIO=2,0",    led_off,        0},
    {"AT+GTMCUGPIO=3,1",    led_on,         1},
    {"AT+GTMCUGPIO=3,0",    led_off,        1},
    {"AT+GTMCUGPIO=4,1",    led_on,         2},
    {"AT+GTMCUGPIO=4,0",    led_off,        2},

    {"AT+GTMCUGPIO=10,0",   wdt_on,       0},
    {"AT+GTMCUGPIO=19,1",   L716_reset,     0},
};

int main(void)
{
    uint32_t i = 0;
    uint32_t cmd_len = sizeof(odm_cmds)/sizeof(odm_cmds[0]);

    //enable gpio clk
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);

    led_init();
    uart_init();
		if(DEBUGMODE)    printf("MCU_Start\r\n");

    //start: reset L716 module
    gpio_bit_set(GPIOB, GPIO_PIN_1);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    delay(100);
    gpio_bit_reset(GPIOB, GPIO_PIN_1);
    //end: reset L716 module
    delay(400);
		led_off(0);
		led_off(1);
		led_off(2);
    if(DEBUGMODE) printf("GO\r\n");
    
    while (1)
    {
/*
			if (uart0_rec_flag) {
            int mcu_at_flag = 0;

            for (i=0; i<cmd_len; i++) {
                odm_cmds_t *pcmd = (odm_cmds_t *)&odm_cmds[i];
                if (pcmd->hanlder != NULL && !strncasecmp(uart0_rxbuf, pcmd->cmd, strlen(pcmd->cmd))) {
                    printf("%s", uart0_rxbuf);
                    pcmd->hanlder(pcmd->arg);
                    mcu_at_flag = 1;
                    break;
                }
            }

            if (mcu_at_flag == 0) {
                uart1_send_string(uart0_rxbuf);
            }

            memset(uart0_rxbuf, 0, sizeof(uart0_rxbuf));
            rx_cnt = 0;
            uart0_rec_flag = 0;
        }
				*/
				if(sim_flag!=0){
/*					if(rx1_flag==1){
						delay(200);
						sim_flag=1;
					}
				*/
					if(sim_flag==1){
						if(DEBUGMODE) printf("simFlag: 1\r\n");
						led_on(0);
						uart1_send_string("ATE1\r\n");
						delay(200);
						uart1_send_string("AT+MIPCALL=1,\"M2MPLNAMR\"\r\n");
						sim_flag=99;
						delay(50);
					}
					else if(sim_flag==2){
						if(DEBUGMODE) printf("simFlag: 2\r\n");
						uart1_send_string("AT+MIPOPEN=1,22,\"0.0.0.0\",0,0\r\n");
						sim_flag=99;
						delay(10);
					}
					else if(sim_flag==3){
						if(DEBUGMODE) printf(" simFlag: 3 ");
//						uart1_send_string("AT+MIPCALL?\r\n");
						led_on(1);
						led_off(2);
						delay(10);
//						delay(300);
					}
					else if(sim_flag==4){
						if(DEBUGMODE) printf("simFlag: 4\r\n");
						led_on(2);
						delay(10);
//						uart1_send_string("AT+MIPCALL=1,\"M2MPLNAMR\"\r\n");
					}
					else if(sim_flag==24){
						if(DEBUGMODE) printf("simFlag: 24\r\n");
						copy_uart1_to_meter();
						led_off(1);						
						delay(10);
					}
					else if(sim_flag==7){
						if(DEBUGMODE) printf("simFlag: 7. Sending toServer\r\n");
						led_off(2);
						prepare_data_server();
						delay(20);
//						uart1_send_string("AT+MIPCALL=1,\"M2MPLNAMR\"\r\n");
					}
					else if(sim_flag==17){
						if(DEBUGMODE) printf("simFlag: 17. Done Sending DataMetertoServer Now\r\n");
						delay(10);
//						send_data_server();
						sim_flag=3;
						memset(meter_buf,0,4000);
						memset(uart0_rxbuf1,0,2000);
						memset(uart0_rxbuf,0,2000);
						memset(uart1_temp,0,4000);
						memset(uart1_rxbuf,0,4000);
						memset(uart1_tx,0,2000);
						len_meter_buf=0;
						len_rxmeter=0;
					}
					else if(sim_flag==27){
//						if(DEBUGMODE) printf("simFlag: 27. wait\r\n");
						delay(10);
					}
					else if(sim_flag==99){
//						if(DEBUGMODE) printf("simFlag: 99. wait\r\n");
						delay(10);
					}
					
				}
    }
}
