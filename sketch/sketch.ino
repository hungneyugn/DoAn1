/*
*File: doan1.ino
*Author: Nguyen Phi Hung, Le Van Thanh
*Date: 14/03/2023
*Description: This is program that use to make the options to open cabinet
*/
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <string.h>
#include <EEPROM.h>
#include <SPI.h>      
#include <MFRC522.h>
#include <setjmp.h>
#include <Adafruit_Fingerprint.h>
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial);
/*Number of bytes saved in eeprom*/
#define EEPROM_SIZE 200
uint8_t id;
uint8_t numFinger;

#define relay 2
jmp_buf buf2;  // return main menu
jmp_buf buf3;  // return main menu (start state) when pressing key A
jmp_buf buf4;  // return master menu
jmp_buf buf5;  // return menu change id card
jmp_buf buf6;
//Setup keypad 4x4
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns
char keys[ROW_NUM][COLUMN_NUM] = 
{                                 // A: ON/OFF
  {'1', '2', '3', 'A'},           // B: UP
  {'4', '5', '6', 'B'},           // C: DOWN
  {'7', '8', '9', 'C'},           // B: OK
  {'*', '0', '#', 'D'}            // #: delect
};                                // *: return

byte pin_rows[ROW_NUM]      = {12,14,27,26}; 
byte pin_column[COLUMN_NUM] = {25,33,32,4};   
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
//Setup lcd
LiquidCrystal_I2C lcd(0x27,16,2);
//global variable
uint8_t g_choiceMainMenu =1;
uint8_t g_choiceMasterMenu =1;
uint8_t g_choiceChangeID =1;
uint8_t g_choiceFingerMenu = 1;
/*State of LCD | 0: off ; 1: on*/
bool g_state = 0;                
uint8_t flag = 0;
/*Pass read from EEPROM*/
char pass[7]="";  
/*Save last used EEPROM cell*/
uint16_t lastCell;                
//Setup RFID
#define RST  13
#define SDA  5
MFRC522 mfrc522(SDA, RST);
/*Save master id*/
uint8_t masterId[5]="";            
void main_Menu(char);
void choose_MainMenu(uint8_t);

/*
*Function: turn_On_OFF
*Description: Turn on or turn off lcd
*Input: None
*Output: None
*/
void turn_On_OFF()             
{
  /*Toggle state*/
  g_state =!g_state;
  Serial.println(g_state);
  int again2;
  /*Global variable to choice main menu*/
  g_choiceMainMenu =1;
  again2 = setjmp(buf2);
  if(g_state == 1)
  {
    lcd.init();       
    lcd.backlight();
    main_Menu(1);
  }
  else 
  {
    lcd.clear();
    lcd.noBacklight();
  }
}

/*
*Function: clearLine
*Description: clear a line of lcd
*Input:
*   line: one of two lines of lcd(0 or 1)
*Output: None
*/
void clearLine(uint8_t line)
{
  if(line == 0) lcd.setCursor(0,0);
  else lcd.setCursor(0,1);
  /*Print 16 spaces*/
  lcd.print("                ");
}

/*
*Function: displayLine
*Description: display string on a line
*Input:
*   index: index start to print string on a line
*   line: one of two lines of lcd(0 or 1)
*   inform: string which we want to print on lcd
*Output: None
*/
void displayLine(uint8_t index,uint8_t line,char* inform) 
{
  clearLine(line);
  lcd.setCursor(index,line);    
  lcd.print(inform);  
}

/*
*Function: delect
*Description: delect char using keypad
*Input:
    i: index of char need to be delected
*Output: None
*/
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

/*
*Function: savepass
*Description: save pass using keypad
*Input:
    a: array save password
*Output: None
*/
void savepass(char a[]) 
{
   for(int i = 0 ; i<6;i++)
  {
    pass[i] = a[i];
    EEPROM.write(i+1,a[i]);
    EEPROM.commit();
  }
}

/*
*Function: readpass
*Description: read pass from eeprom
*Input:
    a: array save password
*Output: None
*/
void readpass(char a[]) 
{
  for(int i =0 ; i<6;i++)
  {
    a[i] = EEPROM.read(i+1);
  } 
}

