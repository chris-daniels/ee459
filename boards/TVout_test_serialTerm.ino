#include <TVout.h>
#include "pollserial.h"
#include "font6x8.h"

TVout TV;
pollserial pserial;
char flag;
char camera;
unsigned long time;

void setup()  {
  //change_baud();
  camera = 0;
  flag = 0;
  
  start_TV();
  
  TV.println("--Welcome--");
  TV.delay(3000);

  TV.clear_screen();
  TV.println("GARMIN AERO");

}

void loop() {
  
  if (pserial.available()) {
    
    //TV.print((char)pserial.read());
    
    char received = (char)pserial.read();
    
    if(received == 0x25) { // this is a "%"
      TV.clear_screen();
      TV.println("GARMIN AERO");
    }
    
    else if(received == 0x23) // this is a pound "#"
      flag = 1;  
    
    else if(flag) { 
      if(received & 0x01) {
        // turn off video pins!
        DDRB &= ~(1<<PB1);
        DDRD &= ~(1<<PD7);
        flag = 0;
        camera = 1;
        //delay(8000); // delay about 5 sec
      }
      else if (camera) {
        DDRB |= (1<<PB1);
        DDRD |= (1<<PD7);
        //TV.println("Welcome back");
        flag = 0;
        camera = 0;
      }

    }
    
    else
      TV.print(received);
      
  }
}

/*void serialLoop() {
  char byte_in = 0x01;
  while(byte_in != 0x23) { // received a pound, so step out of while loop
    if(Serial.available() > 0) 
      char byte_in = Serial.read();
  }
  while(!Serial.available()) ; // wait until next byte received
  
  if(Serial.available() > 0)
      char byte_in = Serial.read();
  
  if((byte_in & 0x01) == 0) {  
    Serial.end();
    start_TV();
    TV.println("2:44pm");
  }
  else serialLoop();
}*/

void start_TV() {
  //TV.begin(NTSC,184,72);
  TV.begin(NTSC, 120, 90);
  TV.select_font(font6x8);
  TV.set_hbi_hook(pserial.begin(9600));
}

/*void change_baud() {
  Serial.begin(115200);
  Serial.print("$$$"); // enter command mode
  delay(1000);
  //Serial.print("SU,96"); // change to 9600 bitrate
  Serial.print("U,9600,N"); // temporarily change to 9600 bitrate
  delay(100);
  Serial.begin(9600);
  delay(100);
  //Serial.print("R,1"); // restart for permanent change
  Serial.print("---<cr>"); // exit command mode
  delay(100);
  Serial.end(); // close connection
}*/
