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
uint8_t choiceMainMenu =1;
uint8_t choiceMasterMenu =1;
uint8_t choiceChangeID =1;
uint8_t choiceFingerMenu = 1;
bool state = 0;                 // trang thai lcd | 0: off ; 1: on
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
void turn_On_OFF()              //bat tat lcd 
{
  state =!state;
  Serial.println(state);
  int again2;
  choiceMainMenu =1;
  again2 = setjmp(buf2);
  if(state == 1)
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

void thongBao(char* thongbao)   // Hien thi thong bao
{
  lcd.clear();
 lcd.setCursor(1,0)
  lcd.print(thongbao);
}
void delect(int i)              // xoa ki tu khi nhap mat khau
{
  if(i>=0 && i<=6)
  {
    lcd.setCursor(i+4,1);
    lcd.print(" ");
    Serial.print(i);
    lcd.setCursor(i+4,1);
  }
}
void savepass(char a[])      // luu mat khau vao eeprom
{
   for(int i = 0 ; i<6;i++)
  {
    pass[i] = a[i];
    EEPROM.write(i+1,a[i]);
    EEPROM.commit();
  }
}
void readpass(char a[])         // doc mat khau tu eeprom
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
  else if((key == 'B'&& state == 1)||(key == 'C'&& state == 1))typeMenu(key);
  else if(key =='D'&& state == 1)choose_menu(choice, lastCell);
}
void readIndex(uint16_t *lastCell)
{
  *lastCell = EEPROM.read(0);
  Serial.print("lastcell: ");
  Serial.println(*lastCell);
}
void saveIndex(uint16_t lastCell)
{
  EEPROM.write(0,lastCell);
  EEPROM.commit();
}
//-------------------------HAM MAIN MENU------------------------------------------
void main_Menu(char key)             //mainscreen
{
  if(key == 'B')choiceMainMenu -=1;
  else if (key == 'C')choiceMainMenu += 1;
  if(choiceMainMenu == 1)
  {
    lcd.clear();
    lcd.setCursor(1,0);   
    lcd.print("CHOOSE A OPTION");
    lcd.setCursor(0,1);  
    lcd.print("> ENTER PASSWORD");
  }
  else if(choiceMainMenu == 2)
  {
    lcd.clear();
    lcd.setCursor(0,0);   
    lcd.print("> SCAN ID CARD");
    lcd.setCursor(0,1);  
    lcd.print("  FINGERPRINT");
  }
  else if(choiceMainMenu == 3)
  {
    lcd.clear();
    lcd.setCursor(0,0);   
    lcd.print("  SCAN ID CARD");
    lcd.setCursor(0,1);  
    lcd.print("> FINGERPRINT");
  }else if(choiceMainMenu < 1)choiceMainMenu =1;
  else choiceMainMenu =3;

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
void read_Id_Card(uint8_t id[])
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
void Read_MasterID(uint8_t id[])
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
void Set_MasterID(uint8_t id[])
{
  for(int i =0; i < 4;i++)
  {
    EEPROM.write(i+7,id[i]);
    EEPROM.commit();
  }
}
void enterpass()                //Chuc nang 1: nhap mat khau
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
    thongBao((char*)"CABINET IS OPEN");
    delay(3000);
    close_cabinet();
    longjmp(buf2,1);
  }else
  {
    thongBao((char*)"WRONG PASSWORD");
    delay(1000);
    goto again;
  }

}
void scanID(uint16_t *lastCell)                   //Chuc nang 2: scan id card
{
  lable:
  char key;
  uint8_t idCard[5] ="";
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("PUT IN ID CARD");
  lcd.setCursor(5,1);
  while(idCard[0]==0 && idCard[1]==0 && idCard[2]==0 && idCard[3]==0 && key != '*' && key !='A')
  {
    read_Id_Card(idCard);
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
    thongBao((char*)"WRONG ID CARD");
    delay(1500);
    goto lable;    
  }
  else if(index == 7)
  {
    open_cabinet();
    thongBao((char*)"CABINET IS OPEN");
    delay(3000);
    close_cabinet();
    master_Menu(1);
    while(key != '*' && key != 'A')
    {
      key = keypad.getKey(); 
      Handle_Key(key,&master_Menu,&choose_MasterMenu,choiceMasterMenu,lastCell);
    }
    choiceMasterMenu = 1;
    if(key == '*') longjmp(buf2,1);
    else if(key =='A') longjmp(buf3,1);
  }
  else
  {    
    open_cabinet();
    thongBao((char*)"CABINET IS OPEN");
    delay(3000);
    close_cabinet();
    longjmp(buf2,1);
  }
}
void scanfinger()                   //Chuc nang 3: quet van tay
{
  char key;
  int id = -1;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SCAN FINGERPRINT");
  while(key != '*' && key != 'A'&& id == -1)
  { 
    uint8_t p = finger.getImage();
    key = keypad.getKey(); 
    if (p == FINGERPRINT_OK){
       key = keypad.getKey(); 
      id = checkvantay(p);
      if(id==-1)
      {
        thongBao((char*)"WRONG FGPRINT");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SCAN FINGERPRINT");
      }
    }
  }
  if(key == '*') longjmp(buf2,1);
  else if(key =='A') longjmp(buf3,1);
  open_cabinet();
  thongBao((char*)"CABINET IS OPEN");
  delay(3000);
  close_cabinet();
  longjmp(buf2,1);

}
//-----------------------HAM MASTER MENU-----------------------------------------
void master_Menu(char key)    
{
  if(key == 'B')choiceMasterMenu -=1;
  else if (key == 'C')choiceMasterMenu += 1;
  int again4;
  again4 = setjmp(buf4);
  if(choiceMasterMenu == 1)
  {
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("SECURITY");
    lcd.setCursor(0, 1);
    lcd.print(">CHANGE PASSWORD");
  }
  else if(choiceMasterMenu == 2)
  {
    lcd.clear();
    lcd.setCursor(0,0);   
    lcd.print(">CHANGE ID CARD");
    lcd.setCursor(0,1);  
    lcd.print(" CHANGE FGPRINT");
  }
  else if(choiceMasterMenu == 3)
  {
    lcd.clear();
    lcd.setCursor(0,0);   
    lcd.print(" CHANGE ID CARD");
    lcd.setCursor(0,1);  
    lcd.print(">CHANGE FGPRINT");
  }else if(choiceMasterMenu < 1)choiceMasterMenu =1;
  else choiceMasterMenu =3;
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
void changePass()
{
  char a[7] ="" ;
  int i =0;
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("NEW PASSWORD");
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
  thongBao((char*)"PASSWORD CHANGED");
  delay(2000);
  lcd.noBlink();
  master_Menu(1); 
}
//.......................HAM CHANGE FINGERPRINT..................................
void changeFingerMenu(char key)
{
  if(key == 'B')choiceFingerMenu -=1;
  else if (key == 'C')choiceFingerMenu += 1;
int again6;
again6 = setjmp(buf6);  
  if(choiceFingerMenu == 1)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("> ADD FGPRINT");
    lcd.setCursor(0, 1);
    lcd.print("  REMOVE FGPRINT");
  }
  else if(choiceFingerMenu == 2)
  {
    lcd.clear();
    lcd.setCursor(0,0);   
    lcd.print("  ADD FGPRINT");
    lcd.setCursor(0, 1);
    lcd.print("> REMOVE FGPRINT");
  }else if(choiceFingerMenu < 1)choiceFingerMenu =1;
  else choiceFingerMenu =2;
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
  uint8_t flag = 0;
  int id=-1;  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SCAN FINGERPRINT");
  char key;
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
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("FGPRINT EXISTS");
          delay(1000);
          goto again;          
      }
    }
  }
  if(key == '*') 
  {
    choiceFingerMenu = 1;
    longjmp(buf6,1);
  }
  else if(key =='A') longjmp(buf3,1);
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("ADD COMPLETELY");
  delay(1000);
  changeFingerMenu(1);  

}
void removeFinger()
{  
   again:
  uint8_t flag = 0;
  int id=-1;  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SCAN FINGERPRINT");
  lcd.setCursor(1, 1);
  lcd.print("NEED TO DELECT");
  char key;
  while(key != '*' && key != 'A'&& id == -1 && flag == 0)
  { 
    uint8_t p = finger.getImage();
    key = keypad.getKey(); 
    if (p == FINGERPRINT_OK){
      id = checkvantay(p);
      if(id == -1 ){
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print("FINGERPRINT");
          lcd.setCursor(1, 1);
          lcd.print("DOESN'T EXIST");
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
    choiceFingerMenu = 1;
    longjmp(buf6,1);
  }
  else if(key =='A') longjmp(buf3,1);
  lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("REMOVED");
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
    Handle_Key(key,&changeFingerMenu,&chooseFinger,choiceFingerMenu,lastCell);
  }
  choiceFingerMenu =1;
  if(key == '*') 
  {
    master_Menu(1);
  }
  else if(key =='A') longjmp(buf3,1);
}
//-----------------------HAM CHANGE ID CARD--------------------------------------
void changeID_Menu(char key)
{
  if(key == 'B')choiceChangeID -=1;
  else if (key == 'C')choiceChangeID += 1;
  int again5;
  again5 = setjmp(buf5);
  if(choiceChangeID == 1)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("> ADD ID CARD");
    lcd.setCursor(0, 1);
    lcd.print("  REMOVE ID CARD");
  }
  else if(choiceChangeID == 2)
  {
    lcd.clear();
    lcd.setCursor(0,0);   
    lcd.print("  ADD ID CARD");
    lcd.setCursor(0, 1);
    lcd.print("> REMOVE ID CARD");
  }else if(choiceChangeID < 1)choiceChangeID =1;
  else choiceChangeID =2;
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
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SCAN NEW ID CARD");
  while(id[0]==0 && id[1]==0 && id[2]==0 && id[3]==0 && key != '*' && key !='A')
  {
    read_Id_Card(id);
    key = keypad.getKey(); 
  }
  if(key == '*')
  {
    choiceChangeID = 1;
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
    thongBao((char*)"ID EXISTES");
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
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("ADD COMPLETELY");
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
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SCAN THE ID CARD");
  lcd.setCursor(1,1);
  lcd.print("NEED TO REMOVE");
  while(id[0] == 0 && id[1] == 0 && id[2] == 0 && id[3] == 0 && key != '*' && key !='A')
  {
    read_Id_Card(id);
    key = keypad.getKey(); 
  }
  if(key == '*') {
    choiceChangeID = 2;
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
    thongBao((char*)"ID DOESN'T EXIST");
    delay(1500);
    goto label1;
  }
  else if(index == 7)
  {
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("CAN'T REMOVE");
    lcd.setCursor(2,1);
    lcd.print(" MASTER CARD");
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
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("REMOVED");
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
    Handle_Key(key,&changeID_Menu,&choose_changeID,choiceChangeID,lastCell);
  }
  choiceChangeID =1;
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
  uint8_t flag = 0;
  int p = -1;
  finger.getTemplateCount();
  uint8_t numFinger;
  numFinger=finger.templateCount;
  //doi van tay cua user
 
  // switch (p) {
  //   case FINGERPRINT_OK:
  //     Serial.println("Image taken");
  //     break;
  //   case FINGERPRINT_NOFINGER:
  //     Serial.println(".");
  //     break;
  //   case FINGERPRINT_PACKETRECIEVEERR:
  //     Serial.println("Communication error");
  //     break;
  //   case FINGERPRINT_IMAGEFAIL:
  //     Serial.println("Imaging error");
  //     break;
  //   default:
  //     Serial.println("Unknown error");
  //     break;
  // }
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

  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("PUT OUT");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  p = -1;
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("CONFIRM AGAIN");
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
    return p;
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
  Read_MasterID(masterId);
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
  Handle_Key(key,&main_Menu,&choose_MainMenu,choiceMainMenu,&lastCell);
}
