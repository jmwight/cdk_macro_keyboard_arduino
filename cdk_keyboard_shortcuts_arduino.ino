 #include <Arduino.h>
#include "Adafruit_TinyUSB.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>
#include <stdlib.h> // NULL memory address definition 

#define KEY_1_PRESSED(x) 1 & x
#define KEY_2_PRESSED(x) 1 << 1 & x
#define KEY_3_PRESSED(x) 1 << 2 & x
#define KEY_4_PRESSED(x) 1 << 3 & x
#define KEY_5_PRESSED(x) 1 << 4 & x
#define KEY_6_PRESSED(x) 1 << 5 & x
#define KEY_7_PRESSED(x) 1 << 6 & x
#define KEY_8_PRESSED(x) 1 << 7 & x
#define KEY_9_PRESSED(x) 1 << 8 & x
#define KEY_10_PRESSED(x) 1 << 9 & x
#define KEY_11_PRESSED(x) 1 << 10 & x
#define KEY_12_PRESSED(x) 1 << 11 & x
// The MacroPad has 12 keys, and the NeoPixels are chained on Pin 19
#define NUM_PIXELS 12
#define PIN_NEOPIXEL 19
#define SCREEN_TIMEOUT  14000
#define SHORT_DELAY 500
#define MEDIUM_DELAY  1000
#define LONG_DELAY  2000
#define SUPER_DELAY 3000

void read_line(int lines);
void read_ro();
uint8_t *fnl();
uint8_t *pp(int print_num);
uint8_t *pp_tcm(int print_num);
uint8_t *fc();
uint8_t *change_to_tyler();
void reset_last_key_press_state();
void open_ro();
void send_key(uint8_t k);
void send_key_with_modifier(uint8_t k, uint8_t modifier);
uint8_t send_text(char *s);
void write_to_screen(char *s);

const int pin = D0;
bool activeState = false;
uint8_t *last_func_key_press_state = NULL;

static uint8_t  wait = 0; // wait already written to screen

// HID report descriptor using TinyUSB's template
uint8_t const desc_keyboard_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

// USB HID objects
Adafruit_USBD_HID usb_keyboard;

// Display
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &SPI1, OLED_DC, OLED_RST, OLED_CS);

// Keyboard lighting
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// the setup function runs once when you press reset or power the board
void setup()
{
  /* Display setup */
  // Start the screen hardware (true overrides the default I2C reset)
  display.begin(0, true);
 
  // Clear the buffer
  display.clearDisplay();
  display.display();
 
  // Configure text properties
  display.setTextSize(1);               // Text size (1 is tiny, 2 is medium)
  display.setTextColor(SH110X_WHITE);   // Use SH110X_WHITE or SH110X_BLACK
  display.setTextWrap(true);           // Stop text from stretching weirdly

  /* key light setup */
  pixels.begin();
  pixels.setBrightness(20); // 0 to 255
 
  /* USB communication setup */
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  /* init 12 keys */
  for(int i = 1; i < 13; ++i)
  {
    pinMode(i, INPUT_PULLUP);
  }

  // HID Keyboard
  usb_keyboard.setPollInterval(2);
  usb_keyboard.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  usb_keyboard.setReportDescriptor(desc_keyboard_report, sizeof(desc_keyboard_report));
  usb_keyboard.setStringDescriptor("TinyUSB HID Keyboard");
  usb_keyboard.begin();

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  // Set up button, pullup opposite to active state
  pinMode(pin, activeState ? INPUT_PULLDOWN : INPUT_PULLUP);

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(MEDIUM_DELAY);
  digitalWrite(13, LOW);
}

void light_on_one_second(void)
{
  digitalWrite(13, HIGH);
  delay(MEDIUM_DELAY);
  digitalWrite(13, LOW);
}

