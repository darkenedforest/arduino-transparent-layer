#include <Adafruit_GFX.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_ImageReader.h>
#include <Adafruit_ST7789.h>


#define SD_CS               12      // SD card select pin
#define TFT_CS              15      // TFT select pin
#define TFT_DC              27      // TFT display/command pin
#define TFT_RST             33      // Or set to -1 and connect to Arduino RESET pin
#define TRANSPARENT_COLOR   63488   // Pixel color to replace with background


SdFat                   SD;
Adafruit_ImageReader    reader(SD);
Adafruit_ST7789         tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
ImageReturnCode         stat;
Adafruit_Image          bg;                     // Object to store background image
Adafruit_Image          sprite;                 // Object to store sprite image


int                     sprite_h, sprite_w,     // Sprite height and width
                        sprite_x, sprite_y;     // x and y origin of top left corner of the sprite on the display
uint16_t                *bg_patch;              // Pointer to background patch buffer, to be dynamically allocated later




void setup() {

  // Start the serial monitor
  Serial.begin(115200);

  // Initalize the display. I'm using the 1.14" 135x240 TFT. Set your size accordingly.
  tft.init(135, 240);

  // Check the SD card motor turns over
  if (!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for (;;); // Fatal error, do not continue
  }

  // I like a nice black screen to start out
  tft.fillScreen(ST77XX_BLACK);


  // Load bg.bmp located on root folder of the SD card into bg image object
  stat = reader.loadBMP("/bg.bmp", bg);

  /*  
   *  Make sure it successfully loaded. Trying to read the buffer of an empty image object will cause the world to come crashing down around you.
   *  No joke, there will be fires. Buildings will collapse. Loved ones will be lost.  Don't tempt fate or suffer the consequences.   
   */
  if (stat != IMAGE_SUCCESS) {
    tft.println("Failed to load image. Is the SD card installed?");
    for (;;); // Fatal error, do not continue
  };

  // Load sprite.bmp located on root folder of the SD card into bg image object
  stat = reader.loadBMP("/sprite.bmp", sprite);

  // Same ominous worning as above.
  if (stat != IMAGE_SUCCESS) {
    tft.println("Failed to load image. Is the SD card installed?");
    for (;;); // Fatal error, do not continue
  };

  //  Incase you need to rotate the screen.
  //  tft.setRotation(3);


  /*
   * Ok, this is where the fun begins. This is probably why you are reading this code.
   * Adafruit_Image object holds a canvas obtained from the GFX library. So we get the canvas
   * and then get the canvas buffer to gain access to the raw pixel data.
   */

  // Declare canvas pointer
  GFXcanvas16* bg_canvas;


  /* 
   *  getCanvas returns void and must be type converted. Read the description in Adafruit_Image library, 
   *  file Adafruit_ImageReader.cpp, line 144 for more details.
   */ 
  tft.println("Getting background canvas...");
  bg_canvas = (GFXcanvas16*) bg.getCanvas();

  // Now that we have the canvas we can get a pointer to the buffer.
  tft.println("Getting background canvas buffer...");
  uint16_t* bg_buff = bg_canvas->getBuffer();


  // Do the same thing to get the pixe; data for the sprite.
  GFXcanvas16* sprite_canvas;

  tft.println("Getting sprite canvas...");
  sprite_canvas   = (GFXcanvas16*) sprite.getCanvas();

  tft.println("Getting sprite canvas buffer...");
  uint16_t*       sprite_buff = sprite_canvas->getBuffer();
  int             bg_h = bg_canvas->height(),          // Get the background image height from canvas
                  bg_w = bg_canvas->width(),           // Get the background image width from canvas
                  bg_x = 0, bg_y = 0;                  // Set the x and y origin point for background image

  sprite_x        = 100;                               // Set the sprite x origin point
  sprite_y        = 60;                                // Set the sprite y origin point
  sprite_h        = sprite_canvas->height();           // Get the sprite image height from canvas
  sprite_w        = sprite_canvas->width();            // Get the sprite image width from canvas

  /*
   * Dynamically allocate the memory for the backgound pactch image. Generally speaking, dynamic memory allocation
   * is considered a bad thing. So, while it works in this instance to determine the space needed for the image
   * on the sd card without know the actually dimensions of the image, don't go getting allocate and deallocate 
   * happy. It will lead to fragmented memory and your microcontroller will become schizophrenic and possibly
   * develop multiple personality disorder.
   */
  tft.println("Allocating memory for patch...");
  bg_patch        = (uint16_t*) malloc (sprite_h * sprite_w);


  
  
  /*
   * This is an ugly set of loops. j represents each row of the image. The loop ends when all the rows have been processed.
   * i represents each pixel in the row. Since you are moving laterally through the image buffers, you have to multiple j times
   * the row width to get to to the correct place in memory that represents the start of the row. Then add i tom move that many
   * pixels into the row.
   * i.e. If the image is 20 pixesl heigh and 30 pixels wide:
   *    In the first loop j = 0. 0 x 30 = 0. So you are still at the first index position in the image buffer. 
   *    i is 0, so 0 + 0 = 0. This is the first pixel. i loops through to 29. So you have 0 + 1 = 1 (second pixel), 
   *    0 + 2 = 2...0 + 29 = 29 (last pixel in the first row.)
   *    Now j = 1. 1 * 30 = 30 Position 30 in the index is the first pixel in the second row, etc.
   *       
   *    It gets a little more complicated for the background image, because you are starting at the origin point of the sprite
   *    in the background image buffer. (sprite_y * bg_w + sprite_x) then you multiple the row number (j) by the background width
   *    to wrap around the background image to the start of the next row. Then add i to move through the pixels in the row.
   *    
   *  Each pixel from the background image is stored in the previously created array "bg_patch" to build the patch image.
   *  Then the sprite image is checked to see if its the special color
   */
  tft.println("Preforming find and replace of pixels and building background patch...");
  for (int16_t j = 0; j < sprite_h; j++) {
    for (int16_t i = 0; i < sprite_w; i++) {
      int index = j * sprite_w + i;
      uint16_t bg_index = sprite_y * bg_w + sprite_x + j * bg_w + i;
      bg_patch[index] = bg_buff[bg_index];
      Serial.println(sprite_buff[index]);
      if (sprite_buff[index] == TRANSPARENT_COLOR) {
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
}