/*
*Function: Handle_Key
*Description: processing when press control key
*Input:
    key: save key which is pressed
    typeMenu : a function pointer to type of menu need to be operated
    choose_menu: a function pointer to function choose one of options of that menu
*Output: None
*/
void Handle_Key(char key,void(*typeMenu)(char),void(*choose_menu)(uint8_t,uint16_t*),uint8_t choice,uint16_t* lastCell)
{
  /*
  *key A: turn on or turn off lcd
  *key B: surf up 
  *key C: surf down
  *key D: enter(choose the option) 
  */  
  if(key == 'A')
  {
    turn_On_OFF();
  } 
  else if((key == 'B'&& g_state == 1)||(key == 'C'&& g_state == 1))typeMenu(key);
  else if(key =='D'&& g_state == 1)choose_menu(choice, lastCell);
}

/*
*Function: readIndex
*Description: read index of last cell used in eeprom
*Input:
    lastCell: Pointer to variable which save used last cell
*Output: None
*/
void readIndex(uint16_t *lastCell)
{
  *lastCell = EEPROM.read(0);
  Serial.print("lastcell: ");
  Serial.println(*lastCell);
}

/*
*Function: saveIndex
*Description: save new value of last cell to eeprom
*Input:
    lastCell: value of index used last cell
*Output: None
*/
void saveIndex(uint16_t lastCell)
{
  EEPROM.write(0,lastCell);
  EEPROM.commit();
}
/*
*Function: main_Menu
*Description: display main menu on LCD
*Input:
    key: key which is pressed
*Output: None
*/
void main_Menu(char key) 
{
  if(key == 'B')g_choiceMainMenu -=1;
  else if (key == 'C')g_choiceMainMenu += 1;
  if(g_choiceMainMenu == 1)
  {
    displayLine(0,0,(char*)"CHOOSE AN OPTION");
    displayLine(0,1,(char*)"> ENTER PASSWORD");
  }
  else if(g_choiceMainMenu == 2)
  {
    displayLine(0,0,(char*)"> SCAN ID CARD");
    displayLine(0,1,(char*)"  FINGERPRINT");
  }
  else if(g_choiceMainMenu == 3)
  {
    displayLine(0,0,(char*)"  SCAN ID CARD");
    displayLine(0,1,(char*)"> FINGERPRINT");
  }else if(g_choiceMainMenu < 1)g_choiceMainMenu =1;
  else g_choiceMainMenu =3;
}

/*
*Function: choose_MainMenu
*Description: Execute option which user choose
*Input:
*   choice: the choice option which user choose
*   lastCell: value of index used last cell
*Output: None
*/
void choose_MainMenu(uint8_t choice,uint16_t *lastCell)
{
  switch(choice)
  {
    case 1 : enterpass();
              break;
    case 2:  scanID(lastCell);
              break; 
    case 3 : scanfinger();
              break;      
  }
}
/*
*Function: open_cabinet
*Description: function open cabinet
*Input: None
*Output: None
*/
void open_cabinet()          
{
  digitalWrite(relay,HIGH);
}
/*
*Function: close_cabinet
*Description: function close cabinet
*Input: None
*Output: None
*/
void close_cabinet()           
{
  digitalWrite(relay,LOW);
}

/*
*Function: readIdCard
*Description: read id card
*Input:
*   id: a string save key of rfid card
*Output: None
*/
void readIdCard(uint8_t id[])
{
  Serial.println("vo nhung khong co the 1");
  /*check if the card is scanned or not*/
  if ( ! mfrc522.PICC_IsNewCardPresent()) return;
  /*check if reading card is successful or not*/
  Serial.println("vo nhung khong co the 2");
  if ( ! mfrc522.PICC_ReadCardSerial()) return;
  /*read key of card and save it in id argument*/
   Serial.println("co the");
  for (int i = 0; i < 4; i++) 
  { 
    id[i] = mfrc522.uid.uidByte[i];    
  }
  Serial.print("UID của thẻ: ");   
 for(int i =0;i<4;i++)
  {
    Serial.print(id[i]);
    Serial.print(" ");
  }
  Serial.println();
  mfrc522.PICC_HaltA();  
  mfrc522.PCD_StopCrypto1();
}

