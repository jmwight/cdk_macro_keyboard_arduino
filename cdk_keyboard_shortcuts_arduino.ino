#include <Arduino.h>
#include "Adafruit_TinyUSB.h"

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

void read_line(int lines);
void read_ro();
void tcm(int print_num);
void fnl();
uint8_t send_text(char *s);

const int pin = D0;
bool activeState = false;

// HID report descriptor using TinyUSB's template
uint8_t const desc_keyboard_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

// USB HID objects
Adafruit_USBD_HID usb_keyboard;

// the setup function runs once when you press reset or power the board
void setup() 
{
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  /* init 12 keys */
  for(int i = 1; i < 13; ++i)
  {
    pinMode(i, INPUT_PULLUP);
  }

  Serial.begin(115200);

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
  delay(1000);
  digitalWrite(13, LOW);

  Serial.println("Adafruit TinyUSB HID Composite example");
}

void light_on_one_second(void)
{
  digitalWrite(13, HIGH);
  delay(1000);
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
    
    /*if(!digitalRead(1))
    {
        //send_key(HID_KEY_ENTER);
        read_line();
    }*/

    if (keys_pressed)
    {
      
      if(KEY_1_PRESSED(keys_pressed))
        read_line();
      else if(KEY_2_PRESSED(keys_pressed))
        read_another_line();
      else if(KEY_3_PRESSED(keys_pressed))
        tcm(2);

      has_key = true;
    }
    else
    {
      // send empty key report if previously has key pressed
      if (has_key)
        usb_keyboard.keyboardRelease(0);
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
}

void send_key(uint8_t k)
{
  uint8_t keycode[6] = {0};
  keycode[0] = k;
  usb_keyboard.keyboardReport(0, 0, keycode);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(10);
  usb_keyboard.keyboardRelease(0);
  delay(10);
  digitalWrite(13, LOW);
}

void read_line()
{
  char i;
  for(i = 0; i < 7; ++i)
  {
    send_key(HID_KEY_ENTER);  
  }
}

void read_another_line()
{
  char i;
  for(i = 0; i < 6; ++i)
  {
    send_key(HID_KEY_ENTER);  
  }
}

void tcm(int print_num)
{
  if(print_num == 1)
    send_text("pf\ny\n.\ncmp\ntcm\n\npp\n\n\n1\n");
  else
    send_text("pf\ny\n.\ncmp\ntcm\n\npp\n\n\n2\n");
  /*send_key(HID_KEY_P);
  send_key(HID_KEY_F);
  send_key(HID_KEY_ENTER);
  send_key(HID_KEY_PERIOD);
  send_key(HID_KEY_ENTER);
  send_key(HID_KEY_C);
  send_key(HID_KEY_M);
  send_key(HID_KEY_P);
  send_key(HID_KEY_ENTER);*/
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
