#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <string.h>
#include <EEPROM.h>
#include <SPI.h>      
#include <MFRC522.h>
#include <setjmp.h>
#include <Adafruit_Fingerprint.h> // thư viện vân tay
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
//so byte eeprom luu
#define EEPROM_SIZE 200
uint8_t id;
uint8_t numFinger;

#define relay 2
jmp_buf buf2;  // return main menu
jmp_buf buf3;  // return dau chuong trinh khi nhan A
jmp_buf buf4;  // return master menu
jmp_buf buf5; //return  menu change id card
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

byte pin_rows[ROW_NUM]      = {12,14,32,33}; 
byte pin_column[COLUMN_NUM] = {25,26,27,4};   
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
//Setup lcd
LiquidCrystal_I2C lcd(0x27,16,2);
//global variable
uint8_t g_choiceMainMenu =1;
uint8_t g_choiceMasterMenu =1;
uint8_t g_choiceChangeID =1;
uint8_t g_choiceFingerMenu = 1;
bool g_state = 0;                 // trang thai lcd | 0: off ; 1: on
uint8_t flag = 0;
char pass[7]="";               // pass doc tu eeprom
uint16_t lastCell;                // luu vi tri o nho chua du lieu cuoi cung
//Setup RFID
#define RST  13
#define SDA  5
MFRC522 mfrc522(SDA, RST);
uint8_t masterId[5]="";            // Luu id cua the master
void main_Menu(char);
void choose_MainMenu(uint8_t) ;