void process_hid()
{
  uint16_t keys_pressed = 0; // bitfield for keys pressed
 
  for(int i = 1; i < 13; ++i) {
    keys_pressed |= (!digitalRead(i)) << i-1;
  }

  // Remote wakeup
  if (TinyUSBDevice.suspended() && keys_pressed) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  /*------------- Keyboard -------------*/
  if (usb_keyboard.ready())
  {
    // use to send key release report
    static bool has_key = false;
   
    // multiple keys pressed at the same time
    if ((keys_pressed & (keys_pressed - 1)))
      return;

    if (keys_pressed)
    {
      if(KEY_1_PRESSED(keys_pressed))
      {
        key_light(0, true);
        read_line();
      }
      else if(KEY_2_PRESSED(keys_pressed))
      {
        key_light(1, true);
        read_another_line();
      }
      else if(KEY_3_PRESSED(keys_pressed))
      {
        key_light(2, true);
        last_func_key_press_state = fnl();
      }
      else if(KEY_4_PRESSED(keys_pressed))
      {
        key_light(3, true);
        last_func_key_press_state = pp_tcm(2);
      }
      else if(KEY_5_PRESSED(keys_pressed))
      {
        key_light(4, true);
        last_func_key_press_state = pp(2);
      }
      else if(KEY_6_PRESSED(keys_pressed))
      {
        key_light(5, true);
        last_func_key_press_state = fc();
      }
      else if(KEY_7_PRESSED(keys_pressed))
      {
        key_light(6, true);
        last_func_key_press_state = change_to_tyler();
      }
      else if(KEY_12_PRESSED(keys_pressed))
      {
        key_light(11, true);
        reset_last_key_press_state();
      }
      else
      {
        write_to_screen("Key not programmed");
        delay(100);
      }

      has_key = true;
    }
    else
    {
      // send empty key report if previously has key pressed
      if (has_key)
        usb_keyboard.keyboardRelease(0);
        key_lights_off();
      has_key = false;
    }
  }
}

void loop()
{
  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  // poll gpio once each 10 ms
  static uint32_t ms = 0;
  if (millis() - ms > 10) {
    ms = millis();
    process_hid();
  }

  static uint32_t ms2 = 0;
  if (millis() - ms2 > SCREEN_TIMEOUT) {
    ms2 = millis();
    display.clearDisplay();
    display.display();
    write_to_screen("Waiting...");
    wait = true;
    display.setContrast(0);
    display.display();
  }
}

/* turns on or off a specific key backcolor color (white) */
void key_light(uint8_t key, uint8_t on)
{
  if(on)
  {
    pixels.setPixelColor(key, pixels.Color(255, 255, 255));
    pixels.show();
  }
  else
  {
    pixels.setPixelColor(key, pixels.Color(0, 0, 0));
    pixels.show();
  }
}

/* turns off all key backlights */
void key_lights_off()
{
  int i;
  for(i = 0; i < 12; ++i)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
}

/* sends an individual key with a modifier. Example SHIFT+key or CTRL+ALT+key. Modifier is OR bitmask of CTRL & ALT or whatever combo
   yout want */
void send_key_with_modifier(uint8_t k, uint8_t modifier)
{
  uint8_t keycode[6] = {0};
  keycode[0] = k;
  usb_keyboard.keyboardReport(0, modifier, keycode);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(10);
  usb_keyboard.keyboardRelease(0);
  delay(10);
  digitalWrite(13, LOW);
}


/* sends an individual key */
void send_key(uint8_t k)
{
  send_key_with_modifier(k, 0); // just use send_key_with_modifier with no modifier 
}

void write_to_screen(char *s)
{
  display.clearDisplay();
  display.display();
  display.setContrast(80);
  // Set the coordinate cursor (X, Y) where text starts
  display.setCursor(0, 10);            
  display.print(s);

  // Push the text from the microchip memory to the physical glass
  display.display();
  wait = false;
}

void read_line()
{
  write_to_screen("Read Line");
  char i;
  for(i = 0; i < 7; ++i)
  {
    send_key(HID_KEY_ENTER);  
  }
}

