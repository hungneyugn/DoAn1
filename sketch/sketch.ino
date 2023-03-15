#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include<string.h>
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},           // A: ON/OFF
  {'4', '5', '6', 'B'},           // B: UP
  {'7', '8', '9', 'C'},           // C: DOWN
  {'*', '0', '#', 'D'}            // B: OK
};                                // #: delect

byte pin_rows[ROW_NUM]      = {19,18,32,33}; 
byte pin_column[COLUMN_NUM] = {25,26,27,2};   
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
LiquidCrystal_I2C lcd(0x27,16,2);
uint8_t choice =1;
bool state = 0;

void turn_On_OFF()
{
  state =!state;
  Serial.println(state);
  if(state == 1)
  {
    //turn on lcd
    lcd.init();       
    lcd.backlight();
    //man hinh chinh
    lcd.setCursor(1,0);   
    lcd.print("CHOOSE A OPTION");
    lcd.setCursor(0,1);  
    lcd.print("> ENTER PASSWORD");
  }
  else 
  {
    lcd.clear();
    lcd.noBacklight();
    choice =1;
  }
}

void setup() 
{
  Serial.begin(9600);
}
void menu(char key)             //mainscreen
{
  if(key == 'B')choice -=1;
  else if (key == 'C')choice += 1;
  if(choice == 1)
  {
    lcd.clear();
    lcd.setCursor(1,0);   
    lcd.print("CHOOSE A OPTION");
    lcd.setCursor(0,1);  
    lcd.print("> ENTER PASSWORD");
  }
  else if(choice == 2)
  {
    lcd.clear();
    lcd.setCursor(0,0);   
    lcd.print("> SCAN ID CARD");
    lcd.setCursor(0,1);  
    lcd.print("  FINGERPRINT");
  }
  else if(choice == 3)
  {
    lcd.clear();
    lcd.setCursor(0,0);   
    lcd.print("  SCAN ID CARD");
    lcd.setCursor(0,1);  
    lcd.print("> FINGERPRINT");
  }else if(choice < 1)choice =1;
  else choice =3;

}
void choose(uint8_t choice)     //choose function
{
  switch(choice)
  {
    case 1 : enterpass();
              break;
    case 2:  scanID();
              break; 
    case 3 : finger();
              break;      
  }
}
void mocua()                    // open cabinit
{
  
}
void thongBao(char* thongbao)   // Hien thi thong bao
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(thongbao);
}
void delect(int i)
{
  if(i>=0 && i<=6)
  {
    lcd.setCursor(i+4,1);
    lcd.print(" ");
    Serial.print(i);
    lcd.setCursor(i+4,1);
  }
}
void enterpass()                //nhap mat khau
{
 char a[7] ="" ;
 char pass[] ="123456";
  int i =0;
  again:
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  lcd.setCursor(5,1);
  lcd.blink();
  while(i<6)
  {
    char key = keypad.getKey();
    a[i]=key;
    if(a[i] == '#')
    {
      Serial.print(i);
      delect(i);
      i--;
    }
    else if (a[i]) 
    {
      lcd.print(a[i]);
      i++;
    }
  }
  lcd.noBlink();
  if(strcmp((char*)a,(char*)pass) == 0)
  {
    thongBao((char*)"CABINET IS OPEN");
  }else
  {
    thongBao((char*)"WRONG PASSWORD");
    delay(3000);
    goto again;
  }
}
void scanID()
{
  
}
void finger()
{

}

void loop() {
  char key = keypad.getKey();
  if(key == 'A')
  {
    turn_On_OFF();
  } 
  else if((key == 'B'&& state == 1)||(key == 'C'&& state == 1))menu(key);
  else if(key =='D')choose(choice);
}