/*
*Function: readMasterID
*Description: read Master ID from EEPROM
*Input:
*   id: a string save key of rfid card
*Output: None
*/
void readMasterID(uint8_t id[])
{
  for(int i =0 ; i<4;i++)
  {
    id[i] = EEPROM.read(i+7);
  }
  for(int i =0;i<4;i++)
  {
    Serial.print(id[i]);
    Serial.print(" ");
  }
  Serial.println();
}

/*
*Function: setMasterID
*Description: save master id into EEPROM
*Input:
*   id: a string save key of rfid card
*Output: None
*/
void setMasterID(uint8_t id[])
{
  for(int i =0; i < 4;i++)
  {
    EEPROM.write(i+7,id[i]);
    EEPROM.commit();
  }
}

/*
*Function: enterpass
*Description: enter password using keypad, display on lcd
*Input: None
*Output: None
*/
void enterpass()
{
  again:
  char a[7] ="" ;
  int i =0;
  displayLine(1,0,(char*)"ENTER PASSWORD");
  clearLine(1);
  lcd.setCursor(5,1);
  lcd.blink();
  while(i<7)
  {
    char key = keypad.getKey();
    /*
    *key #: delect char
    *key *: return previous menu
    */    
    if(key == '#')
    {
      Serial.print(i);
      delect(i);
      i--;
      if(i < 0)i=0;
    }
    /*if enough 6 characters, increase i to cancel the loop*/
    else if(key == 'D' && i == 6)
    { 
        i++;
    }
    else if(key == '*')
    {
      longjmp(buf2,1);
    }
    /*password is only number characters*/
    else if (key && key!='A'&& key!='B'&& key!='C'&& key!='D'&& i<6 ) 
    {
      a[i]=key;
      Serial.println(i);
      lcd.print(a[i]);
      i++;
    }
  }
  lcd.noBlink();
  /*compare entered password and password readed from eeproom*/
  if(strcmp((char*)a,(char*)pass) == 0)
  {
    open_cabinet();
    displayLine(1, 0,(char*)"CABINET IS OPEN");
    clearLine(1);
    delay(3000);
    close_cabinet();
    longjmp(buf2,1);
  }else
  {
    displayLine(1, 0,(char*)"WRONG PASSWORD");
    clearLine(1);
    delay(1000);
    goto again;
  }
}

/*
*Function: scanID
*Description: scan id card
    lastCell: Pointer to variable which save used last cell
*Output: None
*/
void scanID(uint16_t *lastCell) 
{
  lable:
  char key;
  uint8_t idCard[5] ="";
  displayLine(1, 0, (char*)"PUT IN ID CARD");
  clearLine(1);
  lcd.setCursor(5,1);
  Serial.println("Da Vao");
  while(idCard[0]==0 && idCard[1]==0 && idCard[2]==0 && idCard[3]==0 && key != '*' && key !='A')
  {
    Serial.println("dang quet");
    readIdCard(idCard);
    key = keypad.getKey(); 
  }
  if(key == '*') longjmp(buf2,1);
  else if(key =='A')
  {
    turn_On_OFF();
    longjmp(buf3,1);
  }

  uint16_t index = CompareID(idCard,*lastCell);
   Serial.println(index);
  if(index == 0)
  {
    displayLine(1, 0,(char*)"WRONG ID CARD");
    clearLine(1);
    delay(1500);
    goto lable;    
  }
  else if(index == 7)
  {
    open_cabinet();
    displayLine(1, 0,(char*)"CABINET IS OPEN");
    clearLine(1);
    delay(3000);
    close_cabinet();
    master_Menu(1);
    while(key != '*' && key != 'A')
    {
      key = keypad.getKey();
      /*Execute master menu function*/ 
      Handle_Key(key,&master_Menu,&choose_MasterMenu,g_choiceMasterMenu,lastCell);
    }
    g_choiceMasterMenu = 1;
    if(key == '*') longjmp(buf2,1);
    else if(key =='A') longjmp(buf3,1);
  }
  else
  {    
    open_cabinet();
    displayLine(1, 0,(char*)"CABINET IS OPEN");
    clearLine(1);
    delay(3000);
    close_cabinet();
    longjmp(buf2,1);
  }
}