//-------------------------HAM SU DUNG CHUNG------------------------------------
void turn_On_OFF()              //bat tat lcd //r
{
  g_state =!g_state;
  Serial.println(g_state);
  int again2;
  g_choiceMainMenu =1;
  again2 = setjmp(buf2);
  if(g_state == 1)
  {
    // //turn on lcd
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
void clearLine(uint8_t line) //r
{
  if(line == 0) lcd.setCursor(0,0);
  else lcd.setCursor(0,1);
  lcd.print("                ");
}
void displayLine(uint8_t index,uint8_t line,char* inform) //r
{
  clearLine(line);
  lcd.setCursor(index,line);    
  lcd.print(inform);  
}
void delect(int i)              // xoa ki tu khi nhap mat khau//r
{
  if(i>=0 && i<=6)
  {
    lcd.setCursor(i+4,1);
    lcd.print(" ");
    Serial.print(i);
    lcd.setCursor(i+4,1);
  }
}
void savepass(char a[])      // luu mat khau vao eeprom//r
{
   for(int i = 0 ; i<6;i++)
  {
    pass[i] = a[i];
    EEPROM.write(i+1,a[i]);
    EEPROM.commit();
  }
}
void readpass(char a[])         // doc mat khau tu eeprom//r
{
  for(int i =0 ; i<6;i++)
  {
    a[i] = EEPROM.read(i+1);
  } 
}
void Handle_Key(char key,void(*typeMenu)(char),void(*choose_menu)(uint8_t,uint16_t*),uint8_t choice,uint16_t* lastCell)
{
  if(key == 'A')
  {
    turn_On_OFF();
  } 
  else if((key == 'B'&& g_state == 1)||(key == 'C'&& g_state == 1))typeMenu(key);
  else if(key =='D'&& g_state == 1)choose_menu(choice, lastCell);
}
void readIndex(uint16_t *lastCell) //r
{
  *lastCell = EEPROM.read(0);
  Serial.print("lastcell: ");
  Serial.println(*lastCell);
}
void saveIndex(uint16_t lastCell) //r
{
  EEPROM.write(0,lastCell);
  EEPROM.commit();
}
//-------------------------HAM MAIN MENU------------------------------------------
void main_Menu(char key)             //mainscreen
{
  if(key == 'B')g_choiceMainMenu -=1;
  else if (key == 'C')g_choiceMainMenu += 1;
  if(g_choiceMainMenu == 1)
  {
    displayLine(1,0,(char*)"CHOOSE A OPTION");
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
void choose_MainMenu(uint8_t choice,uint16_t *lastCell)     //choose function
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
void open_cabinet()             // mo tu
{
  digitalWrite(relay,HIGH);
}
void close_cabinet()            // dong tu
{
  digitalWrite(relay,LOW);
}
void readIdCard(uint8_t id[])//r
{
  if ( ! mfrc522.PICC_IsNewCardPresent()) return; //Kiem tra xem co the quet hay khong
  if ( ! mfrc522.PICC_ReadCardSerial()) return;  //Kiem tra xem doc the co thanh cong hay khong
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
void readMasterID(uint8_t id[])//r
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
void setMasterID(uint8_t id[])//r
{
  for(int i =0; i < 4;i++)
  {
    EEPROM.write(i+7,id[i]);
    EEPROM.commit();
  }
}
void enterpass()                //Chuc nang 1: nhap mat khau//r
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
      longjmp(buf2,1);
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
void scanID(uint16_t *lastCell)                   //Chuc nang 2: scan id card//r
{
  lable:
  char key;
  uint8_t idCard[5] ="";
  displayLine(1, 0, (char*)"PUT IN ID CARD");
  clearLine(1);
  lcd.setCursor(5,1);
  while(idCard[0]==0 && idCard[1]==0 && idCard[2]==0 && idCard[3]==0 && key != '*' && key !='A')
  {
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
void scanfinger()                   //Chuc nang 3: quet van tay
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
//-----------------------HAM MASTER MENU-----------------------------------------
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
void choose_MasterMenu(uint8_t choice,uint16_t *lastCell)     //choose function
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
void changePass() //r
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
//.......................HAM CHANGE FINGERPRINT..................................
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
        layvantay();
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
         xoaid(id);         
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
void changeFinger(uint16_t *lastCell)
{
  changeFingerMenu(1);
  char key;
  while(key != '*' && key != 'A')
  {
    key = keypad.getKey(); 
    Handle_Key(key,&changeFingerMenu,&chooseFinger,g_choiceFingerMenu,lastCell);
  }
  g_choiceFingerMenu =1;
  if(key == '*') 
  {
    master_Menu(1);
  }
  else if(key =='A') longjmp(buf3,1);
}
//-----------------------HAM CHANGE ID CARD--------------------------------------
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
uint16_t CompareID(uint8_t idCard[],uint16_t lastCell)
{
  uint8_t id[5] ="";
  uint16_t j = 7;     // vi tri luu gia tri dau tien cua UID
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
void addID(uint16_t *lastCell)
{
  label1:
  char key;
  uint8_t id[5] ="";
  uint16_t j = 11;     // vi tri luu gia tri dau tien cua UID
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
        while(dem < 4 && EEPROM.read(dem + j) == 255) dem++;  //kiem tra 4 o trong lien tiep
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
    displayLine(1, 0,(char*)"ID DOESN'T EXIST");
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
void changeIDCARD(uint16_t *lastCell)
{
  changeID_Menu(1);
  char key;
  while(key != '*' && key != 'A')
  {
    key = keypad.getKey(); 
    Handle_Key(key,&changeID_Menu,&choose_changeID,g_choiceChangeID,lastCell);
  }
  g_choiceChangeID =1;
  if(key == '*') 
  {
    master_Menu(1);
  }
  else if(key =='A') longjmp(buf3,1);
}
//-------------------------------------------------------------//
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

uint8_t getFingerprintID() { // Khai báo hàm getFingerprintID() trả về kiểu dữ liệu uint8_t.
  uint8_t p = finger.getImage(); // lấy hình ảnh của vân tay và gán kết quả trả về cho biến p.
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");// lấy hình ảnh vân tay thành công
      break;
    case FINGERPRINT_NOFINGER: // không phát hiện được vân tay
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR: // lỗi truyền
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL: // lỗi chụp hình ảnh
      Serial.println("Imaging error");
      return p;
    default: // có lỗi không xác định
      Serial.println("Unknown error");
      return p;
  }
}
//lay van ta
uint8_t layvantay() {
  //uint8_t flag = 0;
  int p = -1;
  finger.getTemplateCount();
  uint8_t numFinger;
  numFinger=finger.templateCount;
  //doi van tay cua user
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
  //xac nhan van tay
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
  //doi van tay cua user
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
  //tao model cho ID da chon
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

  // luu ID vao model
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
// xoa van tay
uint8_t xoaid(uint8_t id) {
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
//-----------------------HAM CHINH-----------------------------------------------
void setup() 
{
  Serial2.begin(9600);
  finger.begin(57600);
  lcd.init();
  lcd.clear();
  lcd.noBacklight();
  Serial.begin(115200);
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
  Serial.println(numFinger); //In số lượng mẫu vân tay đã lưu trữ trên cảm biến
}
void loop() {
  int again3;
  again3 = setjmp(buf3);
  char key = keypad.getKey();
  Handle_Key(key,&main_Menu,&choose_MainMenu,g_choiceMainMenu,&lastCell);
}
