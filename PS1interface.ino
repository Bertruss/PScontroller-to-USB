#include <Joystick.h>

// Adapt a Sony Playstation controller (all-digital variants, such as scph-1010) to USB
// Intended platform is any atmega32u4 based microcontroller
// author: Michael Hautman
//
// reference: https://gamesx.com/controldata/psxcont/psxcont.htm

//Pin definitions:
int CLK = 10;
int CMD = 18;
int ATT = 19;
int DATA = 20;

//bitmap - 
//bit 1-8: //start/select/dpad
//bit 9-16: //triggers/cross/triangle/square/circle
//bit 17-24: //Data rdy msg, useful for msg fidelity testing

//previous state of button activation
byte laststate[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

//message contents
byte rec[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 


void setup() {
  Joystick.begin();
  pinMode(CLK, OUTPUT);
  pinMode(ATT, OUTPUT);
  pinMode(CMD, OUTPUT);
  pinMode(DATA, INPUT_PULLUP);

  digitalWrite(CLK, HIGH);  
  digitalWrite(ATT, HIGH);  
  digitalWrite(CMD, HIGH);  
}

void loop() {
  ComReset();
  delayMicroseconds(100);
  ComInit();//request data from ocntroller

  for(int i = 0; i < 2; i++){
      softPSSerial(0xFF, i);
      delayMicroseconds(100);
      }
    Decode(0);//start/select/dpad
    Decode(1);//triggers/cross/triangle/square/circle
    memcpy(laststate, rec, sizeof(byte)*16);//copys first 2-bytes of current 'rec' for 'laststate' mem
}

void ComInit(){
   
  digitalWrite(ATT, LOW);
  softPSSerial(0x01, 3); //s: start
  delayMicroseconds(100);

  softPSSerial(0x42, 3); //S: request data, R: controller ID
  delayMicroseconds(100);

  softPSSerial(0xFF, 2); //S: idle, R: data ready
  delayMicroseconds(100);
}

//prepares lines for next data exchange
void ComReset(){
  digitalWrite(CLK, HIGH);  
  digitalWrite(ATT, HIGH);  
  }

// Software defined serial for PS controller comm. protocol
// msg - byte long message sent to controller hw
// sel - selects which byte of memory to save response(0-2), or discards response (3)
void softPSSerial(int msg, int sel){
  int i;
  bool temp;
  for(i = 0; i < 8; i++){
    digitalWrite(CLK, LOW);  
    if((msg >> i) & 1){
      digitalWrite(CMD, HIGH);
    }else {
      digitalWrite(CMD, LOW);
    }
    delayMicroseconds(25);
    digitalWrite(CLK, HIGH);
    if(sel < 3){
      rec[i+8*sel] = digitalRead(DATA);
    }
    delayMicroseconds(30);
  }
  digitalWrite(CMD, HIGH);
}


//transmission decode, with 
void Decode(int type){
     if(type){ //byte 1  
     byte butt;  
      for(int i = 8; i < 16; i++){ 
        butt = i - 8;
        if(rec[i] != laststate[i]){
          if(!rec[i]){
            Joystick.setButton(butt, 1);
            }
          else{
            Joystick.setButton(butt, 0);      
            }  
          }
        }
    }else{ //byte 0
      if(rec[0] != laststate[0])
        Joystick.setButton(8, !rec[0]);
      
      if(rec[3] != laststate[3])
        Joystick.setButton(9, !rec[3]);
      
      int dir = -1;

      //conversion necessary for microsoft's 360 degree hat system
      if(!rec[4] || !rec[5] || !rec[6] || !rec[7]){ //Dpad
        dir = !rec[4]*0 + !rec[5]*90 + !rec[6]*180 + !rec[7]*270 + (-45)*!rec[4]*!rec[5] + (-135)*!rec[5]*!rec[6] + (-225)*!rec[6]*!rec[7] + (45)*!rec[7]*!rec[4];
      }
      Joystick.setHatSwitch(0, dir); //reset
    }
}
