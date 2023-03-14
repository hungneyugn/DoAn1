#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
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

int cursorColumn = 4;
void setup() {
  Serial.begin(9600);

}
void menu(char key)
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
void choose(uint8_t choice)
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

void enterpass()
{
 char a[6];
  int i =0;
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("ENTER PASSWORD");
  lcd.setCursor(5,1);
  lcd.blink();
  while(i<6)
  {
    char key = keypad.getKey();
    printf("%d\n",i);
    if (key) {
    lcd.print(key);
    i++;
  }
}
  lcd.noBlink();   
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
