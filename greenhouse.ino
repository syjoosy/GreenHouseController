//
// LCD
//
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  

//
// BME280
//

#include <GyverBME280.h>                      
GyverBME280 bme;                              

//
// DS3231
//

#include <microDS3231.h>
MicroDS3231 rtc;

//
// BUTTON
//

#define UP_BTN    13
#define DOWN_BTN  12
#define OK_BTN    14
#define BACK_BTN  27

String mainMenu[3] = {"Light", "Window", "Settings"};
String lightMenu[3] = {"Light1", "Light2", "Light3"};
String windowMenu[3] = {"Window1", "Window2", "Window3"};
String settingsMenu[3] = {"StandBy", "Settings2", "Settings3"};
 

int pointer = 0;


void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(5,1);
  lcd.print("GREENHOUSE");
  lcd.setCursor(16,3);
  lcd.print("v0.1");
  delay(2000);
  loadClock();
  bool error = false;
  if (!bme.begin() || !rtc.begin())
  {
    printDiagnostic();
  }

  //rtc.begin();
  delay(2000); 
  lcd.clear ();
  
  pinMode(UP_BTN, INPUT);
  pinMode(DOWN_BTN, INPUT);
  pinMode(OK_BTN, INPUT);
  pinMode(BACK_BTN, INPUT);

}

void printDiagnostic()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  if (bme.begin())
    lcd.print("BME280 - OK");
  else
    lcd.print("BME280 - ERROR");
  lcd.setCursor(0, 1);
  if (rtc.begin())
    lcd.print("DS3231 - OK");
  else
    lcd.print("DS3231 - ERROR");

  
  
}

int standByPeriod = 10000; // время через которое появится экран ожидания в мс
uint32_t standByTimer;         // переменная таймера

boolean displayStandBy = false;
uint32_t updateValuesTimer;         // переменная таймера
int updatePeriod = 2000; // время через которое появится экран ожидания в мс

String temperature = "";
String humidity = "";

void loop() 
{
  keyboardLogic();
  ScreenLogic();
  

}

void ScreenLogic()
{
  if (millis() - standByTimer >= standByPeriod) 
  {
    //standByTimer = millis();
    if (!displayStandBy)
    {
      lcd.clear();
      displayStandBy = true;
    }
    if (millis() - updateValuesTimer >= updatePeriod) 
    {
      updateValuesTimer = millis();
      GetBmeValues();
      drawStandby();
    }
  }
  else
  {
    if (displayStandBy)
    {
      lcd.clear();
      displayStandBy = false;
    }
    menuLogic();
    if (millis() - updateValuesTimer >= updatePeriod) 
    {
      updateValuesTimer = millis();
      DrawTime();
      DrawBmeValues();
    }
  }
}


boolean pointerChange = true; 

boolean okButton = false;
boolean backButton = false;

int menuPointer = 0;
String path = "";
void menuLogic()
{
  if (okButton)
  {
    switch (pointer)
    {
      case 0:
        path.concat(0);
        pointer = 0;
        drawMenu(lightMenu, sizeof(lightMenu));
        break;
      case 1:
        path.concat(1);
        pointer = 0;
        drawMenu(windowMenu, sizeof(windowMenu));
        break;
      case 2:
        path.concat(2);
        pointer = 0;
        drawMenu(settingsMenu, sizeof(settingsMenu));
        break;
    }

    menuPointer++;
    okButton = false;

  } 
  if (backButton)
  {
    path.remove(path.length() - 1);
    pointer = 0;
    menuPointer--;
    if (menuPointer < 0)
      menuPointer = 0;
    backButton = false;
  }
  if (menuPointer == 0)
  {
    drawMenu(mainMenu, sizeof(mainMenu));
  }
  else if (pointerChange)
  {
    if (path == "0")
        drawMenu(lightMenu, sizeof(lightMenu));
    else if (path == "1")
        drawMenu(windowMenu, sizeof(windowMenu));
    else if (path == "2")
        drawMenu(settingsMenu, sizeof(settingsMenu));
    
  }
  //Serial.println(path);
}


boolean standByWithSecons = false;
void drawStandby()
{
  if (standByWithSecons)
  {
    int temp_temperature = temperature.toInt();
    drawDig(temp_temperature / 10, 1, 1);
    drawDig(temp_temperature % 10, 5, 1);
    lcd.print(temperature[3]);
    
    int temp_humidity = humidity.toInt();
    drawDig(temp_humidity / 10, 11, 1);
    drawDig(temp_humidity % 10, 15, 1);
    lcd.print(humidity[3]);
  }
  else
  {
    int temp_temperature = temperature.toInt();
    drawDig(temp_temperature / 10, 1, 1);
    drawDig(temp_temperature % 10, 5, 1);
    lcd.setCursor(8,1);
    lcd.print(char(223));
    
    int temp_humidity = humidity.toInt();
    drawDig(temp_humidity / 10, 11, 1);
    drawDig(temp_humidity % 10, 15, 1);
    lcd.setCursor(18,1);
    lcd.print("%");
  }
}

