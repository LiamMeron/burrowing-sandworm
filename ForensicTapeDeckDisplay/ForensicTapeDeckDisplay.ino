// The main class for the calculation, logic switching, and displaying of
// the forensic tape deck

#include <LiquidCrystal.h>
#define REFVOLT 3.3

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

String tapeSideIdentifierString;
String topGraphIdentifierString;
String bottomGraphIdentifierString;
unsigned long timeOfLastGraphUpdate;

boolean bool_StereoSignalBit;
boolean bool_isGrounded =0;

uint16_t eventTime_Left[100];
uint16_t eventTime_Right[100];
uint16_t eventLevel_Left[100];
uint16_t eventLevel_Right[100];

uint8_t eventCounter_Left = 0;
uint8_t eventCounter_Right = 0;

uint16_t dcOffsetVal_leftNoGain = 0;     //Left channel unamplified
uint16_t dcOffsetVal_leftGain = 0;          //Left channel amplified
uint16_t dcOffsetVal_rightNoGain=0;    //Right channel unamplified
uint16_t dcOffsetVal_rightGain=0;         //Right channel amplified

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

    pinMode(0,INPUT);//    Peak Read(1) / Reset Queue (0)
    pinMode(1,INPUT);//    Pause Record (1) / Null (0)
    
    pinMode(2, INPUT);//    Tape Side B (1) / Side A (0)
    
    pinMode(4, INPUT); //    A+B & A-B (1) / AB (0)
    pinMode(5, INPUT); //    Calibrate(1) / Null(0)
    
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
                                                                                            /*
                                                                                            MEASUREMENTS, VALUES, AND VARIABLES
                                                                                                                                                                  */
    leftChannelValue_Gain = analogRead(A0);
    leftChannelValue_NoGain = analogRead(A1);
    
    rightChannelValue_Gain = analogRead(A4);
    rightChannelValue_NoGain = analogRead(A5);
    
    bool_StereoSignalBit=digitalRead(4);
    bool_isGrounded=digitalRead(5);
    
    if (digitalRead(2) == 1){
        sideOfTape='A';
    }
    else{
        sideOfTape='B';
    }        
    
    
                                                                                                        /*                                        
                                                                                                        CALIBRATION ADJUSTMENTS
                                                                                                                                                              */
    if (leftChannelValue_Gain < 900){                //Use the left channel unamplified
        leftChannelValue = ((leftChannelValue_Gain - dcOffsetVal_leftGain)/80);
    }        
    else{                                                                     //Use the left channel amplified
        leftChannelValue = leftChannelValue_NoGain - dcOffsetVal_leftNoGain;
    } 
    
    if (rightChannelValue_Gain < 900){             //Use the right channel amplified
        rightChannelValue = ((rightChannelValue_Gain - dcOffsetVal_rightGain)/80);
    }
    else{                                                                    //Use the right channel unamplified
        rightChannelValue = rightChannelValue_NoGain - dcOffsetVal_rightNoGain;
    }
    
    
    
    
    
    
                                                                                                                    /*
                                                                                                                    DISPLAY DECISIONS
                                                                                                                                                         */
    if (bool_isGrounded==0){                                //If not calibrating
        if (bool_StereoSignalBit==0){
            //L and R
            clearLine(1);
            printGraph(1, get_db(leftChannelValue) , sideOfTape);
            
            clearLine(3);
            printGraph(3, get_db(rightChannelValue) , sideOfTape);
    }
        else{
            //L-R L+R
            clearLine(1);
            printGraph(1, get_db(leftChannelValue - rightChannelValue) , '^');
            
            clearLine(3);
            printGraph(3, get_db(leftChannelValue + rightChannelValue) , '^');
    }
    }
        
    else{                                                                    //Calibrate
        dcOffsetVal_leftNoGain=analogRead(1);
        dcOffsetVal_leftGain=analogRead(0);
        dcOffsetVal_rightNoGain=analogRead(5);
        dcOffsetVal_rightGain=analogRead(4);
    }
      
    
                                                                                                                    /*
                                                                                                                    LOGGING LOGIC
                                                                                                                                                    */
    if (digitalRead(1) == 0){//Second pole is not in the pause record state
        if (digitalRead(0) == 1){ //If switch, is @ peak read position: Determine if current peak values are events and act accordingly
            saveEvent(sideOfTape, 'L', get_db(analogRead(2)), timeOfLastGraphUpdate);     //A2 is the L channel peak sig
            saveEvent(sideOfTape, 'R', get_db(analogRead(3)), timeOfLastGraphUpdate);    //A3 is the R channel peak sig
        }
        else{ //RESET LOGS //Switch is at "Reset queue" position
            eventCounter_Left = 0;
            eventCounter_Right = 0;
        }
    }   
      //ELSE { PAUSE LOGGING }
}




/****************************************************************************************
 ****************************************************************************************/

void saveEvent(char side, char channel, uint16_t valToTest, uint16_t time){        //If the signal is greater than 0DB save it as an event
    if (valToTest >= 0){
        if (channel == 'L'){
            if (eventCounter_Left <= 100){
                if (side == 'B'){
                    eventTime_Left[eventCounter_Left] = ( ((time + 500) / 1000) | 32768 ); //32768 is B1000000000000000. This sets the first bit of the number to 1 signify that tapeSide == B
                    eventCounter_Left += 1;
                }
                else{//Otherwise the bit should always = 0 for side = A
                    eventTime_Left[eventCounter_Left] = ((time + 500) / 1000 ); 
                    eventCounter_Left += 1;
                }
                                
            }
        }
        else {//Channel == R
            if(eventCounter_Right <= 100){
                if (side == 'B'){
                    eventTime_Right[eventCounter_Right] = ( ((time + 500) / 1000) | 32768 ); //32768 is B1000000000000000. This sets the first bit of the number to 1 signify that tapeSide == B
                    eventCounter_Right += 1;
                }
                else {//Otherwise the bit should always = 0 for side = A
                    eventTime_Right[eventCounter_Right] = ((time + 500) / 1000);
                    eventCounter_Right += 1;
                }
            }
        }
    }
}

float get_db(float i){
    return 20.0*log10(i);
}

void clearLine(int line){
    lcd.setCursor(0,line);
    for(int i = 0; i < 20; i++){
        lcd.print(" ");
    }
}

void printGraph(int line, uint8_t db, char side)
{

    lcd.setCursor(0,line); 
    if (side != '^'){
        lcd.print(side);
    }
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
    if (millis() - timeOfLastGraphUpdate >= 250){
        timeOfLastGraphUpdate = millis();
        updateGraph();
    }
}


