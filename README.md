# Simulate transparent layer on Arduino 

  This sketch is intended to show how to edit the image buffer of an Adafruit_Image object to acheive a "transparent layer" effect for a sprite image
  on top of a background image. The process is accomplished by scanning the pixels of the sprite image for the defined "transparent color" and replacing
  it with the corrosponding background image pixel. An additional buffer is created of the same size as the sprite image. This image is a cutout of the background
  image where the sprite will be placed. This will act as a pacth to cover over the sprite during frame transitions.
  
  Hardware used with this sketch: 
   * Board: Adafruit Feather HUZZAH32 PID 3405 (ESP32)  https://www.adafruit.com/product/3405
   * TFT Display: Adafruit 1.14" 240x135 PID 4383  https://www.adafruit.com/product/4383
  
  A sample video of the process is available here: https://youtu.be/lk2fqWkhBDs
  
  Copy bg.bmp and sprite.bmp to the root folder of the SD card.
 
