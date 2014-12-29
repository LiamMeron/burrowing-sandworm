#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 9, 8, 7, 6);

uint16_t valOfPeakPauseChannel;

void setup(){
    pinMode(2, INPUT);//    Tape Side B (1) / Side A (0)
    
    pinMode(4, INPUT); //    A+B & A-B (1) / AB (0)
    pinMode(5, INPUT_PULLUP); //    Calibrate(1) / Null(0)
    
    Serial.begin(9600);
    lcd.begin(20, 4);
}

void loop(){
    valOfPeakPauseChannel = analogRead(6);
    
    if (digitalRead(2) == 1){//Tape Side
        lcd.setCursor(0,0);
        lcd.print("Side B");
    }
    else{
        lcd.setCursor(0,0);
        lcd.print("Side A");
    }
    
    
    if (digitalRead(5) == 1){
        lcd.setCursor(10, 0);
        lcd.print("CAL.");
    }
    else{
        lcd.setCursor(10, 0);
        lcd.print("NULL");
    }
    
    if (digitalRead(4)==1){
        lcd.setCursor(0,1);
        lcd.print("Stereo.");
    }
    else{
        lcd.setCursor(0,1);
        lcd.print("L-R;L+R");
    }
    
    if (valOfPeakPauseChannel <= 300){//Switch is in Peak mode
        lcd.setCursor (10,1);
        lcd.print("Save");
        lcd.print(valOfPeakPauseChannel);
    }
    else if (valOfPeakPauseChannel >= 700){
        lcd.setCursor(0,2);
        lcd.print("Reset");
        lcd.print(valOfPeakPauseChannel);
    }
    else{
        lcd.setCursor(0,2);
        lcd.print ("Pause");
        lcd.print(valOfPeakPauseChannel);
    }
    
    delay(250);
    lcd.clear();
        
}
