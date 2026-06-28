/************************************************************
 *  TM4C123 Snake Game with ST7735 LCD
 *  ---------------------------------------------------------
 *  Microcontroller : TM4C123
 *  Language        : C
 *  Display         : ST7735 (SPI)
 *  Input           : Gamepad (PORT A)
 ************************************************************/

#include "TM4C123.h"
#include <stdint.h>
#include <stdlib.h>

/* =========================================================
   SYSTEM CONTROL REGISTER
   ========================================================= */
#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long*) 0x400FE608))

/* =========================================================
   PORT A REGISTERS (GAMEPAD INPUT)
   ========================================================= */
#define GPIO_PORTA_DATA_R (*((volatile unsigned long*) 0x400043FC))
#define GPIO_PORTA_DIR_R  (*((volatile unsigned long*) 0x40004400))
#define GPIO_PORTA_DEN_R  (*((volatile unsigned long*) 0x4000451C))
#define GPIO_PORTA_PDR_R  (*((volatile unsigned long*) 0x40004514))

/* Gamepad button pins */
#define P0 (1<<0)
#define P2 (1<<2)   // Right
#define P3 (1<<3)   // Left
#define P4 (1<<4)   // Down
#define P5 (1<<5)   // Up
#define P6 (1<<6)   // Exit / Reset

/* =========================================================
   LCD CONTROL PINS (PORT B)
   ========================================================= */
#define CS_LOW()   (GPIOB->DATA &= ~(1<<2))
#define CS_HIGH()  (GPIOB->DATA |=  (1<<2))
#define DC_CMD()   (GPIOB->DATA &= ~(1<<3))
#define DC_DATA()  (GPIOB->DATA |=  (1<<3))
#define RST_LOW()  (GPIOB->DATA &= ~(1<<1))
#define RST_HIGH() (GPIOB->DATA |=  (1<<1))

/* =========================================================
   COLOR DEFINITIONS (16-bit RGB565)
   ========================================================= */
#define BLACK 0x0000
#define RED   0x001F
#define GREEN 0x07E0
#define WHITE 0xFFFF

/* =========================================================
   GAME CONSTANTS
   ========================================================= */
#define BLOCK   8      // size of one snake block
#define MAX_LEN 53     // maximum snake length

void SystemInit(void) {}

/* =========================================================
   GAMEPAD INITIALIZATION
   ========================================================= */
void gamepad_init(void){
    GPIO_PORTA_DEN_R |= P2|P3|P4|P5|P6;    // enable digital pins
    GPIO_PORTA_DIR_R &= ~(P2|P3|P4|P5|P6); // inputs
    GPIO_PORTA_PDR_R |= P2|P3|P4|P5|P6;    // pull-down resistors
}

/* Simple delay function */
void delay(unsigned long d){
    while(d--);
}

/* =========================================================
   SSI2 (SPI) INITIALIZATION FOR LCD
   ========================================================= */
void SSI2_Init(void)
{
    SYSCTL->RCGCGPIO |= (1<<1);   // Enable Port B
    SYSCTL->RCGCSSI  |= (1<<2);   // Enable SSI2
    while(!(SYSCTL->PRGPIO & (1<<1)));

    /* SPI pins */
    GPIOB->AFSEL |= (1<<4)|(1<<7);
    GPIOB->PCTL  |= 0x20020000;
    GPIOB->DEN   |= (1<<4)|(1<<7);

    /* Control pins */
    GPIOB->DIR |= (1<<1)|(1<<2)|(1<<3);
    GPIOB->DEN |= (1<<1)|(1<<2)|(1<<3);

    CS_HIGH();
    DC_DATA();
    RST_HIGH();

    SSI2->CR1 = 0;
    SSI2->CR0 = 0x07;     // 8-bit, Freescale SPI
    SSI2->CPSR = 10;      // clock divider
    SSI2->CR1 |= (1<<1);  // enable SSI
}

/* Write one byte on SPI */
void SPI_Write(uint8_t data){
    while(!(SSI2->SR & 2));
    SSI2->DR = data;
    while(SSI2->SR & 0x10);
}

/* =========================================================
   ST7735 LCD FUNCTIONS
   ========================================================= */
void ST7735_Command(uint8_t cmd){
    CS_LOW();
    DC_CMD();
    SPI_Write(cmd);
    CS_HIGH();
}

void ST7735_Data(uint8_t data){
    CS_LOW();
    DC_DATA();
    SPI_Write(data);
    CS_HIGH();
}

void ST7735_Init(void){
    RST_LOW(); delay(50000);
    RST_HIGH(); delay(50000);

    ST7735_Command(0x01); delay(150000); // software reset
    ST7735_Command(0x11); delay(150000); // sleep out
    ST7735_Command(0x3A); ST7735_Data(0x05); // 16-bit color
    ST7735_Command(0x29); // display ON
}

