//
// LCD
//
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);  

//
// BME280
//

//#include <GyverBME280.h>                      
//GyverBME280 bme;                              

//
// DS3231
//

//#include <microDS3231.h>
//MicroDS3231 rtc;

//
// BUTTON
//

typedef struct { 
  String name;
  bool state;
} Dictionary;

typedef struct { 
  bool state;
  String temperature;
  String humidity;
  String time;
} WindowInfo;

#define UP_BTN    13
#define DOWN_BTN  12
#define OK_BTN    14
#define BACK_BTN  27

typedef struct 
{
  int percent1;
  int percent2;
  int percent3;
  int percent4;
  int percent5;
  int percent6;
  int percent7;
  int percent8; 
} Soil;


String mainMenu[4] = {"Soil", 
                      "Windows", 
                      "Statistics", 
                      "Settings"};

// const Dictionary test[] {
//     {"Soil", {"SOIL1", "SOIL2"}}
//     {"Window", "SPANISH", "Ajustes"},
//     {"Light", "FRENCH", "Paramètres"},
//     {"Light", "FRENCH", "Paramètres"}
// };

Dictionary windows[]
{
  {"Window1", true},
  {"Window2", false}
};

WindowInfo windowInfo[]
{
  {true, "20<T<45", "15<H<40", "08:00<T<20:00"},
  {false, "20<T<45", "15<H<40", "10:00<T<15:00"}
};
//Dictionary<String, String> MyDict;

String statisticsMenu[3] = {"Statistic1", "Statistic2", "Statistic3"};
String windowMenu[3] = {"Window1", "Window2", "Window3"};
String settingsMenu[3] = {"StandBy", "Settings2", "Settings3"};
String soilMenu[8] = {"15", "27", "55", "71","49", "54","34", "82"};

int pointer = 0;


void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(5,1);
  lcd.print("GREENHOUSE");
  lcd.setCursor(16,3);
  lcd.print("v0.1");
  //delay(2000);
  loadClock();
  bool error = false;

  //if (!bme.begin() || !rtc.begin())
  //{
  //  printDiagnostic();
  //}

  //rtc.begin();
  //delay(2000); 
  lcd.clear ();
  
  pinMode(UP_BTN, INPUT);
  pinMode(DOWN_BTN, INPUT);
  pinMode(OK_BTN, INPUT);
  pinMode(BACK_BTN, INPUT);

}

// void printDiagnostic()
// {
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   if (bme.begin())
//     lcd.print("BME280 - OK");
//   else
//     lcd.print("BME280 - ERROR");
//   lcd.setCursor(0, 1);
//   if (rtc.begin())
//     lcd.print("DS3231 - OK");
//   else
//     lcd.print("DS3231 - ERROR");
// }

int standByPeriod = 60000; // время через которое появится экран ожидания в мс
uint32_t standByTimer;         // переменная таймера

boolean displayStandBy = false;
uint32_t updateValuesTimer;         // переменная таймера
int updatePeriod = 2000; // время через которое появится экран ожидания в мс

String temperature = "35.2";
String humidity = "49.7";

int hours = 15;
int minutes = 23;


void loop() 
{
  keyboardLogic();
  ScreenLogic();
}

String path = "";
bool drawWindowState = false;
void ScreenLogic()
{
  //Serial.println(path);
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
      //GetBmeValues();
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
    if (millis() - updateValuesTimer >= updatePeriod && path == "") 
    {
      updateValuesTimer = millis();
      DrawTime();
      DrawBmeValues();
      lcd.setCursor(17, 3);
      lcd.print("   ");
    }
  }
}


boolean pointerChange = true; 

boolean okButton = false;
boolean backButton = false;

int menuPointer = 0;

void menuLogic()
{
  if (okButton && path == "")
  {
    switch (pointer)
    {
      case 0:
        path.concat(0);
        pointer = 0;
        drawSoil();
        break;
      case 1:
        path.concat(1);
        pointer = 0;
        drawWindow();
        break;
      case 2:
        path.concat(2);
        pointer = 0;
        drawMenu(statisticsMenu, sizeof(statisticsMenu));
        break;
      case 3:
        path.concat(3);
        pointer = 0;
        drawMenu(settingsMenu, sizeof(settingsMenu));
        break;
    } 
    menuPointer++;
    okButton = false;

  }
  if (okButton && path == "1")
  {
    drawWindowState = true;
    okButton = false;
    backButton = false;
  }
  if (drawWindowState && pointerChange)
  {
    menuPointer++;
    if (path.length() != 2)
      path.concat(pointer);
    //pointer = 0;
    //Serial.println("DRAW WINDOW");
    DrawWindowInfo(path);
    pointerChange = false;
    //drawWindowState = false;
  } 

  if (backButton)
  {
    path.remove(path.length() - 1);
    pointer = 0;
    menuPointer--;
    if (menuPointer < 0)
      menuPointer = 0;
    if (drawWindowState)
      drawWindowState = false;
    backButton = false;
  }
  if (menuPointer == 0)
  {
    drawMenu(mainMenu, sizeof(mainMenu));
  }
  else if (pointerChange)
  {
    if (path == "0")
        drawSoil();
    else if (path == "1")
        drawWindow();
    else if (path == "2")
        drawMenu(statisticsMenu, sizeof(statisticsMenu));
    else if (path == "3")
        drawMenu(settingsMenu, sizeof(settingsMenu));
    
  }
  //Serial.println(path);
}

