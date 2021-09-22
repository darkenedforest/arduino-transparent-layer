/*
 * Author: Tyler Baker
 * Created: 9/19/2021
 * Board: ESP32 (Adafruit Feather HUZZAH32 PID 3405
 * TFT Display: Adafruit 1.14" 240x135 PID 4383
 * 
 * This sketch is intended to show how to edit the image buffer of an Adafruit_Image object to acheive a "transparent layer" effect for a sprite image
 * on top of a background image. The process is accomplished by scanning the pixels of the sprite image for the transparent color (defined below) and replacing
 * it with the corrosponding background image pixel. And additional buffer is created of the same size as the sprite image. This image is a cutout of the background
 * image where the sprite will be placed. This will act as a pacth to cover over the sprite during frame transitions.
 * A sample video of the process is available here: https://youtu.be/lk2fqWkhBDs
 */



#include <Adafruit_GFX.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_ImageReader.h>
#include <Adafruit_ST7789.h>



/*
 * Adjust these as needed
 */
#define SD_CS               12      // SD card select pin
#define TFT_CS              15      // TFT select pin
#define TFT_DC              27      // TFT display/command pin
#define TFT_RST             33      // Or set to -1 and connect to Arduino RESET pin
#define SPRITE_X            100     // Sprite x origin point
#define SPRITE_Y            60      // Sprite y origin point
#define BG_X                0       // Background x origin point
#define BG_Y                0       // Background y origin point
#define TRANSPARENT_COLOR   0xF800  // Pixel color to replace with background. It's Red. RGB 255,0,0. 
                                    // Why is it 0xF800 instead of 0xFF0000 or ST77XX_RED? The Adafruit_ImageReader
                                    // library packs 24 bit colors (8:8:8) into 16 bits (5:6:5). You can see the conversion
                                    // in the coreBMP() function of the library. It's in Adafruit_ImageReader.cpp, line 541.
                                    // There is a web site for converting from 8:8:8 to 5:6:5.
                                    // http://greekgeeks.net/#maker-tools_convertColor



/*
 * Declare some stuff
 */
SdFat                   SD;
Adafruit_ImageReader    reader(SD);
Adafruit_ST7789         tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
ImageReturnCode         stat;
Adafruit_Image          bg;                     // Object to store background image
Adafruit_Image          sprite;                 // Object to store sprite image
byte                    sprite_h, sprite_w;     // Sprite height and width    
uint16_t                *bg_patch;              // Pointer to background patch buffer, to be dynamically allocated later




void setup() {

  // Start the serial monitor
  Serial.begin(115200);



  // Initalize the display. I'm using the 1.14" 135x240 TFT. Set your size accordingly.
  tft.init(135, 240);



  tft.setRotation(3);



  // Check the SD card motor turns over
  if (!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for (;;); // Fatal error, do not continue
  }



  // Load bg.bmp located on root folder of the SD card into bg image object
  stat = reader.loadBMP("/bg.bmp", bg);



  /*  
   *  Make sure it successfully loaded. Trying to read the buffer of an empty image object will cause the world to come crashing down around you.
   *  No joke, there will be fires. Buildings will collapse. Loved ones will be lost.  Don't tempt fate or suffer the consequences.   
   */
  if (stat != IMAGE_SUCCESS) {
    for (;;); // Fatal error, do not continue
  };

  

  // Load sprite.bmp located on root folder of the SD card into bg image object
  stat = reader.loadBMP("/sprite.bmp", sprite);

  

  // Same ominous warning as above.
  if (stat != IMAGE_SUCCESS) {
    for (;;); // Fatal error, do not continue
  };



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
  bg_canvas = (GFXcanvas16*) bg.getCanvas();



  /*
   * Now that we have the canvas we can get a pointer to the buffer.
   */
  uint16_t* bg_buff = bg_canvas->getBuffer();



  // Do the same thing to get the pixel data for the sprite.
  GFXcanvas16* sprite_canvas;
  sprite_canvas   = (GFXcanvas16*) sprite.getCanvas();
  uint16_t*       sprite_buff = sprite_canvas->getBuffer();
  byte            bg_h = bg_canvas->height(),          // Get the background image height from canvas
                  bg_w = bg_canvas->width();           // Get the background image width from canvas
  sprite_h        = sprite_canvas->height();           // Get the sprite image height from canvas
  sprite_w        = sprite_canvas->width();            // Get the sprite image width from canvas

  /*
   * Dynamically allocate the memory for the backgound pactch image. Generally speaking, dynamic memory allocation
   * is considered a bad thing. So, while it works in this instance to determine the space needed for the image
   * on the sd card without know the actually dimensions of the image, don't go getting allocate and deallocate 
   * happy. It will lead to fragmented memory and your microcontroller will become schizophrenic and possibly
   * develop multiple personality disorder.
   */
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
   *    in the background image buffer. (SPRITE_Y * bg_w + SPRITE_X) then you multiple the row number (j) by the background width
   *    to wrap around the background image to the start of the next row. Then add i to move through the pixels in the row.
   *    
   *  Each pixel from the background image is stored in the previously created array "bg_patch" to build the patch image.
   *  Then the sprite image is checked to see if its the special color
   */



  for (int16_t j = 0; j < sprite_h; j++) {                               // For each row of pixels
    for (int16_t i = 0; i < sprite_w; i++) {                             // For each pixel in the row
      short index = j * sprite_w + i;                                    // This will increment 0, 1, 2, 3, etc...
      short bg_index = SPRITE_Y * bg_w + SPRITE_X + j * bg_w + i;        // Sprite origin point in background image (SPRITE_Y * bg_w + SPRITE_X) plus current number of 
                                                                         // rows into the sprite image (j * bg_w) plus current number of pixels in the sprite image (i).
                                                                         // This will get you to the pixel data in the background image relative to the sprite image at the
                                                                         // same location on the display.
                                                                         
      bg_patch[index] = bg_buff[bg_index];                               // Add the current backbground pixel to the background patch buffer/
      if (sprite_buff[index] == TRANSPARENT_COLOR) {                     // If the current sprite pixel color is the transparent color,
        sprite_buff[index] = bg_buff[bg_index];                          // replace the sprite pixel with the pixel from the background
      }
    }
  }
  


  // Drop in the background. Use the draw function on the image object.
  bg.draw(tft, BG_X, BG_Y);                                             // Comment out this line if you want to see how the sprite and pacth look without the background
}

void loop() {
  // Alternate drawing the modified sprite image and the background patch every second
  sprite.draw(tft, SPRITE_X, SPRITE_Y);                                 // Use the draw function on the image object.
  delay(1000);
  tft.drawRGBBitmap(SPRITE_X, SPRITE_Y, bg_patch, sprite_w, sprite_h);  // use GFX's drawRGBBitmap to draw the background patch pixel buffer
  delay(1000);
}
