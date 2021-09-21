# Simulate transparent layer on Arduino 

  This sketch is intended to show how to edit the image buffer of an Adafruit_Image object to acheive a "transparent layer" effect for a sprite image
  on top of a background image. The process is accomplished by scanning the pixels of the sprite image for the transparent color (defined below) and replacing
  it with the corrosponding background image pixel. And additional buffer is created of the same size as the sprite image. This image is a cutout of the background
  image where the sprite will be placed. This will act as a pacth to cover over the sprite during frame transitions.
  
  A sample video of the process is available here: https://youtu.be/lk2fqWkhBDs
  
  Copy bg.bmp and sprite.bmp to the root folder of the SD card.
 