/*
*Function: scanfinger
*Description: Scan fingerprint
*Input: None
*Output: None
*/
void scanfinger()                   
{
  char key;
  int id = -1;
  displayLine(0, 0,(char*)"SCAN FINGERPRINT");
  clearLine(1);
  while(key != '*' && key != 'A'&& id == -1)
  { 
    uint8_t p = finger.getImage();
    key = keypad.getKey(); 
    if (p == FINGERPRINT_OK){
       key = keypad.getKey(); 
      id = checkvantay(p);
      if(id==-1)
      {
        displayLine(1, 0,(char*)"WRONG FGPRINT");
        clearLine(1);
        delay(1000);
        lcd.clear();
        displayLine(0, 0,(char*)"SCAN FINGERPRINT");
        clearLine(1);
      }
    }
  }
  if(key == '*') longjmp(buf2,1);
  else if(key =='A') longjmp(buf3,1);
  open_cabinet();
  displayLine(1, 0,(char*)"CABINET IS OPEN");
  clearLine(1);
  delay(3000);
  close_cabinet();
  longjmp(buf2,1);
}
/*
*Function: master_Menu
*Description: display security opitons which only execute by master
*Input:
    key: key which is pressed
*Output: None
*/
void master_Menu(char key)    
{
  if(key == 'B')g_choiceMasterMenu -=1;
  else if (key == 'C')g_choiceMasterMenu += 1;
  int again4;
  again4 = setjmp(buf4);
  if(g_choiceMasterMenu == 1)
  {
    displayLine(4,0,(char*)"SECURITY");
    displayLine(0,1,(char*)">CHANGE PASSWORD");
  }
  else if(g_choiceMasterMenu == 2)
  {
    displayLine(0,0,(char*)">CHANGE ID CARD");
    displayLine(0,1,(char*)" CHANGE FGPRINT");
  }
  else if(g_choiceMasterMenu == 3)
  {
    displayLine(0,0,(char*)" CHANGE ID CARD");
    displayLine(0,1,(char*)">CHANGE FGPRINT");
  }else if(g_choiceMasterMenu < 1)g_choiceMasterMenu =1;
  else g_choiceMasterMenu =3;
}

/*
*Function: choose_MasterMenu
*Description: Execute option which master choose
*Input:
*   choice: the choice option which master choose
*   lastCell: value of index used last cell
*Output: None
*/
void choose_MasterMenu(uint8_t choice,uint16_t *lastCell)
{
  switch(choice)
  {
    case 1 : changePass();
              break;
    case 2:  changeIDCARD(lastCell);
              break; 
    case 3 : changeFinger(lastCell);
              break;      
  }
}

/*
*Function: changePass
*Description: change password
*Input: None
*Output: None
*/
void changePass()
{
  char a[7] ="" ;
  int i =0;
  displayLine(2,0,(char*)"NEW PASSWORD");
  clearLine(1);
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
      lcd.noBlink();
      longjmp(buf4,1);
    }
    else if (key && key!='A'&& key!='B'&& key!='C'&& key!='D'&& i<6 ) 
    {
      a[i]=key;
      Serial.println(i);
      lcd.print(a[i]);
      i++;
    }
  } 
  savepass(a);
  displayLine(1, 0,(char*)"PASSWORD CHANGED");
  clearLine(1);
  delay(2000);
  lcd.noBlink();
  master_Menu(1); 
}

/*
*Function: changeFingerMenu
*Description: display change opitons fingerprint
*Input:
    key: key which is pressed
*Output: None
*/
void changeFingerMenu(char key)
{
  if(key == 'B')g_choiceFingerMenu -=1;
  else if (key == 'C')g_choiceFingerMenu += 1;
int again6;
again6 = setjmp(buf6);  
  if(g_choiceFingerMenu == 1)
  {
    displayLine(0,0,(char*)"> ADD FGPRINT");
    displayLine(0,1,(char*)"  REMOVE FGPRINT");
  }
  else if(g_choiceFingerMenu == 2)
  {
    displayLine(0,0,(char*)"  ADD FGPRINT");
    displayLine(0,1,(char*)"> REMOVE FGPRINT");
  }else if(g_choiceFingerMenu < 1)g_choiceFingerMenu =1;
  else g_choiceFingerMenu =2;
}

