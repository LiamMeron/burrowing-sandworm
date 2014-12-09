#include <LiquidCrystal.h>
#define REFVOLT 3.45

LiquidCrystal lcd(12, 11, 9, 8, 7, 6);
byte pix1[8] = {
    B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000};    //Characters for 
byte pix2[8] = {
    B11000, B11000, B11000, B11000, B11000, B11000,B11000, B11000};     //the normal
byte pix3[8] = {
    B11100, B11100, B11100, B11100, B11100, B11100, B11100, B11100};    //graphing
byte pix4[8] = {
    B11110, B11110, B11110, B11110, B11110, B11110, B11110, B11110};
byte pix5[8] = {
    B11111, B11111, B11111, B11111, B11111, B11111, B11111,B11111};

unsigned long timeElapsedSinceLastFrequencyCalc = 0;
float hz = 0;
float cmSec;
unsigned long timer_start = 0;
volatile unsigned int edges = 0;


float db;
float volts;
int peak;
boolean bool_StereoSignalBit1;
//boolean bool_StereoSignalBit2;
String tapeSideIdentifierString;
String topGraphIdentifierString;
String bottomGraphIdentifierString;
unsigned long timeOfLastGraphUpdate;

float rightChannelValue_NoGain;
float rightChannelValue_Gain;
float leftChannelValue_NoGain;
float leftChannelValue_Gain;
float rightChannelValue;
float leftChannelValue;

float topGraphDisplayValue;
float bottomGraphDisplayValue;
char sideOfTape;

uint16_t lastGraphToggleButtonReading;
uint8_t flag_WhichGraphToDisplay;

void setup(){

    lcd.createChar(0,pix1);
    lcd.createChar(1,pix2);
    lcd.createChar(2,pix3);
    lcd.createChar(3,pix4);
    lcd.createChar(4,pix5);

    pinMode(4, INPUT); //The pin for stereo select signal bits
    pinMode(5, INPUT); //The other pin for stereo select signal bits
    
    Serial.begin(9600);
    lcd.begin(20, 4);

    //WARNING!
    analogReference(EXTERNAL);//DO NOT RUN WITH AREF PIN-IN 
    //IF THIS CMD IS COMMENTED OUT!

    //attach an interrupt to calculate the time
    attachInterrupt(0,isr,CHANGE); //Interrupt on PIN D3 on Leonardo, D2 on 2560
    //capture current time
    timer_start = micros();
}



/******************************************************************************
 *******************************************************************************/

void calculateAndDisplayFrequency(){
    if (micros() - timer_start >= 1000000){
        //If 1000000 micros (1 seconds) has passed, stop the interrupt counter,
        //and stop all interrupts in order to get an accurate time read
        cli();

        // ** detachInterrupt(0); ** //

        //calculate the ticks/second, i.e. frequency and display on screen.
        timeElapsedSinceLastFrequencyCalc = micros() - timer_start; //time since timer started


        hz = edges/2.0/(timeElapsedSinceLastFrequencyCalc/1000000);
        cmSec = convert(hz);
        /*divide by 2.0 because I measure each rise/fall for greater accuracy.
         divide by 1,000,000 because this is the scaler for converting between us
         and seconds, which is what I have been using.*/


        lcd.setCursor(0,0);
        for(int i = 0; i<20;i++){
            lcd.print(" ");
        }
        lcd.setCursor(0,0);
        lcd.print(" SPEED ");
        lcd.print(cmSec);
        lcd.print(" CM/SEC. ");
        //Start interrupts back up
        sei();

        /****DEBUG***/
        //    Serial.println("hz, edges, timeElapsedSinceLastFrequencyCalc:");
        //    Serial.println(hz); //DEBUGGING
        //    Serial.println(edges);
        //    Serial.println(timeElapsedSinceLastFrequencyCalc);
        //    Serial.println(" ");
        /***DEBUG***/



        //reset the edges counter
        edges = 0;

        //Restart the timer,
        timer_start = micros();
    }
}

