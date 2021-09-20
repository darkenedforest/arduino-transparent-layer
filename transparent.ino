#include <Adafruit_GFX.h>    
#include <SdFat.h>    
#include <Adafruit_SPIFlash.h>    
#include <Adafruit_ImageReader.h>
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <OneButton.h>  // for reset button


#define SD_CS   12 // SD card select pin
#define TFT_CS  15 // TFT select pin
#define TFT_DC  27 // TFT display/command pin
#define TFT_RST 33 // Or set to -1 and connect to Arduino RESET pin
#define C_BTN_PIN 4    // Center button pin for reset



SdFat                   SD;        
Adafruit_ImageReader    reader(SD);
Adafruit_Image          bg;
Adafruit_Image          sprite;
Adafruit_ST7789         tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
ImageReturnCode         stat;

// center button
OneButton c_btn = OneButton(
                    C_BTN_PIN,  // Input pin for the button
                    true,        // Button is active LOW
                    true         // Enable internal pull-up resistor
                  );


  int sprite_h;
  int sprite_w;
  int sprite_x;
  int sprite_y;
  uint16_t *bg_patch;   ///[60*60];


void setup() {
  c_btn.attachClick(cClick);
  c_btn.attachLongPressStart(cLongStart);
  Serial.begin(115200);

  



  tft.init(135, 240);
  SD.begin(SD_CS, SD_SCK_MHZ(10));
  tft.fillScreen(ST77XX_BLACK);

  word pre_mem = ESP.getFreeHeap();
  
  Serial.print("Before Image Load: ");
  Serial.print(pre_mem);
  Serial.println(" bytes");

  
  
  stat = reader.loadBMP("/bg/bg.bmp", bg);
  reader.loadBMP("/ball.bmp", sprite);
 
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
  GFXcanvas16* bg_canvas;
  bg_canvas = (GFXcanvas16*) bg.getCanvas();
  uint16_t* bg_buff = bg_canvas->getBuffer();


  GFXcanvas16* sprite_canvas;
  sprite_canvas = (GFXcanvas16*) sprite.getCanvas();
  uint16_t* sprite_buff = sprite_canvas->getBuffer();
 

  int loop_x = 120;
  int loop_y = 80;
  int bg_x = 0;
  int bg_y = 0;
  int bg_h = bg_canvas->height();
  int bg_w = bg_canvas->width();

  sprite_x = 100;
  sprite_y = 60;
  sprite_h = sprite_canvas->height();
  sprite_w = sprite_canvas->width();
  bg_patch = (uint16_t*) malloc (sprite_h * sprite_w);
  
  
  //bg_patch[sprite_h * sprite_w];
  int start = sprite_y * bg_w + sprite_x;
  Serial.print("Start: ");
  //Serial.println(start);

  
  Serial.println(sprite_h);
  Serial.println(sprite_w);

  //tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(3);
  tft.setCursor(60, 0);
  tft.println("Sprite");
  tft.setCursor(0, 30);
  tft.setTextSize(2);
  tft.println("Ball with red border");
  sprite.draw(tft, 90, 70);
  delay(7000);

  tft.fillScreen(ST77XX_BLACK);
  
  tft.setTextSize(3);
  tft.setCursor(20, 0);
  tft.println("Background");
  delay(2000);
  bg.draw(tft, bg_x, bg_y);
  delay(2000);

  tft.fillScreen(ST77XX_BLACK);


  for (int16_t j = 0; j < sprite_h; j++, loop_y++) {
    for (int16_t i = 0; i < sprite_w; i++) {
      int index = j * sprite_w + i;
      uint16_t bg_index = start + j * bg_w + i;
      bg_patch[index] = bg_buff[bg_index];
        Serial.println(sprite_buff[index]);
      if(sprite_buff[index] == 63488){
        sprite_buff[index] = bg_buff[bg_index];
      }
    }
  }

  tft.setTextSize(3);
  tft.setCursor(60, 0);
  tft.println("Patch");
  tft.setCursor(0, 30);
  tft.setTextSize(2);
  tft.println("Same size as sprite");
  tft.drawRGBBitmap(90, 70, bg_patch, sprite_w, sprite_h);
  delay(7000);

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.println("Replace Reds");
  tft.setCursor(0, 30);
  tft.setTextSize(2);
  tft.println("Replace red with");
  tft.println("background pixels");
  sprite.draw(tft, 90, 70);
  delay(7000);
  
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.println("Toggle Sprite and Patch");
  tft.setCursor(0, 30);
  sprite.draw(tft, 90, 70);
  delay(1000);
  tft.drawRGBBitmap(90, 70, bg_patch, sprite_w, sprite_h);
  delay(1000);
  sprite.draw(tft, 90, 70);
  delay(1000);
  tft.drawRGBBitmap(90, 70, bg_patch, sprite_w, sprite_h);
  delay(1000);
  sprite.draw(tft, 90, 70);
  delay(1000);
  tft.drawRGBBitmap(90, 70, bg_patch, sprite_w, sprite_h);
  delay(1000);

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.println("Putting it");
  tft.println("all together");

  delay(4000);
  
  bg.draw(tft, bg_x, bg_y);
 
  //sprite.draw(tft, sprite_x, sprite_y);
}

void loop() {

   sprite.draw(tft, sprite_x, sprite_y);
   delay(1000);
   tft.drawRGBBitmap(sprite_x, sprite_y, bg_patch, sprite_w, sprite_h);
   delay(1000);
 
   
  // put your main code here, to run repeatedly:
  c_btn.tick();
}
void cClick(){
  tft.println("Center button click");
}
void cLongStart(){
 Serial.print("reset");
  ESP.restart();
}