/*
*Function: chooseFinger
*Description: Execute option which user choose in change fingerprint menu
*Input:
*   choice: the choice option which user choose
*   lastCell: value of index used last cell
*Output: None
*/
void chooseFinger(uint8_t choice,uint16_t *lastCell)
{
  switch(choice)
  {
    case 1: addFinger();
            break;
    case 2: removeFinger();
            break; 
  }
}

/*
*Function: addFinger
*Description: add new fingerprint
*Input: None
*Output: None
*/
void addFinger()
{
  again:
  int id=-1;
  displayLine(0, 0,(char*)"SCAN FINGERPRINT");
  clearLine(1);  
  char key;
  flag = 0;
  while(key != '*' && key != 'A'&& id == -1 && flag == 0)
  { 
    uint8_t p = finger.getImage();
    key = keypad.getKey(); 
    if (p == FINGERPRINT_OK){
      id = checkvantay(p);
      if(id == -1 ){
        getFingerprint();
      }
      else{
          displayLine(1, 0,(char*)"FGPRINT EXISTS");
          clearLine(1);
          delay(1000);
          goto again;          
      }
    }
  }
  if(key == '*') 
  {
    g_choiceFingerMenu = 1;
    longjmp(buf6,1);
  }
  else if(key =='A') longjmp(buf3,1);
  displayLine(1, 0,(char*)"ADD COMPLETELY");
  clearLine(1);
  delay(1000);
  changeFingerMenu(1);  
}

/*
*Function: removeFinger
*Description: remove fingerprint
*Input: None
*Output: None
*/
void removeFinger()
{  
  again:
  uint8_t flag = 0;
  int id=-1;
  displayLine(0, 0,(char*)"SCAN FINGERPRINT");
  displayLine(1, 1,(char*)"NEED TO DELECT");  
  char key;
  while(key != '*' && key != 'A'&& id == -1 && flag == 0)
  { 
    uint8_t p = finger.getImage();
    key = keypad.getKey(); 
    if (p == FINGERPRINT_OK){
      id = checkvantay(p);
      if(id == -1 )
      {
        displayLine(2, 0,(char*)"FINGERPRINT");
        displayLine(1, 1,(char*)"DOESN'T EXIST"); 
        delay(1000);
        goto again;
      }
      else{
         delectFingerID(id);         
      }
    }
  }
  if(key == '*') 
  {
    g_choiceFingerMenu = 1;
    longjmp(buf6,1);
  }
  else if(key =='A') longjmp(buf3,1);
  displayLine(4, 0,(char*)"REMOVED");
  clearLine(1);
  delay(1000);
  changeFingerMenu(1);  
}

/*
*Function: changeFinger
*Description: Function change fingerprint
*Input: 
*   lastCell: point to the last cell
*Output: None
*/
void changeFinger(uint16_t *lastCell)
{
  changeFingerMenu(1);
  char key;
  while(key != '*' && key != 'A')
  {
    key = keypad.getKey();
    /*Execute fingerprint menu function*/ 
    Handle_Key(key,&changeFingerMenu,&chooseFinger,g_choiceFingerMenu,lastCell);
  }
  g_choiceFingerMenu =1;
  if(key == '*') 
  {
    master_Menu(1);
  }
  else if(key =='A') longjmp(buf3,1);
}

/*
*Function: changeID_Menu
*Description: display change opitons ID card
*Input:
    key: key which is pressed
*Output: None
*/
void changeID_Menu(char key)
{
  if(key == 'B')g_choiceChangeID -=1;
  else if (key == 'C')g_choiceChangeID += 1;
  int again5;
  again5 = setjmp(buf5);
  if(g_choiceChangeID == 1)
  {
    displayLine(0, 0,(char*)"> ADD ID CARD");
    displayLine(0, 1,(char*)"  REMOVE ID CARD"); 
  }
  else if(g_choiceChangeID == 2)
  {
    displayLine(0, 0,(char*)"  ADD ID CARD");
    displayLine(0, 1,(char*)"> REMOVE ID CARD");
  }else if(g_choiceChangeID < 1)g_choiceChangeID =1;
  else g_choiceChangeID =2;
}

