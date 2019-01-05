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
#define BUTTON 3


//Stuursignaal voor de multiplexers
uint8_t counter = 0;

//Byte die gestuurd wordt /
uint8_t ser = 0;
//toestand van OUT/
uint8_t com; 
//
bool isLifted = 0; 
bool isLegal  = 0;
bool isPlayed = 0;
int turn     = BLACK_TURN;

/*Matrix voor de toestand van de Hall Effect Sensor 
  [0][0] correspondeert met de eerste Hall Effect sensor (rechts boven)
  [7][7] correspondeert met de laatste (64ste) Hall Effect sensor (links onder)*/
char hallSensor[8][8] = {0};
uint8_t chessPieces[8] ={WHITE_PAWN,0,0,0,0,0,0,BLACK_PAWN};

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
  pinMode(BUTTON, INPUT);
  
}

//Schuift 8 Bits in de shift-register
void writeShift(uint8_t bits){
 digitalWrite(RCLK, LOW);
//Schuift de meest significante bit eerst in 
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
//Leest de hall-effect Sensoren 
void readHall(){
  for(uint8_t i = 0; i < 8; i++){
        mux(i);
        hallSensor[0][i] = digitalRead(OUT); 

  }
}
void checkPiece(){

}
//Coordinaat voor schaakstuk 
struct coordinate{
  uint8_t x; 
  uint8_t y;
};
//Licht boord op en geeft aan waar het schaakstuk naar toe gespeeld kan worden 
uint8_t showMove(uint8_t piece,struct coordinate pos){
  uint8_t shiftbit, counter = 1;
  if(turn == WHITE_TURN){
    switch(piece){
      case WHITE_PAWN:
        //Speciale case, wanneer pion eerste zet speelt
        if(chessPieces[pos.x+1] == 0 && chessPieces[pos.x+2] == 0 && pos.x == 0){
          return ( 1 << pos.x + 1 | 1 << pos.x + 2);
        }else if(chessPieces[pos.x+1] == 0){
          return ( 1 << pos.x + 1 );
        }else{
          return 0;
        }
    }
    
  }else if(turn == BLACK_TURN){
    switch(piece){
      case BLACK_PAWN:
        //Speciale case, wanneer pion eerste zet speelt
        if(chessPieces[pos.x-1] == 0 && chessPieces[pos.x-2] == 0 && pos.x == 7){
          return ( 1 << pos.x - 1 | 1 << pos.x - 2);
        }else if(chessPieces[pos.x-1] == 0){
          return ( 1 << pos.x - 1 );
        }else{
          return 0;
        }
    }
  }
   
 
}
//Kijkt als de schaakstuk geldig gezet werd 
uint8_t checkMove(uint8_t piece, struct coordinate pos){
 if(turn == WHITE_TURN){
   switch(piece){

     case WHITE_PAWN:
      if(hallSensor[0][pos.x+1] == false && chessPieces[pos.x+1] == 0){
        isPlayed = 1;
        isLifted = 0;
        return pos.x+1;
      }else if(hallSensor[0][pos.x+2] == false && chessPieces[pos.x+2] == 0 && pos.x == 0){
       isPlayed = 1;
       isLifted = 0;
       return pos.x+2;
    }
   

    default:
      return pos.x;
   }
 }else if(turn == BLACK_TURN){
   switch(piece){

     case BLACK_PAWN:
      if(hallSensor[0][pos.x-1] == false && chessPieces[pos.x-1] == 0){
          isPlayed = 1;
          isLifted = 0;
          return pos.x-1;
        }else if(hallSensor[0][pos.x-2] == false && chessPieces[pos.x-1] == 0 && pos.x == 7){
          isPlayed = 1;
          isLifted = 0;
          return pos.x-2;
        }
    default:
      return 0; 
    } 
 }

  return pos.x;
}

uint8_t shiftbit = 0; 
coordinate coord;
coordinate temp;
void loop() {
    /*leest de waarden van de hall-effect sensoren */
    readHall();
    
    if(isLifted == false && isPlayed == false){
      shiftbit = 0;
      for(uint8_t i = 0; i < 8; i++){
        if(turn == WHITE_TURN){
          if(chessPieces[i] >= WHITE_PAWN && chessPieces[i] <= WHITE_KING){
             if(hallSensor[0][i] == HIGH){
              coord.x  = i;
              isLifted = true; 
            }
          }
        }else if(turn == BLACK_TURN){
          if(chessPieces[i] >= BLACK_PAWN && chessPieces[i] <= BLACK_KING){
             if(hallSensor[0][i] == HIGH){
              coord.x  = i;
              isLifted = true; 
            }
          }
        }
        
       
      }
         
    }else if(isLifted == true && isPlayed == false){
      
      temp.x   = checkMove(chessPieces[coord.x], coord);
      shiftbit = showMove(chessPieces[coord.x],coord);
      if(hallSensor[0][coord.x] == LOW){
        isLifted = false; 
        isPlayed = false;
      }
    }else if(isLifted == false && isPlayed == true){
      
      shiftbit = 0;
      if(hallSensor[0][temp.x] == HIGH){
        isLifted = true; 
        isPlayed = false;
      }
      if(digitalRead(BUTTON) == true){
        isLifted = false;
        isPlayed = false; 
        turn = ((turn == WHITE_TURN) ? BLACK_TURN : WHITE_TURN);
        chessPieces[temp.x] = chessPieces[coord.x];
        chessPieces[coord.x] = 0;
      }
    }

  writeShift(shiftbit);
   
   
  
}








