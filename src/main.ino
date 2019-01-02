#include "include/rules.h"


/*Constantes voor de Shiftregister afkomstig van de datasheet
  http://www.ti.com/lit/ds/symlink/sn74ahc595.pdf
*/
#define SER   A0
#define RCLK  A1
#define SRCLK A2

/*Constantes voor de Multiplexer afkomstig van de datasheet
  http://www.ti.com/lit/ds/symlink/cd4051b.pdf
*/
#define A   A3
#define B   A4
#define C   A5 
#define OUT 2


//Stuursignaal voor de multiplexers
uint8_t counter = 0;

//Byte die gestuurd wordt /
uint8_t ser = 0;
//toestand van OUT/
uint8_t com; 

/*Matrix voor de toestand van de Hall Effect Sensor 
  [0][0] correspondeert met de eerste Hall Effect sensor (rechts boven)
  [7][7] correspondeert met de laatste (64ste) Hall Effect sensor (links onder)*/
char hallSensor[8][8] = {0};
char chessPieces[8] ={PAWN,0,0,0,0,0,0,0};

void setup() {
  // put your setup code here, to run once: 
  Serial.begin(9600);
//Enable Shiftregister /
  pinMode(SER,  OUTPUT);
  pinMode(RCLK, OUTPUT);
  pinMode(SRCLK,OUTPUT);
//Enable Multiplexer /
  pinMode(A,OUTPUT);
  pinMode(B,OUTPUT);
  pinMode(C,OUTPUT);
  pinMode(OUT, INPUT);
}

//Schuift 8 Bits in de shift-register
void writeShift(uint8_t bits){
 digitalWrite(RCLK, LOW);

  for(int i = 7; i >=0; i--){
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

//Zorgt voor het stuursignaal voor de multiplexer*/
inline void mux(uint8_t value){
 digitalWrite(A, value  & ( 1 << 0));
 digitalWrite(B, value  & ( 1 << 1));
 digitalWrite(C, value  & ( 1 << 2));
}
void loop() {
/*leest de waarden van de hall-effect sensoren */
for(uint8_t i = 0; i < 8; i++){
  mux(i);
  hallSensor[0][i] = digitalRead(OUT); 
  }
for(uint8_t i = 0; i < 8; i++){
  if(chessPieces[i] > 0){
    if(~hallSensor[0][i])){
      writeShift(1 << i +1);
    }else{
      writeShift(0x00);
    }
  }
}







}

