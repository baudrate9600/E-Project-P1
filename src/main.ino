#include "C:\Users\sojim\Documents\School\E-Project-P1\include\rules.h"


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
//
bool isPicked = 0; 
bool isLegal  = 0;
bool isPlayed = 0;

/*Matrix voor de toestand van de Hall Effect Sensor 
  [0][0] correspondeert met de eerste Hall Effect sensor (rechts boven)
  [7][7] correspondeert met de laatste (64ste) Hall Effect sensor (links onder)*/
char hallSensor[8][8] = {0};
char chessPieces[8] ={WHITE_PAWN,0,0,0,0,0,0,0};

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
void readHall(){
  for(uint8_t i = 0; i < 8; i++){
        mux(i);
        hallSensor[0][i] = digitalRead(OUT); 
  }
}
void checkPiece(){

}
void showMove(uint8_t piece){

}
struct coordinate{
  uint8_t x; 
  uint8_t y;
};
void loop() {
    /*leest de waarden van de hall-effect sensoren */
    coordinate coord;
    readHall();
    if(isPicked == false && isPlayed == false){
      for(uint8_t i = 0; i < 8; i++){
        if(chessPieces[i] > 0){
          checkPiece();
          if(hallSensor[0][i] == HIGH){
            showMove(1);
            coord.x = i;
            writeShift(1 << i+1 | 1 << i+2);
            isPicked = true;
          }else{
            writeShift(0x00);
          }
        }
      }
      
    }else{
        if(hallSensor[0][coord.x] == LOW ){
            isPicked = false;
            isLegal = 0; 
            isPlayed = 0; 
        }else if(hallSensor[0][coord.x+1] == LOW){
            chessPieces[coord.x] = 0;
            chessPieces[coord.x+1] = WHITE_PAWN;
            isPlayed = 1;
        }else if(hallSensor[0][coord.x+2] == LOW){           
            chessPieces[coord.x] = 0;
            chessPieces[coord.x +2] = WHITE_PAWN;
            isPlayed = 1;
        }
    }

  if(isPlayed == true){
    writeShift(0x00);
    
  }
   
   
  
}








