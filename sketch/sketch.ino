#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include<string.h>
#include <EEPROM.h>
#include <setjmp.h>
#define EEPROM_SIZE 6
#define relay 2
jmp_buf buf;

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns
char keys[ROW_NUM][COLUMN_NUM] = 
{                                 // A: ON/OFF
  {'1', '2', '3', 'A'},           // B: UP
  {'4', '5', '6', 'B'},           // C: DOWN
  {'7', '8', '9', 'C'},           // B: OK
  {'*', '0', '#', 'D'}            // #: delect
};                                // *: return

byte pin_rows[ROW_NUM]      = {12,14,32,33}; 
byte pin_column[COLUMN_NUM] = {25,26,27,4};   
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
LiquidCrystal_I2C lcd(0x27,16,2);
uint8_t choice =1;
bool state = 0;
char pass1[7]="";
void turn_On_OFF()
{
  state =!state;
  Serial.println(state);
  int again2;
  again2 = setjmp(buf);
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
  lcd.init();
  lcd.clear();
  lcd.noBacklight();
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  pinMode(relay,OUTPUT);
  //savepass(pass);
  readpass(pass1);
  Serial.print(pass1);
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
  lcd.setCursor(1,0);
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
void savepass(char pass[])
{
   for(int i =0 ; i<6;i++)
  {
    EEPROM.write(i,pass[i]);
    EEPROM.commit();
  }
}
void readpass(char a[])
{
  for(int i =0 ; i<6;i++)
  {
    a[i] = EEPROM.read(i);
  }
}
void open_cabinet()
{
  digitalWrite(relay,HIGH);
}
void close_cabinet()
{
  digitalWrite(relay,LOW);
}
void enterpass()                //nhap mat khau
{
  again:
  char a[7] ="" ;
  int i =0;
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  lcd.setCursor(5,1);
  lcd.blink();
  while(i<7)
  {
    char key = keypad.getKey();
    if(key == '#')
    {
      Serial.print(i);
      delect(i);
      i--;
      if(i < 0)i=0;
    }
    else if(key == 'D' && i == 6)
    { 
        i++;
    }
    else if(key == '*')
    {
      longjmp(buf,1);
    }
    else if (key && key!='A'&& key!='B'&& key!='C'&& key!='D'&& i<6 ) 
    {
      a[i]=key;
      Serial.println(i);
      lcd.print(a[i]);
      i++;
    }
  }
  lcd.noBlink();
  if(strcmp((char*)a,(char*)pass1) == 0)
  {
    open_cabinet();
    thongBao((char*)"CABINET IS OPEN");
  }else
  {
    thongBao((char*)"WRONG PASSWORD");
    delay(1000);
    goto again;
  }
}
void scanID()
{
  char key = keypad.getKey();
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("PUT IN ID CARD");
  lcd.setCursor(5,1);
  if(key == '*') longjmp(buf,1);
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