int windowPointer = 0;
void DrawWindowInfo(String path)
{
  Serial.print("PATH: ");
  Serial.print(path);
  Serial.print(" LENGTH: ");
  Serial.print(path.length());

  windowPointer = path[path.length() - 1] - 48; // char to int
  Serial.print(" WIN POINTER: ");
  Serial.println(windowPointer);
  
  Serial.print(" POINTER: ");
  Serial.println(pointer);
  clearDisplay();
  lcd.setCursor(0,0);
  if (pointer == 0)
  {
    lcd.print("<");
    lcd.print(windowInfo[windowPointer].state);
    lcd.print(">");
  }
  else
  {
    lcd.print(windowInfo[windowPointer].state);
  }
  lcd.setCursor(0,1);
  if (pointer == 1)
  {
    lcd.print("<");
    lcd.print(windowInfo[windowPointer].temperature);
    lcd.print(">");
  }
  else
  {
    lcd.print(windowInfo[windowPointer].temperature);
  }
  lcd.setCursor(0,2);
  if (pointer == 2)
  {
    lcd.print("<");
    lcd.print(windowInfo[windowPointer].humidity);
    lcd.print(">");
  }
  else
  {
    lcd.print(windowInfo[windowPointer].humidity);
  }
  lcd.setCursor(0,3);
  if (pointer == 3)
  {
    lcd.print("<");
    lcd.print(windowInfo[windowPointer].time);
    lcd.print(">");
  }
  else
  {
    lcd.print(windowInfo[windowPointer].time);
  }
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

void clearDisplay()
{
  for (int i = 0; i < 4; i++)
  {
    lcd.setCursor(0, i);
    lcd.print("                    ");
  }
}


void drawWindow()
{
  clearDisplay();
  for (int i = 0; i < 2; i++)
  {
    lcd.setCursor(0, i);
    lcd.print("                    ");
    lcd.setCursor(0, i);
    if (pointer == i)
      lcd.print("<");
    lcd.print(windows[i].name);
    lcd.print(" - ");
    if (windows[i].state) 
      lcd.print("OPEN");
    else
      lcd.print("CLOSED");
    
    if (pointer == i)
      lcd.print(">");
  }
  pointerChange = false;
}

void drawSoil()
{
  for (int i = 0; i < 8; i++)
  { 
    if (i < 4)
    {
      lcd.setCursor(0, i);
      lcd.print("                    ");
      lcd.setCursor(0, i);
      lcd.print(soilMenu[i]);
      lcd.print("%");
    }
    else
    {
      lcd.setCursor(17, i-4);
      lcd.print(soilMenu[i]);
      lcd.print("%");
    }
  }
      lcd.setCursor(8, 0);
    lcd.print("SOIL");
    lcd.setCursor(6, 3);
    lcd.print("HUMIDITY");
    pointerChange = false;
}

void drawMenu(String *menu, int size)
{
  
  size = size / sizeof(String);
  if (pointerChange || okButton)
  {
  clearDisplay();
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
    pointerChange = true;
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
  //GetTime();
  lcd.setCursor(15, 0);
  if (hours < 10)
  {
    lcd.print(0);
    lcd.print(hours);
  }
  else
  {
    lcd.print(hours);
  }
    lcd.print(":");

  if (minutes < 10)
  {
    lcd.print(0);
    lcd.print(minutes);
  }
  else
  {
    lcd.print(minutes);
  }
}

// void GetTime()
// {
//   hours = rtc.getHours();
//   minutes = rtc.getMinutes();
// }

// void GetBmeValues()
// {
//   temperature = String(bme.readTemperature(), 1);
//   humidity = String(bme.readHumidity(), 1);
// }

void DrawBmeValues()
{
  //GetBmeValues();
  lcd.setCursor(15, 1);
  lcd.print(temperature);
  lcd.print(char(223));
  
  lcd.setCursor(15, 2);
  lcd.print(humidity);
  lcd.print("%");
}