void updateGraph(){
    if (millis() - timeOfLastGraphUpdate >= 250){
        timeOfLastGraphUpdate = millis();

        leftChannelValue_Gain = analogRead(A0);
        leftChannelValue_NoGain = analogRead(A1);

        rightChannelValue_Gain = analogRead(A4);
        rightChannelValue_NoGain = analogRead(A5);
        
        bool_StereoSignalBit1=digitalRead(4);
        
        if (leftChannelValue_Gain < 900){
            leftChannelValue = leftChannelValue_Gain/80);
        }
        else{
            leftChannelValue = leftChannelValue_NoGain;
        } 
        
        if (rightChannelValue_Gain < 900){
            rightChannelValue = (rightChannelValue_Gain/80);
        }
        else{
            rightChannelValue = rightChannelValue_NoGain;
        }
        
        if (digitalRead(5) == 0){
            sideOfTape='A';
        }
        else{
            sideOfTape='B';
        }
        
        if (bool_StereoSignalBit1==0){
            //L and R
            clearLine(1);
            printGraph(1,get_db(leftChannelValue), sideOfTape);
            clearLine(3);
            printGraph(3,get_db(rightChannelValue), sideOfTape);
        }
        else{
            //L-R L+R
            clearLine(1);
            printGraph(1,get_db(leftChannelValue - rightChannelValue),'');
            clearLine(3);
            printGraph(3,get_db(leftChannelValue + rightChannelValue),'');
        }
        
    }
}

void determineGraphParameters(){ //WAITING FOR COLLECTION
    /*
    Read values of the two signal bits and assign to vars
    Determine which side is playing and set var
    Determine whether it is stereo A and B; or A+B and A-B
    */
    bool_StereoSignalBit1  = digitalRead(4);
//    bool_StereoSignalBit2 = digitalRead(5);
    
//    if (bool_StereoSignalBit1 == 0){ //DETERMINES STEREO OR    A-B / A+B
//        
//        if (bool_StereoSignalBit2 == 0){//Bit1==0 and Bit2==0
//            //Stereo, A and B
//            topGraphIdentifierString = "L";
//            bottomGraphIdentifierString = "R";
//        }
//        else{ //Bit1==0 and Bit2==1
//            //Not Stereo, A - B and A + B
//            topGraphIdentifierString = "L - R";
//            bottomGraphIdentifierString = "L + R";
//        }
//    }
//        
//    else{
//        
//        if (bool_StereoSignalBit2 == 0){ //Bit1==1 and Bit2 == 0
//            //Placeholder Conditional
//        }
//        else{ //If Bit1==1 and Bit2== 1
//            //No Display
//        }
//    }

    if (bool_StereoSignalBit1==0){
        //Stereo
    }
    else{
        //A-B and A+B
    }
        
}

/****************************************************************************************
 ****************************************************************************************/



float get_db(float i){
    return 20.0*log10(i);
}

void clearLine(int line){
    lcd.setCursor(0,line);
    for(int i = 0; i < 20; i++){
        lcd.print(" ");
    }
}

void printGraph(int line, int db, String side)
{
    lcd.setCursor(0,line); 
    lcd.print(side);
    for(int i = 0; i<db/5; i++){   //Draws all of the full characters
        lcd.write(4);
    }
    if (db%5 > 0){                    //then,
        lcd.write((db % 5) - 1); //draws the remaining pixels
    }

}


/******************************************************************************
 *******************************************************************************/
float convert(float hz){
    return (2.4*hz)/381;
}

void print_lcd(int l,float s){
    lcd.setCursor(0,l);
    lcd.print(s);
}

void isr(){ //Interrupts Service Routine
    edges += 1;
}

/******************************************************************************
 *******************************************************************************/


/******************************************************************************
 *******************************************************************************/

void loop(){

    calculateAndDisplayFrequency();
//    determineGraphParameters();
    updateGraph();

}


