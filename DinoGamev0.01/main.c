/*
 * main.c
 * Microcontroller: AVR Atmega32
 * LCD: HD4478 (16*2 Alphanumerical LCD)
 * Developer: Shantanu Rathod
 * Description: The code written below is a game logic(replication of Chrome Dinosaur Game)
 *				on 16*2 LCD display using ATMEGA32 AVR Microcontroller.
 *
 *				The game is simple:-
 *				User have to control the jump of DINO when a tree comes on it's way.
 *				To control the jump their is a push button. If DINO collide with the tree
 *				game is over. Score will be appearing at top right of LCD screen. After
 *				Game Over bestscore will be displayed below current score. Best score are
 *				stored in EEPROM of Microcntroller and gets updated if you current score 
 *				become higher than previous bestscore.
 *				After GameOver long press push button to restart the game.
 *				
 *				Custom Characters like DINO and Trees are made by mapping bits and then 
 *				stored in the CGROM of LCD.
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "lcd.h"

// function for storing custom character in CGROM of LCD
void CreateCustomCharacter (unsigned char *Pattern, const char Location)
{
  int i=0;
  lcd_command(0x40+(Location*8));     //Send the Address of CGRAM
  for (i=0; i<8; i++)
  {
    lcd_data (Pattern [ i ] );  //Pass the bytes of pattern on LCD
  }
         
}

//To display a string on LCD
void lcd_print(const char *s)
{
  register char c;

  while ( (c = *s++) ) {
    lcd_putc(c); //display character on LCD
  }
}

//display string by some delay(Animation)
void lcd_print2(const char *s)
{
  register char c;

  while ( (c = *s++) ) {
    lcd_putc(c);
    _delay_ms(10);
  }
  
  _delay_ms(25);
 }
 
 int max(int a, int b) //maximum of two value function
 {
	 if(a >= b)
	 {
		 return a;
	 }
	 
	 return b;
 }
 
#define LINE1_2BOX 0x82 //Jump position of Dino
#define LINE2_2BOX 0xC2 //Rest position of Dino
#define LINE2_15BOX 0xCF //Spawning position of Tree

int main(void)
{
  DDRA &= ~(1<<PA3); //set pin3 of PORTA as input
  lcd_init(LCD_DISP_ON); //Configuring LCD in 4-bit mode with Display ON cursor OFF
  lcd_gotoxy(3,0); //Cursor to Row:1 Col:4
  lcd_print2("DinoGame!!");
  
  //Bitmap of Dino(Custom Character)
  unsigned char Dino[] = {
    0b00011,
    0b00010,
    0b00011,
    0b00110,
    0b10110,
    0b11110,
    0b00100,
    0b00010
  };
  
  // Smashed Dino
  unsigned char sDino[] = {
    0b00011,
    0b00011,
    0b00011,
    0b00110,
    0b10111,
    0b11110,
    0b00100,
    0b00010
  };

  // Smashed Tree
  unsigned char sTree[] = {
    0b00100,
    0b01000,
    0b11100,
    0b10000,
    0b11100,
    0b10000,
    0b10000,
    0b10000
  };  
  
  //Storing Custom Character in CGROM
  CreateCustomCharacter(Dino,3);
  CreateCustomCharacter(sDino,1);
  CreateCustomCharacter(sTree,2);
  
  #define DINO 3
  #define SDINO 1
  #define STREE 2
  
  int i = 0; //for tree varying position
  int j = 0; //for score
  int k = 0; //for collision detection
  int l = 0; //time period in air for Dino
  int o = 0; //checking if game is over
  int Bestscore = 0;
  while(1)
  { 
  while(1)
  {
    label:
	  i %= 16; //to keep variable i within 16 range which limit of our LCD
      lcd_command(LINE2_15BOX - i); //changing position of tree with change in i
      lcd_data(0x5C); //display tree which actually a Yaun sign stored at 0x5C in CGROM of LCD
      
      //jump
      if(((PINA & (1<<PA3)) == 8) && (l < 3))
      {
        lcd_command(LINE1_2BOX);
        lcd_data(DINO); //display Dino
        k = 1;
        l++;
      }
      else //Default Position of Dino
      {
        lcd_command(LINE2_2BOX);
        lcd_data(DINO);
        k = 0;
        
        if(l == 3)
        {
          l = 0;
        }
      }
      
	  //Collision detection
      if(k == 0 && i == 13)
      {
		o = 1;
        break;
      }
	  
	  //speeding up the game with increasing score
	  if(j > 100)
	  {
		   _delay_ms(9);
	  }
	  else if(j > 300)
	  {
		   _delay_ms(8);
	  }
	  else if(j > 500)
	  {
		   _delay_ms(7);
	  }
	  else if(j > 900)
	  {
		   _delay_ms(5);
	  }
	  else
	  {
		 _delay_ms(10);  
	  }
     
      lcd_clrscr(); //clear screen
      
	  //Score printing
      if(j < 10)
      {
        lcd_gotoxy(10,0);
        lcd_print("Hi 00");
        lcd_putc('0' + j);
      }
      else if( j < 100)
      {
        lcd_gotoxy(10,0);
        lcd_print("Hi 0");
        lcd_putc('0' + (j/10));
        lcd_putc('0' + (j%10));
      }
      else if( j < 1000)
      {
        lcd_gotoxy(10,0);
        lcd_print("Hi ");
        lcd_putc('0' + (j/100));
        lcd_putc('0' + ((j/10)%10));
        lcd_putc('0' + ((j%100)%10));
      }
      i++;
      j++;
    
  }  
  
  lcd_command(LINE2_2BOX); //after collision state of Dino and Tree
  lcd_data(SDINO);
  lcd_data(STREE);
  _delay_ms(5);
  lcd_command(LINE2_2BOX);
  lcd_putc('*');
  lcd_gotoxy(0,0);
  
  lcd_print2("GAMEOVER"); //Game Over text animation
  
  //Display Best score
  Bestscore = max((j - 1),eeprom_read_word(0));
  eeprom_write_word(0,Bestscore);
  lcd_gotoxy(13,1);
  lcd_putc('0' + (Bestscore/100));
  lcd_putc('0' + ((Bestscore/10)%10));
  lcd_putc('0' + ((Bestscore%100)%10));
 
  //Retry Game
  if(((PINA & (1<<PB3))) == 8 && o == 1)  
  {
    i = 0;
    j = 0;
    k = 0;
    l = 0;
	o = 0;
    lcd_clrscr();
    lcd_gotoxy(3,0);
    lcd_print2("DinoGame!!");
    goto label; //go to label for game restart
  }
  }
}