void read_another_line()
{
 
  write_to_screen("Read Additional Line");
  char i;
  for(i = 0; i < 6; ++i)
  {
    send_key(HID_KEY_ENTER);  
  }
}

/* print an RO that is something else */
uint8_t *pp(int print_num)
{
  static uint8_t press_num = 0;
  if(press_num == 0)
  {
    ++press_num;
    send_text("fc\ny\n\n");
    delay(MEDIUM_DELAY);
    send_text(".\n");
    write_to_screen("pp:\nChange method of payment if needed. When done press button again");
  }
  else if(press_num == 1)
  {
    ++press_num;
    send_text("pp\n");
    write_to_screen("pp:\nEnter out mileage if not already correct. When done press button again");
  }
  else
  {
    press_num = 0;
    send_text("\n\n");
    delay(LONG_DELAY);
    send_text("y\n");
    delay(SUPER_DELAY);
    if(print_num == 1)
      send_text("1");
    else
      send_text("2");
    send_text("\n");
    write_to_screen("Print");
  }

  return &press_num;
}

/* print an RO that is just TCM */
uint8_t *pp_tcm(int print_num)
{
  static uint8_t press_num = 0;
  if(press_num == 0)
  {
    ++press_num;
    send_text("fc\ny\n\n");
    delay(MEDIUM_DELAY);
 
    send_text(".\ncmp\ntcm\n\npp\n");
    write_to_screen("pp_tcm:\n Please enter out mileage if not already correct then hit key again");
  }
  else
  {
    press_num = 0;
    send_text("\n\n");
    delay(LONG_DELAY);
    send_text("y\n");
    delay(SUPER_DELAY);
    if(print_num == 1)
      send_text("1");
    else
      send_text("2");
    send_text("\n");
    write_to_screen("Print TCM");
  }

  return &press_num;
}

uint8_t *fnl()
{
  static uint8_t press_num = 0;
  if(press_num == 0)
  {
    ++press_num;
    write_to_screen("fnl: \nPlease Enter line on terminal then hit this key again.");
    send_text("fnl ");
    delay(SHORT_DELAY);
  }
  else
  {
    press_num = 0;
    send_text("\n999\n");
    write_to_screen("Finish a Line");
    delay(SHORT_DELAY);
  }

  return &press_num;
}

/* final close an RO */
uint8_t *fc()
{
  static uint8_t press_num = 0;
  // first key press
  if(press_num == 0)
  {
    ++press_num;
    open_preinvoiced_ro();
    send_text("fc\ny\n\n");
    delay(MEDIUM_DELAY); // sleep because stupid cdk hangs when keys entered loading fc
    send_text(".\ncmp\n");
    write_to_screen("fc: Change method of payment to correct, then hit button again");
  }
  // second key press
  else if(press_num == 1)
  {
    ++press_num;
    send_text("fc\n");
    write_to_screen("Enter mileage out, then hit button again");
  }
  // third key press
  else if(press_num == 2)
  {
    press_num = 0;
    send_text("\n\n");
    delay(SUPER_DELAY);
    send_text("y\n");
    delay(LONG_DELAY);
    send_text("20\n");
    write_to_screen("Final Close");
  }

  return &press_num;
}

/* gets out goes to swr, then enter RO number, hit button again it changes to tyler then brings you back to pfc */
uint8_t *change_to_tyler()
{
  static uint8_t press_num = 0;
  if(press_num == 0)
  {
    ++press_num;
    open_preinvoiced_ro();
    send_key(HID_KEY_F3);
    delay(SHORT_DELAY); // not sure if delay(500) needed just there for precaution.
    send_key(HID_KEY_F3);
    delay(SHORT_DELAY);
    send_key(HID_KEY_F3);
    delay(SHORT_DELAY);
    send_text("swr\n");
    write_to_screen("change to tyler: \nenter RO number, hit button again");
  }
  else
  {
    press_num = 0;
    send_text("\n");
    delay(MEDIUM_DELAY);
    send_text("\n11725\n");
    send_key(HID_KEY_F3);
    delay(SHORT_DELAY);
    send_text("pfc\n1553087\n\n");
    write_to_screen("Change to Tyler");
  }
  return &press_num;
}

