#include <Adafruit_GFX.h>    
#include <SdFat.h>    
#include <Adafruit_SPIFlash.h>    
#include <Adafruit_ImageReader.h>
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789


#define SD_CS   12 // SD card select pin
#define TFT_CS  15 // TFT select pin
#define TFT_DC  27 // TFT display/command pin
#define TFT_RST 33 // Or set to -1 and connect to Arduino RESET pin



SdFat                   SD;        
Adafruit_ImageReader    reader(SD);
Adafruit_Image          bg;
Adafruit_ST7789         tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
ImageReturnCode         stat;




void setup() {
  Serial.begin(115200);

  



  tft.init(135, 240);
  SD.begin(SD_CS, SD_SCK_MHZ(10));
  tft.fillScreen(ST77XX_BLUE);

  word pre_mem = ESP.getFreeHeap();
  
  Serial.print("Before Image Load: ");
  Serial.print(pre_mem);
  Serial.println(" bytes");

  
  
  stat = reader.loadBMP("/bg/bg.bmp", bg);
 
  Serial.print("After Image Load: ");
  Serial.print(ESP.getFreeHeap()/1000);
  Serial.println(" kb");

  Serial.print("Image Size: ");
  Serial.print((pre_mem - ESP.getFreeHeap()));
  Serial.println(" bytes");
  Serial.println();
  Serial.println();


  Serial.print("After Image Load: ");
  Serial.println(ESP.getFreeHeap());
  Serial.println();
  Serial.println();

  
  reader.printStatus(stat);

  tft.setRotation(3);
  GFXcanvas16* canvas;
  canvas = (GFXcanvas16*) bg.getCanvas();
  uint16_t* bgBuff = canvas->getBuffer();
 


  int x = 0;
  int y = 0;
  int h = canvas->height();
  int w = canvas->width();
  Serial.println(h);
  Serial.println(w);


  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      //if(bgBuff[j * w + i] == ST77XX_WHITE) //65535
     // { bgBuff[j * w + i] = NULL;}
      tft.drawPixel(x + i, y, bgBuff[j * w + i]);
    }
  }

  
 // tft.drawRGBBitmap(0, 0, bgBuff, canvas->width(), canvas->height());
 // bg.draw(tft, 0, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  //bg.draw(tft, 0, 0);
}



//#ifdef __arm__
//// should use uinstd.h to define sbrk but Due causes a conflict
//extern "C" char* sbrk(int incr);
//#else  // __ARM__
//extern char *__brkval;
//#endif  // __arm__
//
//int freeMemory() {
//  char top;
//#ifdef __arm__
//  return &top - reinterpret_cast<char*>(sbrk(0));
//#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
//  return &top - __brkval;
//#else  // __arm__
//  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
//#endif  // __arm__
//}
