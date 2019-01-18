#include "Arduino.h"
#include "include/Shift_Mux.h"

//Schuift 8 Bits in de shift-register
void writeShift(uint16_t bits){
 digitalWrite(RCLK, LOW);

  for(int i = 15; i >=0; i--){
    digitalWrite(SRCLK, LOW);
   
    if(bits & (1 << i)){
      
      digitalWrite(SER, HIGH);
    }else{
      digitalWrite(SER, LOW);
    }
    
    digitalWrite(SRCLK, HIGH);
  }

  digitalWrite(RCLK, HIGH);

}
void writeShift1(uint16_t bits){
 digitalWrite(RCLK1, LOW);

  for(int i = 15; i >=0; i--){
    digitalWrite(SRCLK1, LOW);
    
    if(bits & (1 << i)){
      
      digitalWrite(SER1, HIGH);
    }else{
      digitalWrite(SER1, LOW);
    }
    
    digitalWrite(SRCLK1, HIGH);
  }

  digitalWrite(RCLK1, HIGH);

}
void writeShift32(uint32_t bits){
  writeShift((uint16_t)bits);
  writeShift1((uint16_t)(bits >> 16));
}
void mux(uint8_t value){
 digitalWrite(A, value  & ( 1 << 0));
 digitalWrite(B, value  & ( 1 << 1));
 digitalWrite(C, value  & ( 1 << 2));
}