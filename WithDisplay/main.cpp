#include <RadioLib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <Arduino.h>

#define ValidValuesCount 16
#define BytesCount 12
#define ButtonsCount 12

Adafruit_FT6206 ts = Adafruit_FT6206();
Adafruit_ILI9341 tft = Adafruit_ILI9341(10, 9);
SX1276 radio = new Module(RADIO_NSS_PORT, RADIO_DIO_0_PORT, RADIO_RESET_PORT, RADIO_DIO_1_PORT);

unsigned long Timestamp;
int BitDuration = 500, ButtonWidth = 50, ButtonHight = 50, BUZZER_P = 19, BUZZER_M = 20;
int Buttons[ButtonsCount][4] = {
    {25, 25, 0, ILI9341_RED},
    {100, 25, 0, ILI9341_GREEN},
    {175, 25, 0, ILI9341_BLUE},
    {250, 25, 0, ILI9341_WHITE},
    {25, 100, 0, ILI9341_MAGENTA},
    {100, 100, 0, ILI9341_CYAN},
    {175, 100, 0, ILI9341_YELLOW},
    {250, 100, 0, ILI9341_DARKGREY},
    {25, 175, 0, ILI9341_DARKGREY},
    {100, 175, 0, ILI9341_DARKGREY},
    {175, 175, 0, ILI9341_DARKGREY},
    {250, 175, 0, ILI9341_DARKGREY},
};

std::array<byte, BytesCount> ByteArray, SlimByteArray;
std::array<std::array<int, BytesCount>, ValidValuesCount> ColorArrayArray{
    {// nothing
     {0x56, 0x84, 0x84, 0x84, 0x84, 0x62, 0x36, 0x84, 0x29},
     // {0x??, 0xZE, 0xGG, 0xRR, 0xBB, 0xSP, 0xIM, 0xZE, 0x??};
     // Slow R G B W M C Y
     {0xA6, 0x84, 0x84, 0xB5, 0x84, 0x8C, 0x45, 0x84, 0xA1},
     {0x2D, 0x84, 0xB5, 0x84, 0x84, 0x8C, 0x45, 0x84, 0x15},
     {0x86, 0x84, 0x84, 0x84, 0xB5, 0x8C, 0x45, 0x84, 0x96},
     {0x29, 0x84, 0xB5, 0xB5, 0xB5, 0x8C, 0x45, 0x84, 0x69},
     {0x22, 0x84, 0x84, 0xB5, 0xB5, 0x8C, 0x45, 0x84, 0x6A},
     {0x8D, 0x84, 0xB5, 0x84, 0xB5, 0x8C, 0x45, 0x84, 0x89},
     {0x8C, 0x84, 0xB5, 0xB5, 0x84, 0x8C, 0x45, 0x84, 0xA4},
     // Fast R G B W M C Y
     {0x42, 0x84, 0x84, 0xB5, 0x84, 0x49, 0x45, 0x84, 0x5A},
     {0x52, 0x84, 0xB5, 0x84, 0x84, 0x49, 0x45, 0x84, 0x4C},
     {0x46, 0x84, 0x84, 0x84, 0xB5, 0x49, 0x45, 0x84, 0xB1},
     {0x5A, 0x84, 0xB5, 0xB5, 0xB5, 0x49, 0x45, 0x84, 0x16},
     {0x95, 0x84, 0x84, 0xB5, 0xB5, 0x49, 0x45, 0x84, 0x6C},
     {0x35, 0x84, 0xB5, 0x84, 0xB5, 0x49, 0x45, 0x84, 0x59},
     {0x24, 0x84, 0xB5, 0xB5, 0x84, 0x49, 0x45, 0x84, 0x85},
     // Blink
     {0x96, 0x84, 0xB5, 0x84, 0xB5, 0xB6, 0x4D, 0x84, 0x32}

    }};

void OneButton(int b, int c)
{
  tft.fillRect(Buttons[b][0], Buttons[b][1], ButtonWidth, ButtonHight, c);
  if (!Buttons[b][2])
    tft.fillRect(Buttons[b][0] + 5, Buttons[b][1] + 5, ButtonWidth - 10, ButtonHight - 10, ILI9341_BLACK);
}