/*
*Function: CompareID
*Description: compare scanned id card with id card stored in EEPROM if it existed in EEPROM or not
*Input:
*   idCard: string save scanned id card
*   lastCell: value of index used last cell
*Output: 
*   j: the last cell in four cells store scanned id card
*/
uint16_t CompareID(uint8_t idCard[],uint16_t lastCell)
{
  uint8_t id[5] ="";
  /*index cell save the firt ID */
  uint16_t j = 7;
  Serial.print("last cell in compare: ");
  Serial.println(lastCell);
  while(j < lastCell)
  {
    for(int i =0 ; i < 4;i++)
    {
      id[i] = EEPROM.read(i+j);
    }
    if(strcmp((char*)idCard,(char*)id) == 0)
    {
      return j;
    }
    else j+=4;
  }
  return 0;
}
/*
*Function: addID
*Description: add new id
*Input:
*   lastCell: pointer points to index used last cell
*Output: None
*/
void addID(uint16_t *lastCell)
{
  label1:
  char key;
  uint8_t id[5] ="";
  /*index of the firt cell save id except master id*/
  uint16_t j = 11;
  displayLine(1, 0,(char*)"SCAN NEW ID CARD");
  clearLine(1);
  while(id[0]==0 && id[1]==0 && id[2]==0 && id[3]==0 && key != '*' && key !='A')
  {
    readIdCard(id);
    key = keypad.getKey(); 
  }
  if(key == '*')
  {
    g_choiceChangeID = 1;
    longjmp(buf5,1);
  } 
  else if(key =='A')
  {
    turn_On_OFF();
    longjmp(buf3,1);
  }
  Serial.println(CompareID(id,*lastCell));
  if(CompareID(id,*lastCell)!= 0)
  {
    displayLine(3, 0,(char*)"ID EXISTES");
    clearLine(1);
    delay(1500);
    goto label1;
  }
  else 
  {
      while(j < *lastCell)
      {
        int dem = 0;
        /*check 4 contiguous empy cell*/
        while(dem < 4 && EEPROM.read(dem + j) == 255) dem++;
        if(dem == 4)
        {
          for(int i =0; i < 4;i++)
          {
            EEPROM.write(i+j,id[i]);
            EEPROM.commit();
          }
          goto lable2;
        }else j+=4;
      }
      for(int i =0; i < 4;i++)
      {
        EEPROM.write(i + *lastCell + 1, id[i]);
        EEPROM.commit();
      }
    saveIndex(*lastCell+4); 
    readIndex(lastCell);
    lable2:
    displayLine(1, 0,(char*)"ADD COMPLETELY");
    clearLine(1);
    delay(1000);
    changeID_Menu(1);
  }
}

/*
*Function: removeID
*Description: remove id 
*Input:
*   lastCell: pointer points to index used last cell
*Output: None
*/
void removeID(uint16_t *lastCell)
{
  label1:
  char key;
  uint8_t id[5] ="";
  uint16_t j = 11;     // vi tri luu gia tri dau tien cua UID
  displayLine(0, 0,(char*)"SCAN THE ID CARD");
  displayLine(1, 1,(char*)"NEED TO REMOVE");
  while(id[0] == 0 && id[1] == 0 && id[2] == 0 && id[3] == 0 && key != '*' && key !='A')
  {
    readIdCard(id);
    key = keypad.getKey(); 
  }
  if(key == '*') {
    g_choiceChangeID = 2;
    longjmp(buf5,1);
  }
  else if(key =='A')
  {
    turn_On_OFF();
    longjmp(buf3,1);
  }
  uint16_t index = CompareID(id,*lastCell);
  if(index== 0)
  {
    displayLine(0, 0,(char*)"ID DOESN'T EXIST");
    clearLine(1);
    delay(1500);
    goto label1;
  }
  else if(index == 7)
  {
    displayLine(2, 0,(char*)"CAN'T REMOVE");
    displayLine(2, 1,(char*)" MASTER CARD");
    delay(1500); 
    goto label1; 
  }
  else
  {
    for(int i =0; i < 4;i++)
    {
        EEPROM.write(i+index,255);
        EEPROM.commit();
    }
    if((*lastCell - index + 1) == 4) 
    {
      *lastCell -= 4;
      saveIndex(*lastCell);
      readIndex(lastCell);
    }
    displayLine(4, 0,(char*)"REMOVED");
    clearLine(1);
    delay(1500);
    longjmp(buf5,1);
  }  
}

