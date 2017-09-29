#include <LiquidCrystal.h>

LiquidCrystal  lcd (8,9,4,5,6,7); 
//Key message
char msgs[5][15] = {"Right Key OK ", 
                    "Up Key OK    ", 
                    "Down Key OK  ", 
                    "Left Key OK  ", 
                    "Select Key OK" };
int  adc_key_val[5] ={30, 150, 360, 535, 760 };
int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int oldkey=-1;

void setup() { 
  lcd.begin(16, 2);
  //optionally, now set up our application-specific display settings, overriding whatever the lcd did in lcd.init()
  //lcd.commandWrite(0x0F);//cursor on, display on, blink on.  (nasty!)
  lcd.clear();
  lcd.write("HELLO FUCKERS");
  lcd.setCursor(0,2);
  lcd.write("press a key");
  delay(1000);
  lcd.setCursor(0,2);
  lcd.write("            ");
}
void loop() {
  adc_key_in = analogRead(0);    
  key = get_key(adc_key_in); 
  if (key != oldkey){   
    delay(50);   
    adc_key_in = analogRead(0); 
    key = get_key(adc_key_in); 
    if (key != oldkey) {      
      oldkey = key;
      if (key >=0){
      lcd.setCursor(0,2);  //line=2, x=0
    lcd.write(msgs[key]);
      }
    }
  }
  
  //delay(1000);
}


int get_key(unsigned int input){
  int k;
  for (k = 0; k < NUM_KEYS; k++){
    if (input < adc_key_val[k]){
      return k;
    }
  }
  if (k >= NUM_KEYS)
    k = -1;    
  return k;
}