/* Set drawing window */
void ST7735_SetAddrWindow(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1){
    ST7735_Command(0x2A);
    ST7735_Data(0); ST7735_Data(x0);
    ST7735_Data(0); ST7735_Data(x1);

    ST7735_Command(0x2B);
    ST7735_Data(0); ST7735_Data(y0);
    ST7735_Data(0); ST7735_Data(y1);

    ST7735_Command(0x2C);
}

/* Fill rectangle on LCD */
void ST7735_FillRect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint16_t c){
    uint32_t p = w*h;
    ST7735_SetAddrWindow(x,y,x+w-1,y+h-1);
    while(p--){
        ST7735_Data(c>>8); ///msb
        ST7735_Data(c);
    }
}

/* =========================================================
   SIMPLE RANDOM FUNCTION
   ========================================================= */
unsigned int simple_rand(unsigned int seed){
    seed = seed*1103515245 + 12345;
    return seed;
}

/* =========================================================
   MAIN FUNCTION
   ========================================================= */
int main(void)
{
    /* Movement step */
    volatile unsigned int mov = BLOCK;
    volatile int dirx = 0, diry = mov;

    /* Snake body arrays */
    volatile int snakeX[MAX_LEN];
    volatile int snakeY[MAX_LEN];
    volatile unsigned int length = 1;

    /* Food position */
    volatile unsigned int foodx, foody;

    /* Head position */
    volatile unsigned int posx = 0, posy = 0;
    volatile unsigned int i;

    /* Initialization */
    SYSCTL_RCGCGPIO_R |= P0;
    gamepad_init();
    SSI2_Init();
    ST7735_Init();

    while(1){
RESTART:
        /* Clear screen */
        ST7735_FillRect(0,0,128,128,BLACK);

        /* Reset snake */
        posx = 0;
        posy = 0;
        snakeX[0] = posx;
        snakeY[0] = posy;
        length = 1;

        /* Random food position */
        foodx = (simple_rand(7)%16)*BLOCK;
        foody = (simple_rand(11)%16)*BLOCK;

        while(1){

            /* -------- DIRECTION CONTROL (ANTI-REVERSE) -------- */

            if((GPIO_PORTA_DATA_R & P2) && (dirx != -mov)){
                dirx = mov; diry = 0;       
            }
            else if((GPIO_PORTA_DATA_R & P3) && (dirx != mov)){
                dirx = -mov; diry = 0;      
            }
            else if((GPIO_PORTA_DATA_R & P4) && (diry != -mov)){
                dirx = 0; diry = mov;       
            }
            else if((GPIO_PORTA_DATA_R & P5) && (diry != mov)){
                dirx = 0; diry = -mov;      
            }
            else if(GPIO_PORTA_DATA_R & P6){
                ST7735_FillRect(0,0,128,128,GREEN);
                break;
            }

            /* Remove tail */
            ST7735_FillRect(snakeX[length-1],snakeY[length-1],BLOCK,BLOCK,BLACK);

            /* Shift body */
            for(i=length-1;i>0;i--){
                snakeX[i]=snakeX[i-1];
                snakeY[i]=snakeY[i-1];
            }

            /* Move head */
            snakeX[0]+=dirx;
            snakeY[0]+=diry;

            /* -------- SCREEN WRAP LOGIC -------- */

            /* wrap UP ? bottom */
            if (snakeY[0] < 0 && diry < 0){
                snakeY[0] = 120;
            }

            /* wrap LEFT ? right */
            if (snakeX[0] < 0 && dirx < 0){
                snakeX[0] = 120;
            }

            /* wrap DOWN ? top */
            if (snakeY[0] > 120){
                snakeY[0] = 0;
            }

            /* wrap RIGHT ? left */
            if (snakeX[0] > 120){
                snakeX[0] = 0;
            }
            /* -------- SELF COLLISION -------- */
            for(i=1;i<length;i++){
                if(snakeX[0]==snakeX[i] && snakeY[0]==snakeY[i]){
                    ST7735_FillRect(0,0,128,128,GREEN);
                    delay(2000000);
                    goto RESTART;
                }
            }

            /* -------- FOOD COLLISION -------- */
            if(snakeX[0] < foodx+BLOCK &&
               snakeX[0]+BLOCK > foodx &&
               snakeY[0] < foody+BLOCK &&
               snakeY[0]+BLOCK > foody)
            {
                if(length < MAX_LEN) length++;
                foodx = (simple_rand(length+3)%16)*BLOCK;
                foody = (simple_rand(length+9)%16)*BLOCK;
            }

            /* Draw food */
            ST7735_FillRect(foodx,foody,BLOCK,BLOCK,RED);

            /* Draw snake */
            for(i=0;i<length;i++){
                ST7735_FillRect(snakeX[i],snakeY[i],BLOCK,BLOCK,GREEN);
            }

            delay(1000000);
        }
    }
}