/* resets the key press state of the last used function key to 0 */
void reset_last_key_press_state()
{
  if(last_func_key_press_state != NULL)
  {
    *last_func_key_press_state = 0;
  }
  write_to_screen("Reset key state last key");
}

/* open RO */
void open_preinvoiced_ro()
{
    send_text("\n");
    delay(MEDIUM_DELAY);
    send_text("ok\n");
    delay(MEDIUM_DELAY); // if not here it goes into the okay box. CDK lag
}

/* instead of having to do send_key(HID_KEY_*) a million times you can do send_text("cmp \n.\ntcm\n") and it converts */
uint8_t send_text(char *s)
{
  do
  {
    switch(*s)
    {
      case 'A':
      case 'a':
        send_key(HID_KEY_A);
        break;
      case 'B':
      case 'b':
        send_key(HID_KEY_B);
        break;
      case 'C':
      case 'c':
        send_key(HID_KEY_C);
        break;
      case 'D':
      case 'd':
        send_key(HID_KEY_D);
        break;
      case 'E':
      case 'e':
        send_key(HID_KEY_E);
        break;
      case 'F':
      case 'f':
        send_key(HID_KEY_F);
        break;
      case 'G':
      case 'g':
        send_key(HID_KEY_G);
        break;
      case 'H':
      case 'h':
        send_key(HID_KEY_H);
        break;
      case 'I':
      case 'i':
        send_key(HID_KEY_I);
        break;
      case 'J':
      case 'j':
        send_key(HID_KEY_J);
        break;
      case 'K':
      case 'k':
        send_key(HID_KEY_K);
        break;
      case 'L':
      case 'l':
        send_key(HID_KEY_L);
        break;
      case 'M':
      case 'm':
        send_key(HID_KEY_M);
        break;
      case 'N':
      case 'n':
        send_key(HID_KEY_N);
        break;
      case 'O':
      case 'o':
        send_key(HID_KEY_O);
        break;
      case 'P':
      case 'p':
        send_key(HID_KEY_P);
        break;
      case 'Q':
      case 'q':
        send_key(HID_KEY_Q);
        break;
      case 'R':
      case 'r':
        send_key(HID_KEY_R);
        break;
      case 'S':
      case 's':
        send_key(HID_KEY_S);
        break;
      case 'T':
      case 't':
        send_key(HID_KEY_T);
        break;
      case 'U':
      case 'u':
        send_key(HID_KEY_U);
        break;
      case 'V':
      case 'v':
        send_key(HID_KEY_V);
        break;
      case 'W':
      case 'w':
        send_key(HID_KEY_W);
        break;
      case 'X':
      case 'x':
        send_key(HID_KEY_X);
        break;
      case 'Y':
      case 'y':
        send_key(HID_KEY_Y);
        break;
      case 'Z':
      case 'z':
        send_key(HID_KEY_Z);
        break;
      case '\n':
        send_key(HID_KEY_ENTER);
        break;
      case '.':
        send_key(HID_KEY_PERIOD);
        break;
      case ' ':
        send_key(HID_KEY_SPACE);
        break;
      case '0':
        send_key(HID_KEY_0);
        break;
      case '1':
        send_key(HID_KEY_1);
        break;
      case '2':
        send_key(HID_KEY_2);
        break;
      case '3':
        send_key(HID_KEY_3);
        break;
      case '4':
        send_key(HID_KEY_4);
        break;
      case '5':
        send_key(HID_KEY_5);
        break;
      case '6':
        send_key(HID_KEY_6);
        break;
      case '7':
        send_key(HID_KEY_7);
        break;
      case '8':
        send_key(HID_KEY_8);
        break;
      case '9':
        send_key(HID_KEY_9);
        break;
      default:
        return -1;
    }
  } while(*s++ != '\0');
  return 0;
}
