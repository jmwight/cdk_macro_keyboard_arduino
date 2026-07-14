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

void tcm(int print_num)
{
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
