#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define TFT_CS   3
#define TFT_DC   4
#define TFT_RST  5
#define TFT_SCK  8
#define TFT_MOSI 10

#define RED_LED   6
#define GREEN_LED 7
#define TOUCH_PIN 21

SPIClass displaySPI(FSPI);
Adafruit_ST7735 tft = Adafruit_ST7735(&displaySPI, TFT_CS, TFT_DC, TFT_RST);

String currentStatus = "Booting";
String currentUser = "";
String currentAI = "Lie-Generator ready.";
int touchStatus;
int lastTouchStatus = LOW;
int LEDStatus;
int lastLEDStatus;

void drawWrappedText(const String &text, int x, int y, int maxChars, uint16_t color) {
  tft.setTextColor(color, ST77XX_BLACK);
  tft.setCursor(x, y);

  String line = "";
  int lineHeight = 10;
  int currentY = y;

  for (int i = 0; i < text.length(); i++) {
    char c = text[i];

    if (c == '\n' || line.length() >= maxChars) {
      tft.setCursor(x, currentY);
      tft.print(line);
      line = "";
      currentY += lineHeight;

      if (currentY > 118) {
        return;
      }

      if (c != '\n') {
        line += c;
      }
    } else {
      line += c;
    }
  }

  if (line.length() > 0 && currentY <= 118) {
    tft.setCursor(x, currentY);
    tft.print(line);
  }
}

void showScreen() {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(4, 4);
  tft.print("LIE-GENERATOR");

  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(4, 18);
  tft.print("Status: ");
  tft.print(currentStatus);

  tft.drawFastHLine(0, 31, 160, ST77XX_WHITE);

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(4, 38);
  tft.print("You:");

  drawWrappedText(currentUser, 4, 50, 25, ST77XX_WHITE);

  tft.setTextColor(ST77XX_RED);
  tft.setCursor(4, 76);
  tft.print("Machine:");

  drawWrappedText(currentAI, 4, 88, 25, ST77XX_WHITE);
}

void setLedMode(String mode) {
  mode.toLowerCase();

  if (mode == "red") {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  } else if (mode == "green") {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  } else if (mode == "both") {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
  }
}

void handleCommand(String commandLine) {
  commandLine.trim();

  int separator = commandLine.indexOf('|');
  if (separator < 0) {
    return;
  }

  String command = commandLine.substring(0, separator);
  String value = commandLine.substring(separator + 1);

  command.toUpperCase();

  if (command == "STATUS") {
    currentStatus = value;
    showScreen();
  } else if (command == "USER") {
    currentUser = value;
    showScreen();
  } else if (command == "AI") {
    currentAI = value;
    showScreen();
  } else if (command == "LED") {
    setLedMode(value);
  } else if (command == "RESET") {
    currentStatus = "Ready";
    currentUser = "";
    currentAI = "Tell me your truth.";
    setLedMode("off");
    showScreen();
  }
}

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(TOUCH_PIN, INPUT);
  setLedMode("off");

  Serial.begin(115200);

  displaySPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  currentStatus = "Ready";
  currentAI = "Tell me your truth.";
  showScreen();
}

void loop() {
  touchStatus = digitalRead(TOUCH_PIN);
  if(touchStatus == HIGH){
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    lastTouchStatus = touchStatus;
  }else{
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    lastTouchStatus = touchStatus;
    currentStatus = "Not Ready";
  }
  if (Serial.available()) {
    String commandLine = Serial.readStringUntil('\n');
    handleCommand(commandLine);
  }
}