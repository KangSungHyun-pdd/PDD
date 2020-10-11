#include <Arduino.h>
#include <Adafruit_DotStar.h>
#include <SPI.h>
#include <Servo.h>
Servo motor;

int pwm_m = 1000;
char str = "";
typedef uint16_t line_t;

// CONFIGURABLE STUFF ------------------------------------------------------

#include "footballteam.h"
#define LED_DATA_PIN  5
#define LED_CLOCK_PIN 6
boolean autoCycle = true; // Set to true to cycle images by default
#define CYCLE_TIME 5     // 이미지 지속시간(sec)
// -------------------------------------------------------------------------

Adafruit_DotStar strip = 
Adafruit_DotStar(NUM_LEDS,LED_DATA_PIN, LED_CLOCK_PIN, DOTSTAR_BGR);

void imageInit(void);  //이미지 초기화


void setup() {
 
  Serial.begin(9600);
  //led setup
  strip.begin(); // DotStar buffer 할당, SPI 초기화
  strip.clear(); // Make sure strip is clear
  strip.show();  // before measuring battery
  imageInit();   // Initialize pointers for default image(기본 이미지에 대한 포인터 초기화)

  //motor setup
  motorInit(); 
}

// GLOBAL STATE STUFF ------------------------------------------------------
//uint32_t t = millis(); // Current time, milliseconds 현재시간
uint32_t lastImageTime = 0L, // 마지막 이미지 변경 시간
         lastLineTime  = 0L;
uint8_t  imageNumber   = 0,  // Current image being displayed 이미지 표시
         imageType,          // Image type: PALETTE[1,4,8] or TRUECOLOR
        *imagePalette,       // -> palette data in PROGMEM
        *imagePixels,        // -> pixel data in PROGMEM
         palette[16][3];     // RAM-based color table for 1- or 4-bit images
line_t   imageLines,         // Number of lines in active image
         imageLine;          // Current line number in image

const uint8_t PROGMEM brightness[] = { 15, 31, 63, 127, 255 };
uint8_t bLevel = sizeof(brightness) - 1;


// Microseconds per line for various speed settings
const uint16_t PROGMEM lineTable[] = { // 375 * 2^(n/3)
  1000000L /  375, // 375 lines/sec = slowest
  1000000L /  472,
  1000000L /  595,
  1000000L /  750, // 750 lines/sec = mid
  1000000L /  945,
  1000000L / 1191,
  1000000L / 1500  // 1500 lines/sec = fastest
};
uint8_t  lineIntervalIndex = 3;
uint16_t lineInterval      = 1000000L /900;

void imageInit() 
{ // Initialize global image state for current imageNumber
  imageType    = pgm_read_byte(&images[imageNumber].type);
  imageLines   = pgm_read_word(&images[imageNumber].lines);
  imageLine    = 0;
  imagePalette = (uint8_t *)pgm_read_word(&images[imageNumber].palette);
  imagePixels  = (uint8_t *)pgm_read_word(&images[imageNumber].pixels);

  if(imageType == PALETTE1)     
  memcpy_P(palette, imagePalette,  2 * 3);
  else if(imageType == PALETTE4) 
  memcpy_P(palette, imagePalette, 16 * 3);
  
  lastImageTime = millis(); // Save time of image init for next auto-cycle
}


void motorInit()
{
  motor.attach(9,1000,1565);
  motor.write(1000);   //초기값은 무조건 0, 캘리브레이션과 다름
}

void nextImage(void) 
{
  if(++imageNumber >= NUM_IMAGES) imageNumber = 0;
  imageInit();
}

void prevImage(void) 
{
  imageNumber = imageNumber ? imageNumber - 1 : NUM_IMAGES - 1;
  imageInit();
}

// MAIN LOOP ---------------------------------------------------------------

void loop() {
 
  toTal();
  motorControl(); 
 }

 
void image0(){
  imageNumber=0;
}
void image1(){
  imageNumber=1;
}
void image2(){
  imageNumber=2;
  
}
void image3(){
  imageNumber=3;
  
}

void imageCon(){
  if(str == "a"){
    image0();
  }else if(str == "b"){
    image1();
  }else if(str == "c"){
    image2();
  }else if(str == "d"){
    image3();
  }

}

void motorControl(){
  motor.write(1085); 
}

void toTal(){
    
    imageCon();
    
  if(autoCycle) {
    if((t - lastImageTime) >= (CYCLE_TIME * 1000L)) nextImage();
  }

  switch(imageType) {

    case PALETTE1: { // 1-bit (2 color) palette-based image
      uint8_t  pixelNum = 0, byteNum, bitNum, pixels, idx,
              *ptr = (uint8_t *)&imagePixels[imageLine * NUM_LEDS / 8];
      for(byteNum = NUM_LEDS/8; byteNum--; ) { // Always padded to next byte
        pixels = pgm_read_byte(ptr++);  // 8 pixels of data (pixel 0 = LSB)
        for(bitNum = 8; bitNum--; pixels >>= 1) {
          idx = pixels & 1; // Color table index for pixel (0 or 1)
          strip.setPixelColor(pixelNum++,
            palette[idx][0], palette[idx][1], palette[idx][2]);
        }
      }
      break;
    }

    case PALETTE4: { // 4-bit (16 color) palette-based image
      uint8_t  pixelNum, p1, p2,
              *ptr = (uint8_t *)&imagePixels[imageLine * NUM_LEDS / 2];
      for(pixelNum = 0; pixelNum < NUM_LEDS; ) {
        p2  = pgm_read_byte(ptr++); // Data for two pixels...
        p1  = p2 >> 4;              // Shift down 4 bits for first pixel
        p2 &= 0x0F;                 // Mask out low 4 bits for second pixel
        strip.setPixelColor(pixelNum++,
          palette[p1][0], palette[p1][1], palette[p1][2]);
        strip.setPixelColor(pixelNum++,
          palette[p2][0], palette[p2][1], palette[p2][2]);
      }
      break;
    }

    case PALETTE8: { // 8-bit (256 color) PROGMEM-palette-based image
      uint16_t  o;
      uint8_t   pixelNum,
               *ptr = (uint8_t *)&imagePixels[imageLine * NUM_LEDS];
      for(pixelNum = 0; pixelNum < NUM_LEDS; pixelNum++) {
        o = pgm_read_byte(ptr++) * 3; // Offset into imagePalette
        strip.setPixelColor(pixelNum,
          pgm_read_byte(&imagePalette[o]),
          pgm_read_byte(&imagePalette[o + 1]),
          pgm_read_byte(&imagePalette[o + 2]));
      }
      break;
    }

    case TRUECOLOR: { // 24-bit ('truecolor') image (no palette)
      uint8_t  pixelNum, r, g, b,
              *ptr = (uint8_t *)&imagePixels[imageLine * NUM_LEDS * 3];
      for(pixelNum = 0; pixelNum < NUM_LEDS; pixelNum++) {
        r = pgm_read_byte(ptr++);
        g = pgm_read_byte(ptr++);
        b = pgm_read_byte(ptr++);
        strip.setPixelColor(pixelNum, r, g, b);
      }
      break;
    }
  }


  if(++imageLine >= imageLines) imageLine = 0; // Next scanline, wrap around

  strip.show(); // Refresh LEDs
  //lastLineTime = t; 
}

void serialEvent(){
  while(Serial.available()){
    str = Serial.read();
    imageCon();
    Serial.println(str);
  }
}
