#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal  lcd (8,9,4,5,6,7); 


//vars for lcd shield (with adc buttons)
int  adc_key_val[5] ={30, 150, 360, 535, 760 };
int NUM_KEYS = sizeof(adc_key_val)/sizeof(int);
int adc_key_in;
int key=-1;
int oldkey=-1;
const unsigned long adress = 0;


//constants
const unsigned int POST_COUNT = 4;
const unsigned long COUNT_MAX = 999999;
const unsigned long p_delay = 7*1000; //ms
const unsigned long p_op = 40; //s
const unsigned long t_reset = 5*1000; //ms
const unsigned long t_false = 1*1000;
const unsigned long range = -1;
//constants for real buttons
const int buttons[] = {0, 1, 2, 3};


//coffee vars
unsigned long post[POST_COUNT];
unsigned long show[POST_COUNT];
unsigned long t0[POST_COUNT];
unsigned long t1[POST_COUNT];
bool pressed[POST_COUNT];
bool counted[POST_COUNT];
bool false_start[POST_COUNT];


void EEPROMWritelong(int address, long value) {
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
}


long EEPROMReadlong(long address) {
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}


void update_lcd(unsigned int key) {
  switch(key) {
    case 0:
        lcd.setCursor(0,0);
        lcd.write("        ");
        lcd.setCursor(0,0);
        lcd.print(show[0]);
        break;
    case 1:
        lcd.setCursor(8,0);
        lcd.write("        ");
        lcd.setCursor(8,0);
        lcd.print(show[1]);
        break;
    case 2:
        lcd.setCursor(0,1);
        lcd.write("        ");
        lcd.setCursor(0,1);
        lcd.print(show[2]);
        break;
    default:
      break;
  }
}


void clear_lcd() {
  lcd.setCursor(0,0);
  lcd.write("        ");
  lcd.setCursor(8,0);
  lcd.write("        ");
  lcd.setCursor(0,1);
  lcd.write("        ");
  
  lcd.setCursor(0,0);
  lcd.print(show[0]);
  lcd.setCursor(8,0);
  lcd.print(show[1]);
  lcd.setCursor(0,1);
  lcd.print(show[2]);
}


void update_post(int key) {
  switch(key){
    case 0:
      post[0]++;
      if (post[0] > COUNT_MAX) {
        post[0] = COUNT_MAX;
      }
      else {
        EEPROMWritelong(adress, post[0]);
      }
      break;
    case 1:
      post[1]+=2;
       if (post[1] > COUNT_MAX) {
        post[1] = COUNT_MAX;
      }
      else {
        EEPROMWritelong(adress+4, post[1]);
      }
      break;
    case 2:
      post[2]++;
      if (post[2] > COUNT_MAX) {
        post[2] = COUNT_MAX;
      }
      else {
        EEPROMWritelong(adress+8, post[2]);
      }
      break;
    default:
      break;
  }
}


void start_key(int key) {
  if (key < 0 || key >= POST_COUNT) {
    return;
  }
  
      t0[key] = millis();
      t1[key] = millis();
}


void stop_key(int key) {
  if (key < 0 || key >= POST_COUNT) {
    return;
  }
  
  t0[key] = 0;
  t1[key] = 0;
}


void update_key(int key) {
  if (key < 0 || key >= POST_COUNT) {
    return;
  }
  
  t1[key] = millis();
}


unsigned long get_time(unsigned long t1, unsigned long t0) {
  if (t1 >= t0) {
    return (t1 - t0) ;
  }
  else {
    return ((range - t0) + t1) ;
  }
}


void setup() {
//  EEPROMWritelong(adress, 0);
//  EEPROMWritelong(adress+4, 0);
//  EEPROMWritelong(adress+8, 0);
 
  post[0] = EEPROMReadlong(adress);
  post[1] = EEPROMReadlong(adress+4);
  post[2] = EEPROMReadlong(adress+8);
  
  for (unsigned int i = 0; i < POST_COUNT; i++) {
    if (post[i] > COUNT_MAX) {
      post[i] = COUNT_MAX;
    }
    show[i] = post[i];
  }

  for (unsigned int i = 0; i < POST_COUNT; i++) {
    pinMode(buttons[i], INPUT);           // set pin to input
    digitalWrite(buttons[i], HIGH);       // turn on pullup resistors
  }
    
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(show[0]);
  lcd.setCursor(8,0);
  lcd.print(show[1]);
  lcd.setCursor(0,1);
  lcd.print(show[2]);
}


void loop() {
  delay(50);
  
  for (unsigned int i = 0; i < POST_COUNT; i++) {
    key = i;
    if (digitalRead(key) == LOW) {
      
      if (key > -1 && key < 3) {
        if (pressed[key] == false) {
          start_key(key);
          pressed[key] = true;
          if (false_start[key] == true) {
            update_post(key);
            false_start[key] = false;
          }
        }
        else {

          if (!counted[key]) {
           update_key(key);
            if (get_time(t1[key], t0[key]) >= t_false) {
              false_start[key] = true;
            }
            
            if (get_time(t1[key], t0[key]) >= p_delay) {
              update_post(key);
              counted[key] = true;
              false_start[key] = false;
              start_key(key);
            }
          }
          else {
            if (get_time(t1[key], t0[key])/1000 >= p_op) {
              update_post(key);
              counted[key] = true;
              start_key(key);
            }
          }
          show[key] = get_time(t1[key], t0[key]);
          update_lcd(key);
        }
      }

  
      else if (key == 3) {
        if (pressed[key] == false) {
          start_key(key);
          pressed[key] = true;
        }
        else {
          update_key(key);
          if (get_time(t1[key], t0[key]) >= t_reset) {
              EEPROMWritelong(adress, 0);
              EEPROMWritelong(adress+4, 0);
              EEPROMWritelong(adress+8, 0);
              for (unsigned int i = 0; i < POST_COUNT; i++) {
                 post[i] = 0;
                 show[i] = post[i];
                 update_lcd(i);
              }
              stop_key[key];
              pressed[key] = false;
            }
          }
      }
    }

  
      else if (digitalRead(buttons[i]) == HIGH)  {
          if (pressed[key] == true) {
            pressed[key] = false;
            stop_key(key);
            show[key] = 0;
            counted[key] = false;
            show[key] = post[key];
            clear_lcd();
          }
      }

  }
  
} //void loop(void)