void drawMenu(String *menu, int size)
{
  size = size / sizeof(String);
  if (pointerChange || okButton)
  {
  for (int i = 0; i < size; i++)
  {
    lcd.setCursor(0, i);
    lcd.print("               ");
    lcd.setCursor(0, i);
    if (pointer == i)
    {
      lcd.print("<");
      lcd.print(menu[i]);
      lcd.print(">");
    }
    else
    {
      lcd.print(menu[i]);
    }
  }
  pointerChange = false;
  }
}

void keyboardLogic()
{
  if (digitalRead(UP_BTN))
  {
    pointer--;
    if (pointer < 0)
      pointer = 0;
    pointerChange = true;
    standByTimer = millis();
  }
  if (digitalRead(DOWN_BTN))
  {
    pointer++;
    pointerChange = true;
    standByTimer = millis();
  }
  if (digitalRead(OK_BTN))
  {
    okButton = true;
    standByTimer = millis();
  }
  if (digitalRead(BACK_BTN))
  {
    backButton = true;
    pointerChange = true;
    standByTimer = millis();

  }
}

uint8_t LT[8] = {0b11111,
                 0b11111,
                 0b11111,
                 0b11111,
                 0b11111,
                 0b11111,
                 0b11111,
                 0b11111};
uint8_t UB[8] = {0b11111,
                 0b11111,
                 0b11111,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b00000};
uint8_t LB[8] = {0b00000,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b11111,
                 0b11111,
                 0b11111};
uint8_t LR[8] = {0b11111,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b00000};
uint8_t UMB[8] = {0b11111,
                 0b11111,
                 0b11111,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b11111,
                 0b11111};
uint8_t LMB[8] = {0b11111,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b00000,
                 0b11111,
                 0b11111,
                 0b11111};

void drawDig(byte dig, byte x, byte y) {
  switch (dig) {
    case 0:
      lcd.setCursor(x, y); // set cursor to column 0, line 0 (first row)
      lcd.write(0);  // call each segment to create
      lcd.write(1);  // top half of the number
      lcd.write(0);
      lcd.setCursor(x, y + 1); // set cursor to colum 0, line 1 (second row)
      lcd.write(0);  // call each segment to create
      lcd.write(4);  // bottom half of the number
      lcd.write(0);
      break;
    case 1:
      lcd.setCursor(x, y);
      lcd.write(32);
      lcd.write(1);
      lcd.write(0);
      lcd.setCursor(x, y + 1);
      lcd.write(32);
      lcd.write(32);
      lcd.write(0);
      break;
    case 2:
      lcd.setCursor(x, y);
      lcd.write(6);
      lcd.write(6);
      lcd.write(0);
      lcd.setCursor(x, y + 1);
      lcd.write(0);
      lcd.write(7);
      lcd.write(7);
      break;
    case 3:
      lcd.setCursor(x, y);
      lcd.write(6);
      lcd.write(6);
      lcd.write(0);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(0);
      break;
    case 4:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(4);
      lcd.write(0);
      lcd.setCursor(x, y + 1);
      lcd.write(5);
      lcd.write(5);
      lcd.write(0);
      break;
    case 5:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(0);
      break;
    case 6:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(0);
      lcd.write(7);
      lcd.write(0);
      break;
    case 7:
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(1);
      lcd.write(0);
      lcd.setCursor(x, y + 1);
      lcd.write(32);
      lcd.write(0);
      lcd.write(32);
      break;
    case 8:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(0);
      lcd.setCursor(x, y + 1);
      lcd.write(0);
      lcd.write(7);
      lcd.write(0);
      break;
    case 9:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(0);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(0);
      break;
    case 10:
      lcd.setCursor(x, y);
      lcd.write(32);
      lcd.write(32);
      lcd.write(32);
      lcd.setCursor(x, y + 1);
      lcd.write(32);
      lcd.write(32);
      lcd.write(32);
      break;
  }
}

void loadClock() {
  lcd.createChar(0, LT);
  lcd.createChar(1, UB);
  //lcd.createChar(2, FG);
  //lcd.createChar(3, FJ);
  lcd.createChar(4, LB);
  lcd.createChar(5, LR);
  lcd.createChar(6, UMB);
  lcd.createChar(7, LMB);
}

void DrawTime()
{
  lcd.setCursor(15, 0);
  if (rtc.getHours() < 10)
  {
    lcd.print(0);
    lcd.print(rtc.getHours());
  }
  else
  {
    lcd.print(rtc.getHours());
  }
    lcd.print(":");

  if (rtc.getMinutes() < 10)
  {
    lcd.print(0);
    lcd.print(rtc.getMinutes());
  }
  else
  {
    lcd.print(rtc.getMinutes());
  }
}

void GetBmeValues()
{
  temperature = String(bme.readTemperature(), 1);
  humidity = String(bme.readHumidity(), 1);
}

void DrawBmeValues()
{
  GetBmeValues();
  lcd.setCursor(15, 1);
  lcd.print(temperature);
  lcd.print(char(223));
  
  lcd.setCursor(15, 2);
  lcd.print(humidity);
  lcd.print("%");
}