/*
*Function: choose_changeID
*Description: Execute option which user choose in change id menu
*Input:
*   choice: the choice option which user choose
*   lastCell: pointer points to index used last cell
*Output: None
*/
void choose_changeID(uint8_t choice,uint16_t *lastCell)
{
  switch(choice)
  {
    case 1: addID(lastCell);
            break;
    case 2: removeID(lastCell);
            break; 
  }
}

/*
*Function: changeIDCARD
*Description: change id card
*Input:
*   lastCell: pointer points to index used last cell
*Output: None
*/
void changeIDCARD(uint16_t *lastCell)
{
  changeID_Menu(1);
  char key;
  while(key != '*' && key != 'A')
  {
    key = keypad.getKey(); 
    /*Execute change id menu function*/
    Handle_Key(key,&changeID_Menu,&choose_changeID,g_choiceChangeID,lastCell);
  }
  g_choiceChangeID =1;
  if(key == '*') 
  {
    master_Menu(1);
  }
  else if(key =='A') longjmp(buf3,1);
}
/*
*Function: checkvantay
*Description: check fingerprint if it existed or not
*Input:
*   p: 
*Output: None
*/
int checkvantay(uint8_t p) {
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  return finger.fingerID;
}
/*Don't use/
/*
*Function: getFingerprintID
*Description: get fingerprint id
*Input:None
*Output: None
*/
uint8_t getFingerprintID() { 
  /*get image of fingerprint and assign result to p */
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
    /*get image successfully */
      Serial.println("Image taken");
      break;
    /*don't find fingerprint*/
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    /*Error transmit*/
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    /*Error take image*/
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
}
/*
*Function: getFingerprint
*Description: get fingerprint id
*Input:None
*Output: None
*/
uint8_t getFingerprint() {
  int p = -1;
  finger.getTemplateCount();
  uint8_t numFinger;
  numFinger=finger.templateCount;
  /*Wait user's fingerprint*/
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
  }
  }
/*confirm fingerprint*/
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
again:
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("PUT OUT");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  p = -1;
  displayLine(1, 0,(char*)"CONFIRM AGAIN");
  clearLine(1);
  delay(2000);
  /*Wait user's fingerprint*/
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  Serial.print("da tao room cho ");
  Serial.println(numFinger);
/*generate model for chosen id*/
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    //return p;
    goto again;
  } else {
    Serial.println("Unknown error");
    return p;
  }

/*Save id into model*/
  p = finger.storeModel(++numFinger);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    flag = 1;
    return flag;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}
/*
*Function: delectFingerID
*Description: Remove fingerprint id
*Input:
*   id: fingerprint id which need to be removed
*Output: None
*/
uint8_t delectFingerID(uint8_t id) {
  uint8_t p = -1;
  p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.print("Unknown error: 0x");
    Serial.println(p, HEX);
    return p;
  }
}

void setup() 
{
  Serial.begin(9600);
  finger.begin(57600);
  lcd.init();
  lcd.clear();
  lcd.noBacklight();
  EEPROM.begin(EEPROM_SIZE);
  SPI.begin();    
  mfrc522.PCD_Init();
  pinMode(relay,OUTPUT);
  readpass(pass);
  readMasterID(masterId);
  readIndex(&lastCell);
  Serial.println(pass);
  Serial.println(lastCell);
  Serial.print("SL Van tay da luu:");
  finger.getTemplateCount();
  numFinger = finger.templateCount;
  Serial.println(numFinger);
}
void loop() {
  int again3;
  again3 = setjmp(buf3);
  char key = keypad.getKey();
  /*Execute main menu function*/
  Handle_Key(key,&main_Menu,&choose_MainMenu,g_choiceMainMenu,&lastCell);
}
