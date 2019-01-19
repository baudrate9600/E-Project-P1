#include "rules.h"


/*Constantes voor de Shiftregister afkomstig van de datasheet
  http://www.ti.com/lit/ds/symlink/sn74ahc595.pdf
*/
#define SER   A0
#define RCLK  A1
#define SRCLK A2
#define SER1   A3
#define RCLK1  A4
#define SRCLK1 A5
/*Constantes voor de Multiplexer afkomstig van de datasheet
  http://www.ti.com/lit/ds/symlink/cd4051b.pdf
*/
#define A   4
#define B   3
#define C   2 
#define OUT0 10
#define OUT1 11
#define OUT2 12 
#define OUT3 13
#define BUTTON 5 

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
int turn     = WHITE_TURN;

/*Matrix voor de toestand van de Hall Effect Sensor 
  [0][0] correspondeert met de eerste Hall Effect sensor (rechts boven)
  [7][7] correspondeert met de laatste (64ste) Hall Effect sensor (links onder)*/
char hallSensor[8][8] = {0};
uint8_t chessPieces[8] ={WHITE_ROOK,0,0,0,0,0,0, BLACK_ROOK};

void setup() {
  // put your setup code here, to run once: 
  Serial.begin(9600);
//Enable Shiftregister /
  pinMode(SER,  OUTPUT);
  pinMode(RCLK, OUTPUT);
  pinMode(SRCLK,OUTPUT);
  pinMode(SER1,  OUTPUT);
  pinMode(RCLK1, OUTPUT);
  pinMode(SRCLK1,OUTPUT);

//Enable Multiplexer /
  pinMode(A,OUTPUT);
  pinMode(B,OUTPUT);
  pinMode(C,OUTPUT);
  pinMode(OUT0, INPUT);
  pinMode(BUTTON, INPUT);
  
}

//Schuift 16 Bits in de shift-register
void writeShift(uint16_t bits){
 digitalWrite(RCLK, LOW);
//Schuift de meest significante bit eerst in 
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
        hallSensor[0][i] = digitalRead(OUT0); 
        hallSensor[1][i] = digitalRead(OUT1);

  }
}

//Coordinaat voor schaakstuk 
struct coordinate{
  uint8_t x; 
  uint8_t y;
};
//Licht boord op en geeft aan waar het schaakstuk naar toe gespeeld kan worden 
uint16_t showMove(uint8_t piece,struct coordinate pos){
  uint16_t sbit =0;
  //WHITE turn
  //******************************************************************************
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
    
    case WHITE_ROOK:
        for(int i = 1; i < 8; i++){
          if(pos.x+i == 8){
            break; 
          }
          if(chessPieces[pos.x+i] == 0){
            
            sbit |= (1 << (uint8_t)(pos.x+i));
          }else if(chessPieces[pos.x+i] >= BLACK_PAWN && chessPieces[pos.x+i] <= BLACK_KING){
            sbit |= (1 << (uint8_t)(pos.x+i));
            break;
          }
          else{
            break;
          }
        }
        for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.x-i] == 0){
            sbit |= (1 << pos.x-i);
          }else if(chessPieces[pos.x-i] >= BLACK_PAWN && chessPieces[pos.x-i] <= BLACK_KING){
            sbit |= (1 << pos.x-i);

            break;
          }
        }
        return sbit;
    }
//BLACK turn
  //******************************************************************************
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
    case BLACK_ROOK:
        for(int i = 1; i < 8; i++){
          if(pos.x+i == 8){
            break; 
          }
          if(chessPieces[pos.x+i] == 0){
            sbit |= (1 << pos.x+i);
          }else if(chessPieces[pos.x+i] >= WHITE_PAWN && chessPieces[pos.x+i] <= WHITE_KING){
            sbit |= (1 << pos.x+i);
            break;
          }else{
            break;
          }
        }
        for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.x-i] == 0){
            sbit |= (1 << pos.x-i);
          }else if(chessPieces[pos.x-i] >= WHITE_PAWN && chessPieces[pos.x-i] <= WHITE_KING){
            sbit |= (1 << pos.x-i);
            break;
          }else{
            break;
          }
        }
        return sbit;
    }
  }
   
 
}
//Kijkt als de schaakstuk geldig gezet werd 
uint8_t checkMove(uint8_t piece, struct coordinate pos){
  //WHITE turn
  //******************************************************************************
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
    case WHITE_ROOK:
     for(int i = 1; i < 8-pos.x; i++){
          if(chessPieces[pos.x+i] == 0 && hallSensor[0][pos.x+i] == false){
            isPlayed = 1;
            isLifted = 0;
            return pos.x+i;
          }else if(chessPieces[pos.x+i] >= BLACK_PAWN && chessPieces[pos.x+i] <=BLACK_KING){
            if(hallSensor[0][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            return pos.x+i;
            }
          }
     }
     for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.x-i] == 0 && hallSensor[0][pos.x-i] == false){
            isPlayed = 1;
            isLifted = 0;
            return pos.x-i;
          }else if(chessPieces[pos.x-i] >= BLACK_PAWN && chessPieces[pos.x-i] <=BLACK_KING){
            if(hallSensor[0][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            return pos.x-i;
            }
          }
          
     } 
    default:
      return pos.x;
   }
  //BLACK turn
  //******************************************************************************
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
      case BLACK_ROOK:
     for(int i = 1; i < 8-pos.x; i++){
          if(chessPieces[pos.x+i] == 0 && hallSensor[0][pos.x+i] == false){
            isPlayed = 1;
            isLifted = 0;
            return pos.x+i;
          }else if(chessPieces[pos.x+i] >= WHITE_PAWN && chessPieces[pos.x+i] <=WHITE_KING){
            if(hallSensor[0][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            return pos.x+i;
            }
          }
      }
     for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.x-i] == 0 && hallSensor[0][pos.x-i] == false){
            isPlayed = 1;
            isLifted = 0;
            return pos.x-i;
          }else if(chessPieces[pos.x-i] >= BLACK_PAWN && chessPieces[pos.x-i] <=BLACK_KING){
            if(hallSensor[0][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            return pos.x-i;
            }
         } 
     }
    default:
      return 0; 
    } 
 }

  return pos.x;
}
void takeMove(){

}
uint16_t shiftbit = 0; 
coordinate coord;
coordinate temp;

void loop(){
    /*leest de waarden van de hall-effect sensoren */
    readHall();
    /*toestand een*/
    if(isLifted == false && isPlayed == false){
      Serial.println("1");
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
    /*toestand twee*/
    }else if(isLifted == true && isPlayed == false){
      
      temp.x   = checkMove(chessPieces[coord.x], coord);
      shiftbit = showMove(chessPieces[coord.x],coord);
      //Serial.println(coord.x);
      if(hallSensor[0][coord.x] == LOW){
        isLifted = false; 
        isPlayed = false;
      }
      Serial.println("2");
    /*toestand drie*/
    }else if(isLifted == true  && isLifted == true ){
      Serial.println("3");
      if(hallSensor[0][temp.x] == LOW && hallSensor[0][coord.x] == HIGH){
        isLifted = false;
        isPlayed = true; 
      }else if(hallSensor[0][temp.x] == HIGH && hallSensor[0][coord.x] == LOW){
        isLifted =false;
        isPlayed =false;
      }
    /*toestand drie*/
    }else if(isLifted == false && isPlayed == true){
      Serial.println("4");
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
        Serial.println("has Played");
      }
    }
  Serial.println(shiftbit);
  Serial.println(coord.x);
  writeShift(shiftbit);
}