void ByteArraySend(void)
{
  ByteArray[0] = 0xAA;
  ByteArray[1] = 0xAA;
  for (int i = 2; i < BytesCount; i++)
    ByteArray[i] = SlimByteArray[i - 2] >> 2 | ((i == 2 ? 1 : SlimByteArray[i - 3]) & 0x03) << 6;
  for (int j = 0; j < BytesCount; j++)
    for (int i = 0; i < 8; i++)
    {
      digitalWrite(RADIO_DIO_2_PORT, (128U & ByteArray[j]) / 128U);
      delayMicroseconds(BitDuration);
      ByteArray[j] <<= 1;
    }
  digitalWrite(RADIO_DIO_2_PORT, 0);
  for (int k = 0; k < 8; k++)
    delayMicroseconds(BitDuration);
}

void SendColor(int c)
{
  for (int j = 0; j < BytesCount - 3; j++)
    SlimByteArray[j] = ColorArrayArray[c][j];
  ByteArraySend();
}

void beep(void)
{
  for (int i = 0; i < 100; i++)
  {
    digitalWrite(BUZZER_P, !digitalRead(BUZZER_P));
    digitalWrite(BUZZER_M, !digitalRead(BUZZER_P));
    delay(1);
  }
  delay(50);
}

void setup()
{
  Serial.begin(115200);
  pinMode(RADIO_DIO_2_PORT, OUTPUT);
  pinMode(BUZZER_P, OUTPUT);
  pinMode(BUZZER_M, OUTPUT);
  pinMode(PIN_A0, INPUT_ANALOG);
  SPI.setMOSI(RADIO_MOSI_PORT);
  SPI.setMISO(RADIO_MISO_PORT);
  SPI.setSCLK(RADIO_SCLK_PORT);
  radio.beginFSK();
  radio.setFrequency(915.0F);
  radio.setOOK(true);
  radio.transmitDirect();
  SPI.setMOSI(MOSI);
  SPI.setMISO(MISO);
  SPI.setSCLK(SCK);
  tft.begin();
  ts.begin(40);
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(3);
  for (int i = 0; i < ButtonsCount; i++)
    OneButton(i, Buttons[i][3]);
}

void loop()
{
  if (millis() - Timestamp > 300)
  {
    Timestamp = millis();
    if (ts.touched())
    {
      TS_Point p = ts.getPoint();
      int y = tft.height() - p.x;
      int x = p.y;
      for (int i = 0; i < ButtonsCount; i++)
        if ((x > Buttons[i][0]) && (x < (Buttons[i][0] + ButtonWidth)) && (y > Buttons[i][1]) && (y <= (Buttons[i][1] + ButtonHight)))
        {
          Buttons[i][2] = 1 - Buttons[i][2];
          OneButton(i, Buttons[i][3]);
          beep();
        }
    }

    switch (1)
    {
    case 0:
      for (int a10 = 0x00; a10 <= 0xFF; a10++)
      {
        for (int a11 = 0x00; a11 <= 0xFF; a11++)
        {
          SendColor(0);
          for (int j = 0; j < BytesCount - 3; j++)
            SlimByteArray[j] = ColorArrayArray[15][j];

          // change SlimByteArray
          // SlimByteArray[0] = a10; // ColorArrayArray[13][0]; //
          // SlimByteArray[8] = a11;
          // SlimByteArray[5] = 0xB6; // 0x64; // speed
          // SlimByteArray[6] = 0x4D; // 0x6A; // impact? effect?

          ByteArraySend();
          delay(40);
          if (analogRead(PIN_A0) > 600)
          {
            beep();
            for (int i = 0; i < BytesCount - 3; i++)
              Serial.print(SlimByteArray[i], HEX);
            Serial.println();
          }
        }
      }
      break;

    case 1:
      for (int b = 0; b < ButtonsCount; b++)
        if (Buttons[b][2])
        {
          SendColor(0);
          SendColor(b + 1);
        }
      break;

    default:
      for (int k = 1; k < ValidValuesCount; k++)
      {
        SendColor(0);
        SendColor(k);
        delay(2000);
      }
      break;
    }
  }
}
