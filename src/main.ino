#include "rules.h"
/*voor je eigen welzijn collapse de functies. Code is harstikke redundant en niet gecomment.
ja ik kan beter...

maar ik wil slapen */


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

#define X_SIZE 4 

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
uint8_t chessPieces[4][8] ={
  EMPTY , WHITE_PAWN  ,EMPTY  ,EMPTY  ,EMPTY  ,EMPTY  ,EMPTY  ,EMPTY ,
   EMPTY,WHITE_PAWN  ,EMPTY  ,EMPTY  ,EMPTY  ,EMPTY  ,BLACK_PAWN  ,EMPTY,
  EMPTY ,WHITE_PAWN  ,EMPTY  ,EMPTY  ,EMPTY  ,EMPTY  ,BLACK_ROOK  ,EMPTY,
  EMPTY  ,WHITE_PAWN  ,EMPTY  ,EMPTY  ,EMPTY  ,EMPTY  ,EMPTY  ,EMPTY
  };

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
void writeShift(uint32_t bits){
 digitalWrite(RCLK, LOW);
//Schuift de meest significante bit eerst in 
  for(int i = 15; i >=0; i--){
    digitalWrite(SRCLK, LOW);

    if((uint16_t)bits & (1 << i)){
      digitalWrite(SER, HIGH);
    }else{
      digitalWrite(SER, LOW);
    }
    digitalWrite(SRCLK, HIGH);
  }

  digitalWrite(RCLK, HIGH);

  digitalWrite(RCLK1, LOW);
//Schuift de meest significante bit eerst in 
  for(int i = 15; i >=0; i--){
    digitalWrite(SRCLK1, LOW);

    if((uint16_t)(bits >> 16) & (1 << i)){
      digitalWrite(SER1, HIGH);
    }else{
      digitalWrite(SER1, LOW);
    }
    digitalWrite(SRCLK1, HIGH);
  }

  digitalWrite(RCLK1, HIGH);

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
        hallSensor[2][i] = digitalRead(OUT2); 
        hallSensor[3][i] = digitalRead(OUT3);

  }
}

//Coordinaat voor schaakstuk 
struct coordinate{
  uint8_t x; 
  uint8_t y;
};
//Licht boord op en geeft aan waar het schaakstuk naar toe gespeeld kan worden 
uint32_t showMove(uint8_t piece,struct coordinate pos){
  uint32_t sbit =0;
  //WHITE turn
  //******************************************************************************
  if(turn == WHITE_TURN){
    

    if(piece == WHITE_PAWN){
    
        //Speciale case, wanneer pion eerste zet speelt
        if(chessPieces[pos.y][pos.x+1] == 0 && chessPieces[pos.y][pos.x+2] == 0 && pos.x == 1){
          sbit |= ( 1UL << pos.x + 1 + pos.y * 8| 1UL << pos.x + 2+ pos.y * 8);
        }else if(chessPieces[pos.y][pos.x+1] == 0){
          sbit |= ( 1UL << pos.x + 1 + pos.y * 8 );
        }
        if(chessPieces[pos.y+1][pos.x+1] >= BLACK_PAWN && chessPieces[pos.y+1][pos.x+1] <= BLACK_KING){
          
          if(pos.y + 1 == 4 || pos.x + 1 ==8){
          }else{
            sbit |= ( 1UL << pos.x + 1 + 8 * (pos.y+1)  );
          }
        }
        if(chessPieces[pos.y-1][pos.x+1] >= BLACK_PAWN && chessPieces[pos.y-1][pos.x+1] <= BLACK_KING){
          
          if(pos.y - 1 == -1 || pos.x + 1 ==8){
            
          }else{
            
            sbit |= ( 1UL << pos.x + 1 + 8 *  (pos.y-1) );
          }
        }
      return sbit;
    }else if(piece == WHITE_ROOK){
        for(int i = 1; i < 8; i++){
          if(pos.x+i == 8){
            break; 
          }
          if(chessPieces[pos.y][pos.x+i] == 0){
            
            sbit |= (1UL << (pos.x+i + 8 * pos.y));
          }else if(chessPieces[pos.y][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y][pos.x+i] <= BLACK_KING){
            sbit |= (1UL << (pos.x+i + 8 * pos.y));
            break;
          }
          else{
            break;
          }
        }
        for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.y][pos.x-i] == 0){
            sbit |= (1UL << pos.x-i + 8 * pos.y);
          }else if(chessPieces[pos.y][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x-i + 8 * pos.y);

            break;
          }
        }
        for(int i = 1; i < 4; i++){
          if(pos.y == 3){
            break;
          }
          if(chessPieces[pos.y +i][pos.x] == 0){
            sbit |= (1UL << pos.x + 8 *i);
          }else if(chessPieces[pos.y + i][pos.x] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x + 8 *i);
            break;
          }
        }
        for(int i = 1; i < pos.y +1; i++){
          if(chessPieces[pos.y - i][pos.x] == 0){
            sbit |= (1UL << pos.x + 8 * (pos.y -i));
          }else if(chessPieces[pos.y - i][pos.x] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x + 8 * (pos.y -i));
            break;
          }
        }
        return sbit;
    }else if(piece == WHITE_BISHOP){
        for(int i = 1; i < 4 - pos.y && pos.x + i != 8; i++){
          if(chessPieces[pos.y+i][pos.x+i] == 0 ){
            sbit |= (1UL << pos.x+i + 8 *(pos.y+i) );
          }else if(chessPieces[pos.y+i][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x+i] <= BLACK_KING){
            sbit |= (1UL << pos.x+i + 8 *(pos.y+i) );
            break; 
          }else{
            break;
          }
        }
        for(int i = 1; i < pos.y +1  && pos.x  != 0; i++){
          if(pos.x - i == -1 ){
              break;
            }
          if(chessPieces[pos.y-i][pos.x-i] == 0 ){
            sbit |= (1UL << pos.x-i + 8 *(pos.y-i) );
          }else if(chessPieces[pos.y-i][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x-i + 8 *(pos.y-i) );
            break; 
          }else{
            break;
          }
        }
         
        //DEze klopt wss niet 
        for(int i = 1; i < pos.y +1 && pos.x + i != 8 ; i++){
          if(chessPieces[pos.y-i][pos.x+i] == 0 ){
            sbit |= (1UL << pos.x+i + 8 *(pos.y-i) );
          }else if(chessPieces[pos.y-i][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x+i] <= BLACK_KING){
            sbit |= (1UL << pos.x+i + 8 *(pos.y-i) );
            break; 
          }else{
            break;
          }
        }
        
        for(int i = 1; i <  4 -pos.y && pos.x  != 0; i++){
           if(pos.x - i == -1 ){
              break;
            }
          if(chessPieces[pos.y+i][pos.x-i] == 0 ){
           
            sbit |= (1UL << pos.x-i + 8 *(pos.y+i) );
          }else if(chessPieces[pos.y+i][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x-i + 8 *(pos.y+i) );
            break; 
          }else{
            break;
          }
        }
       
        return sbit;
    }else if(piece == WHITE_QUEEN){
      for(int i = 1; i < 4 - pos.y && pos.x + i != 8; i++){
          if(chessPieces[pos.y+i][pos.x+i] == 0 ){
            sbit |= (1UL << pos.x+i + 8 *(pos.y+i) );
          }else if(chessPieces[pos.y+i][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x+i] <= BLACK_KING){
            sbit |= (1UL << pos.x+i + 8 *(pos.y+i) );
            break; 
          }else{
            break;
          }
        }
        for(int i = 1; i < pos.y +1  && pos.x  != 0; i++){
          if(pos.x - i == -1 ){
              break;
            }
          if(chessPieces[pos.y-i][pos.x-i] == 0 ){
            sbit |= (1UL << pos.x-i + 8 *(pos.y-i) );
          }else if(chessPieces[pos.y-i][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x-i + 8 *(pos.y-i) );
            break; 
          }else{
            break;
          }
        }
         
        //DEze klopt wss niet 
        for(int i = 1; i < pos.y +1 && pos.x + i != 8 ; i++){
          if(chessPieces[pos.y-i][pos.x+i] == 0 ){
            sbit |= (1UL << pos.x+i + 8 *(pos.y-i) );
          }else if(chessPieces[pos.y-i][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x+i] <= BLACK_KING){
            sbit |= (1UL << pos.x+i + 8 *(pos.y-i) );
            break; 
          }else{
            break;
          }
        }
        
        for(int i = 1; i <  4 -pos.y && pos.x  != 0; i++){
           if(pos.x - i == -1 ){
              break;
            }
          if(chessPieces[pos.y+i][pos.x-i] == 0 ){
           
            sbit |= (1UL << pos.x-i + 8 *(pos.y+i) );
          }else if(chessPieces[pos.y+i][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x-i + 8 *(pos.y+i) );
            break; 
          }else{
            break;
          }
        }
        for(int i = 1; i < 8; i++){
          if(pos.x+i == 8){
            break; 
          }
          if(chessPieces[pos.y][pos.x+i] == 0){
            
            sbit |= (1UL << (pos.x+i + 8 * pos.y));
          }else if(chessPieces[pos.y][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y][pos.x+i] <= BLACK_KING){
            sbit |= (1UL << (pos.x+i + 8 * pos.y));
            break;
          }
          else{
            break;
          }
        }
        for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.y][pos.x-i] == 0){
            sbit |= (1UL << pos.x-i + 8 * pos.y);
          }else if(chessPieces[pos.y][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x-i + 8 * pos.y);

            break;
          }
        }
        for(int i = 1; i < 4; i++){
          if(pos.y == 3){
            break;
          }
          if(chessPieces[pos.y +i][pos.x] == 0){
            sbit |= (1UL << pos.x + 8 *i);
          }else if(chessPieces[pos.y + i][pos.x] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x + 8 *i);
            break;
          }
        }
        for(int i = 1; i < pos.y +1; i++){
          if(chessPieces[pos.y - i][pos.x] == 0){
            sbit |= (1UL << pos.x + 8 * (pos.y -i));
          }else if(chessPieces[pos.y - i][pos.x] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x-i] <= BLACK_KING){
            sbit |= (1UL << pos.x + 8 * (pos.y -i));
            break;
          }
        }
        return sbit;
      
    }else if(piece == WHITE_KING){
      
      if(chessPieces[pos.y][pos.x+1] == 0 ||  chessPieces[pos.y][pos.x+1] >= BLACK_PAWN && chessPieces[pos.y][pos.x+1] <=BLACK_KING ){
        if(pos.x + 1 == 8){}else{
          sbit|=  ( 1UL << pos.x + 1 + pos.y * 8);
        }
      }
      if(chessPieces[pos.y][pos.x-1] == 0 || chessPieces[pos.y][pos.x-1] >= BLACK_PAWN && chessPieces[pos.y][pos.x-1] <=BLACK_KING ){
        if(pos.x - 1 == -1){}else{
          sbit|=  ( 1UL << pos.x - 1 + pos.y * 8);
        }
      }
      if(chessPieces[pos.y+1][pos.x] == 0 || chessPieces[pos.y+1][pos.x] >= BLACK_PAWN && chessPieces[pos.y+1][pos.x] <=BLACK_KING){
        if(pos.y + 1 == 4){}else{
          sbit|=  ( 1UL << pos.x + (pos.y+1) * 8);
        }
      }
      if(chessPieces[pos.y-1][pos.x] == 0 || chessPieces[pos.y-1][pos.x] >= BLACK_PAWN && chessPieces[pos.y-1][pos.x] <=BLACK_KING){
        if(pos.y - 1 == -1){}else{
          sbit|=  ( 1UL << pos.x + (pos.y-1) * 8);
        }
      }
      if(chessPieces[pos.y+1][pos.x+1] == 0 || chessPieces[pos.y+1][pos.x+1] >= BLACK_PAWN && chessPieces[pos.y+1][pos.x+1] <=BLACK_KING){
        if(pos.x + 1 == 8 || pos.y + 1 == 4){}else{
          sbit|=  ( 1UL << pos.x +1 + (pos.y+1) * 8);
        }
      }
      if(chessPieces[pos.y-1][pos.x+1] == 0 || chessPieces[pos.y-1][pos.x+1] >= BLACK_PAWN && chessPieces[pos.y-1][pos.x+1] <=BLACK_KING){
        if(pos.x + 1 == 8 || pos.y - 1 == -1){}else{
          sbit|=  ( 1UL << pos.x +1 + (pos.y-1) * 8);
        }

      }
      if(chessPieces[pos.y-1][pos.x-1] == 0 || chessPieces[pos.y-1][pos.x-1] >= BLACK_PAWN && chessPieces[pos.y-1][pos.x-1] <=BLACK_KING){
        if(pos.x - 1 == -1 || pos.y - 1 == -1){}else{
          sbit|=  ( 1UL << pos.x  -1+ (pos.y-1) * 8);
        }

      } if(chessPieces[pos.y+1][pos.x-1] == 0 || chessPieces[pos.y+1][pos.x-1] >= BLACK_PAWN && chessPieces[pos.y+1][pos.x-1] <=BLACK_KING){
        if(pos.x - 1 == -1 || pos.y + 1 == 4){}else{
          sbit|=  ( 1UL << pos.x -1 + (pos.y+1) * 8);
        }
      }
      return sbit;
    }

//BLACK turn
  //******************************************************************************
  }else if(turn == BLACK_TURN){
  if(piece == BLACK_PAWN){
        //Speciale case, wanneer pion eerste zet speelt
      //Speciale case, wanneer pion eerste zet speelt
        if(chessPieces[pos.y][pos.x-1] == 0 && chessPieces[pos.y][pos.x-2] == 0 && pos.x == 6){
          sbit |= ( 1UL << pos.x - 1 + pos.y * 8| 1UL << pos.x - 2+ pos.y * 8);
        }else if(chessPieces[pos.y][pos.x-1] == 0){
          sbit |= ( 1UL << pos.x - 1 + pos.y * 8 );
        }
        if(chessPieces[pos.y+1][pos.x-1] >= WHITE_PAWN && chessPieces[pos.y+1][pos.x-1] <= WHITE_KING){
          if(pos.y + 1 == 8 || pos.x - 1 ==-1){}else{
            sbit |= ( 1UL << pos.x - 1 + (pos.y+1) * 8 );
          }
        }
        if(chessPieces[pos.y-1][pos.x-1] >= WHITE_PAWN && chessPieces[pos.y-1][pos.x-1] <= WHITE_KING){
          if(pos.y - 1 == -1 || pos.x -1 ==-1){}else{
            sbit |= ( 1UL << pos.x - 1 + (pos.y-1) * 8 );
          }
        }
      return sbit;  
  }else if(piece == BLACK_ROOK){
        for(int i = 1; i < 8; i++){
          if(pos.x+i == 8){
            break; 
          }
          if(chessPieces[pos.y][pos.x+i] == 0){
            sbit |= (1UL << pos.x+i + pos.y * 8) ;
          }else if(chessPieces[pos.y][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y][pos.x+i] <= WHITE_KING){
            sbit |= (1UL << pos.x+i + pos.y * 8) ;
            break;
          }else{
            break;
          }
        }
        for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.y][pos.x-i] == 0){
            sbit |= (1UL << pos.x-i + pos.y * 8);
          }else if(chessPieces[pos.y][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y][pos.x-i] <= WHITE_KING){
            sbit |= (1UL << pos.x-i + pos.y * 8);
            break;
          }else{
            break;
          }
        }
        for(int i = 1; i < 4; i++){
          if(pos.y == 3){
            break;
          }
          if(chessPieces[pos.y +i][pos.x] == 0){
            sbit |= (1UL << pos.x + 8 *(pos.y+i));
          }else if(chessPieces[pos.y + i][pos.x] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x] <= WHITE_KING){
            sbit |= (1UL << pos.x + 8 *(pos.y +i));
            break;
          }
        }
        for(int i = 1; i < pos.y +1; i++){
          if(chessPieces[pos.y - i][pos.x] == 0){
            sbit |= (1UL << pos.x + 8 * (pos.y -i));
          }else if(chessPieces[pos.y - i][pos.x] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x] <= WHITE_PAWN){
            sbit |= (1UL << pos.x + 8 * (pos.y -i));
            break;
          }
        }
        return sbit;
    }else if(piece == BLACK_BISHOP){
      for(int i = 1; i < 4 - pos.y && pos.x + i != 8; i++){
          if(chessPieces[pos.y+i][pos.x+i] == 0 ){
            sbit |= (1UL << pos.x+i + 8 *(pos.y+i) );
          }else if(chessPieces[pos.y+i][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x+i] <= WHITE_KING){
            sbit |= (1UL << pos.x+i + 8 *(pos.y+i) );
            break; 
          }else{break;}
        }
        for(int i = 1; i < pos.y +1  && pos.x  != 0; i++){
          if(chessPieces[pos.y-i][pos.x-i] == 0 ){
            if(pos.x - i == -1 ){
              break;
            }
            sbit |= (1UL << pos.x-i + 8 *(pos.y-i) );
          }else if(chessPieces[pos.y-i][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x-i] <= WHITE_KING){
            sbit |= (1UL << pos.x-i + 8 *(pos.y-i) );
            break; 
          }else{break;}
        }

         
        //DEze klopt wss niet 
        for(int i = 1; i < pos.y +1 && pos.x + i != 8 ; i++){
          if(chessPieces[pos.y-i][pos.x+i] == 0 ){
            sbit |= (1UL << pos.x+i + 8 *(pos.y-i) );
          }else if(chessPieces[pos.y-i][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x+i] <= WHITE_KING){
            sbit |= (1UL << pos.x+i + 8 *(pos.y-i) );
            break; 
          }else{break;}
        }
        
        for(int i = 1; i <  4 -pos.y && pos.x != 0 ; i++){
          if(pos.x - i == -1 ){
              break;
            }
          if(chessPieces[pos.y+i][pos.x-i] == 0 ){
            sbit |= (1UL << pos.x-i + 8 *(pos.y+i) );
          }else if(chessPieces[pos.y+i][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x-i] <= WHITE_KING){
            sbit |= (1UL << pos.x-i + 8 *(pos.y+i) );
            break; 
          }else{
            break;
          }
        }
       
        return sbit;
    }else if(piece == BLACK_QUEEN){
      for(int i = 1; i < 8; i++){
          if(pos.x+i == 8){
            break; 
          }
          if(chessPieces[pos.y][pos.x+i] == 0){
            sbit |= (1UL << pos.x+i + pos.y * 8) ;
          }else if(chessPieces[pos.y][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y][pos.x+i] <= WHITE_KING){
            sbit |= (1UL << pos.x+i + pos.y * 8) ;
            break;
          }else{
            break;
          }
        }
        for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.y][pos.x-i] == 0){
            sbit |= (1UL << pos.x-i + pos.y * 8);
          }else if(chessPieces[pos.y][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y][pos.x-i] <= WHITE_KING){
            sbit |= (1UL << pos.x-i + pos.y * 8);
            break;
          }else{
            break;
          }
        }
        for(int i = 1; i < 4; i++){
          if(pos.y == 3){
            break;
          }
          if(chessPieces[pos.y +i][pos.x] == 0){
            sbit |= (1UL << pos.x + 8 *(pos.y+i));
          }else if(chessPieces[pos.y + i][pos.x] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x] <= WHITE_KING){
            sbit |= (1UL << pos.x + 8 *(pos.y +i));
            break;
          }else{
            break;
          }
        }
        for(int i = 1; i < pos.y +1; i++){
          if(chessPieces[pos.y - i][pos.x] == 0){
            sbit |= (1UL << pos.x + 8 * (pos.y -i));
          }else if(chessPieces[pos.y - i][pos.x] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x] <= WHITE_PAWN){
            sbit |= (1UL << pos.x + 8 * (pos.y -i));
            break;
          }else{
            break;
          }
        }
        for(int i = 1; i < 4 - pos.y && pos.x + i != 8; i++){
          if(chessPieces[pos.y+i][pos.x+i] == 0 ){
            sbit |= (1UL << pos.x+i + 8 *(pos.y+i) );
          }else if(chessPieces[pos.y+i][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x+i] <= WHITE_KING){
            sbit |= (1UL << pos.x+i + 8 *(pos.y+i) );
            break; 
          }else{
            break;
          }
        }
        for(int i = 1; i < pos.y +1  && pos.x  != 0; i++){
          if(chessPieces[pos.y-i][pos.x-i] == 0 ){
            if(pos.x - i == -1 ){
              break;
            }
            sbit |= (1UL << pos.x-i + 8 *(pos.y-i) );
          }else if(chessPieces[pos.y-i][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x-i] <= WHITE_KING){
            sbit |= (1UL << pos.x-i + 8 *(pos.y-i) );
            break; 
          }else{
            break;
          }
        }

         
        //DEze klopt wss niet 
        for(int i = 1; i < pos.y +1 && pos.x + i != 8 ; i++){
          if(chessPieces[pos.y-i][pos.x+i] == 0 ){
            sbit |= (1UL << pos.x+i + 8 *(pos.y-i) );
          }else if(chessPieces[pos.y-i][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x+i] <= WHITE_KING){
            sbit |= (1UL << pos.x+i + 8 *(pos.y-i) );
            break; 
          }else{
            break;
          }
        }
        
        for(int i = 1; i <  4 -pos.y && pos.x != 0 ; i++){
          if(pos.x - i == -1 ){
              break;
            }
          if(chessPieces[pos.y+i][pos.x-i] == 0 ){
            sbit |= (1UL << pos.x-i + 8 *(pos.y+i) );
          }else if(chessPieces[pos.y+i][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x-i] <= WHITE_KING){
            sbit |= (1UL << pos.x-i + 8 *(pos.y+i) );
            break; 
          }else{
            break;
          }
        }
       
        return sbit;
    }else if(piece == BLACK_KING){
      if(chessPieces[pos.y][pos.x+1] == 0 ||  chessPieces[pos.y][pos.x+1] >= WHITE_PAWN && chessPieces[pos.y][pos.x+1] <=WHITE_KING ){
        if(pos.x + 1 == 8){}else{
          sbit|=  ( 1UL << pos.x + 1 + pos.y * 8);
        }
      }
      if(chessPieces[pos.y][pos.x-1] == 0 || chessPieces[pos.y][pos.x-1] >= WHITE_PAWN && chessPieces[pos.y][pos.x-1] <=WHITE_KING ){
        if(pos.x - 1 == -1){}else{
          sbit|=  ( 1UL << pos.x - 1 + pos.y * 8);
        }
      }
      if(chessPieces[pos.y+1][pos.x] == 0 || chessPieces[pos.y+1][pos.x] >= WHITE_PAWN && chessPieces[pos.y+1][pos.x] <=WHITE_KING){
        if(pos.y + 1 == 4){}else{
          sbit|=  ( 1UL << pos.x + (pos.y+1) * 8);
        }
      }
      if(chessPieces[pos.y-1][pos.x] == 0 || chessPieces[pos.y-1][pos.x] >= WHITE_PAWN && chessPieces[pos.y-1][pos.x] <=WHITE_KING){
        if(pos.y - 1 == -1){}else{
          sbit|=  ( 1UL << pos.x + (pos.y-1) * 8);
        }
      }
      if(chessPieces[pos.y+1][pos.x+1] == 0 || chessPieces[pos.y+1][pos.x+1] >= WHITE_PAWN && chessPieces[pos.y+1][pos.x+1] <=WHITE_KING){
        if(pos.x + 1 == 8 || pos.y + 1 == 4){}else{
          sbit|=  ( 1UL << pos.x +1 + (pos.y+1) * 8);
        }
      }
      if(chessPieces[pos.y-1][pos.x+1] == 0 || chessPieces[pos.y-1][pos.x+1] >= WHITE_PAWN && chessPieces[pos.y-1][pos.x+1] <=WHITE_KING){
        if(pos.x + 1 == 8 || pos.y - 1 == -1){}else{
          sbit|=  ( 1UL << pos.x +1 + (pos.y-1) * 8);
        }

      }
      if(chessPieces[pos.y-1][pos.x-1] == 0 || chessPieces[pos.y-1][pos.x-1] >= WHITE_PAWN && chessPieces[pos.y-1][pos.x-1] <=WHITE_KING){
        if(pos.x - 1 == -1 || pos.y - 1 == -1){}else{
          sbit|=  ( 1UL << pos.x  -1+ (pos.y-1) * 8);
        }

      } if(chessPieces[pos.y+1][pos.x-1] == 0 || chessPieces[pos.y+1][pos.x-1] >= WHITE_PAWN && chessPieces[pos.y+1][pos.x-1] <=WHITE_KING){
        if(pos.x - 1 == -1 || pos.y + 1 == 4){}else{
          sbit|=  ( 1UL << pos.x -1 + (pos.y+1) * 8);
        }
      }
      return sbit;
    }else{
      return sbit;
    }
  }
   
 
}
//Kijkt als de schaakstuk geldig gezet werd 
struct coordinate checkMove(uint8_t piece, struct coordinate pos){
  //WHITE turn
  //******************************************************************************
 if(turn == WHITE_TURN){
   
   if(piece == WHITE_PAWN){
      if(hallSensor[pos.y][pos.x+1] == false && chessPieces[pos.y][pos.x+1] == 0){
        isPlayed = 1;
        isLifted = 0;
        pos.x +=1;
        return pos;
      }else if(hallSensor[pos.y][pos.x+2] == false && chessPieces[pos.y][pos.x+2] == 0 && pos.x == 1){
       isPlayed = 1;
       isLifted = 0;
        pos.x += 2;
       return pos;
    }else if(chessPieces[pos.y-1][pos.x+1] >= BLACK_PAWN && chessPieces[pos.y-1][pos.x+1] <= BLACK_KING){
             if(hallSensor[pos.y-1][pos.x+1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=1;
                pos.y-=1;
              return pos;
            }
    }else if(chessPieces[pos.y+1][pos.x+1] >= BLACK_PAWN && chessPieces[pos.y+1][pos.x+1] <= BLACK_KING){
             if(hallSensor[pos.y+1][pos.x+1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=1;
                pos.y+=1;
              return pos;
            }
    }
   }else if(piece == WHITE_ROOK){
     for(int i = 1; i < 8-pos.x; i++){
          if(chessPieces[pos.y][pos.x+i] == 0 && hallSensor[pos.y][pos.x+i] == false){
            isPlayed = 1;
            isLifted = 0;
           pos.x+=i;
            return pos;
          }else if(chessPieces[pos.y][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y][pos.x+i] <=BLACK_KING){
            if(hallSensor[pos.y][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x +=i;
            return pos;
            }
          }
     }

     for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.y][pos.x-i] == 0 && hallSensor[pos.y][pos.x-i] == false){
            isPlayed = 1;
            isLifted = 0;
           pos.x -=i;
            return pos;
          }else if(chessPieces[pos.y][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y][pos.x-i] <=BLACK_KING){
            if(hallSensor[pos.y][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x-=i;
            return pos;
            }
          }
      
     } 
     for(int i = 1; i <  4;i++){
       if(pos.y + i == 4){
            break;
          }
        if(chessPieces[pos.y + i][pos.x] == 0 && hallSensor[pos.y + i][pos.x] == false){
          
          isPlayed = 1;
          isLifted = 0;
           pos.y +=i;
          return pos;
        }else if(chessPieces[pos.y +i][pos.x] >= BLACK_PAWN && chessPieces[pos.y +i][pos.x] <=BLACK_KING){
          if(hallSensor[pos.y+i][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y+=i;
            return pos;
            }
        }
      }
      for(int i = 1; i < pos.y + 1; i++){
        if(chessPieces[pos.y -i][pos.x] == 0 && hallSensor[pos.y - i][pos.x] == false){
          
          isPlayed = 1;
          isLifted = 0;
           pos.y -=i;
          return pos;
        }else if(chessPieces[pos.y -i][pos.x] >= BLACK_PAWN && chessPieces[pos.y -i][pos.x] <=BLACK_KING){
          if(hallSensor[pos.y-i][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y-=i;
            return pos;
            }
        }
      }
  }else if(piece == WHITE_BISHOP){
      for(int i = 1; i < 4 - pos.y  && pos.x + i != 8; i++){
          if(chessPieces[pos.y+i][pos.x+i] == 0 && hallSensor[pos.y + i][pos.x+i] == false){
            isPlayed = 1;
              isLifted = 0;
              pos.x+=i;
              pos.y+=i;
              return pos;
          }else if(chessPieces[pos.y+i][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x+i] <= BLACK_KING){
            if(hallSensor[pos.y+i][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=i;
                pos.y+=i;
              return pos;
            }
          }
        }
        for(int i = 1; i < pos.y +1; i++){
          if(chessPieces[pos.y-i][pos.x-i] == 0 && hallSensor[pos.y - i][pos.x-i] == false){
             isPlayed = 1;
              isLifted = 0;
              pos.x-=i;
              pos.y-=i;
              return pos;
          }else if(chessPieces[pos.y-i][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x-i] <= BLACK_KING){
            if(hallSensor[pos.y-i][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=i;
                pos.y-=i;
              return pos;
            }
            
          }
        }

         
        //DEze klopt wss niet 
        for(int i = 1; i < pos.y +1 && pos.x + i != 8 ; i++){
          if(chessPieces[pos.y-i][pos.x+i] == 0 && hallSensor[pos.y - i][pos.x+i] == false){
            isPlayed = 1;
              isLifted = 0;
              pos.x+=i;
              pos.y-=i;
              return pos;
          }else if(chessPieces[pos.y-i][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x+i] <= BLACK_KING){
            if(hallSensor[pos.y-i][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=i;
                pos.y-=i;
              return pos;
            }
          }
        }
        
        for(int i = 1; i < 4 - pos.y ; i++){
          if(chessPieces[pos.y+i][pos.x-i] == 0 && hallSensor[pos.y + i][pos.x-i] == false ){
           isPlayed = 1;
              isLifted = 0;
              pos.x-=i;
              pos.y+=i;
              return pos;
          }else if(chessPieces[pos.y+i][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x-i] <= BLACK_KING){
            if(hallSensor[pos.y+i][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=i;
                pos.y+= i;
              return pos;
            }
          }
        }
  }else if(piece == WHITE_QUEEN){
      for(int i = 1; i < 4 - pos.y  && pos.x + i != 8; i++){
          if(chessPieces[pos.y+i][pos.x+i] == 0 && hallSensor[pos.y + i][pos.x+i] == false){
            isPlayed = 1;
              isLifted = 0;
              pos.x+=i;
              pos.y+=i;
              return pos;
          }else if(chessPieces[pos.y+i][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x+i] <= BLACK_KING){
            if(hallSensor[pos.y+i][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=i;
                pos.y+=i;
              return pos;
            }
          }
        }
        for(int i = 1; i < pos.y +1; i++){
          if(chessPieces[pos.y-i][pos.x-i] == 0 && hallSensor[pos.y - i][pos.x-i] == false){
             isPlayed = 1;
              isLifted = 0;
              pos.x-=i;
              pos.y-=i;
              return pos;
          }else if(chessPieces[pos.y-i][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x-i] <= BLACK_KING){
            if(hallSensor[pos.y-i][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=i;
                pos.y-=i;
              return pos;
            }
            
          }
        }

         
        //DEze klopt wss niet 
        for(int i = 1; i < pos.y +1 && pos.x + i != 8 ; i++){
          if(chessPieces[pos.y-i][pos.x+i] == 0 && hallSensor[pos.y - i][pos.x+i] == false){
            isPlayed = 1;
              isLifted = 0;
              pos.x+=i;
              pos.y-=i;
              return pos;
          }else if(chessPieces[pos.y-i][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y-i][pos.x+i] <= BLACK_KING){
            if(hallSensor[pos.y-i][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=i;
                pos.y-=i;
              return pos;
            }
          }
        }
        
        for(int i = 1; i < 4 - pos.y ; i++){
          if(chessPieces[pos.y+i][pos.x-i] == 0 && hallSensor[pos.y + i][pos.x-i] == false ){
           isPlayed = 1;
              isLifted = 0;
              pos.x-=i;
              pos.y+=i;
              return pos;
          }else if(chessPieces[pos.y+i][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y+i][pos.x-i] <= BLACK_KING){
            if(hallSensor[pos.y+i][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=i;
                pos.y+= i;
              return pos;
            }
          }
        }
        for(int i = 1; i < 8-pos.x; i++){
          if(chessPieces[pos.y][pos.x+i] == 0 && hallSensor[pos.y][pos.x+i] == false){
            isPlayed = 1;
            isLifted = 0;
           pos.x+=i;
            return pos;
          }else if(chessPieces[pos.y][pos.x+i] >= BLACK_PAWN && chessPieces[pos.y][pos.x+i] <=BLACK_KING){
            if(hallSensor[pos.y][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x +=i;
            return pos;
            }
          }
     }

     for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.y][pos.x-i] == 0 && hallSensor[pos.y][pos.x-i] == false){
            isPlayed = 1;
            isLifted = 0;
           pos.x -=i;
            return pos;
          }else if(chessPieces[pos.y][pos.x-i] >= BLACK_PAWN && chessPieces[pos.y][pos.x-i] <=BLACK_KING){
            if(hallSensor[pos.y][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x-=i;
            return pos;
            }
          }
      
     } 
     for(int i = 1; i <  4;i++){
       if(pos.y + i == 4){
            break;
          }
        if(chessPieces[pos.y + i][pos.x] == 0 && hallSensor[pos.y + i][pos.x] == false){
          
          isPlayed = 1;
          isLifted = 0;
           pos.y +=i;
          return pos;
        }else if(chessPieces[pos.y +i][pos.x] >= BLACK_PAWN && chessPieces[pos.y +i][pos.x] <=BLACK_KING){
          if(hallSensor[pos.y+i][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y+=i;
            return pos;
            }
        }
      }
      for(int i = 1; i < pos.y + 1; i++){
        if(chessPieces[pos.y -i][pos.x] == 0 && hallSensor[pos.y - i][pos.x] == false){
          
          isPlayed = 1;
          isLifted = 0;
           pos.y -=i;
          return pos;
        }else if(chessPieces[pos.y -i][pos.x] >= BLACK_PAWN && chessPieces[pos.y -i][pos.x] <=BLACK_KING){
          if(hallSensor[pos.y-i][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y-=i;
            return pos;
            }
        }
      }
  }else if(WHITE_KING){
      if(chessPieces[pos.y][pos.x+1] == 0 && hallSensor[pos.y ][pos.x+1] == false){
        if(pos.x + 1 == 8){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x+=1;
            return pos;
        }
      }else if( chessPieces[pos.y][pos.x+1] >= BLACK_PAWN && chessPieces[pos.y][pos.x+1] <=BLACK_KING){
        if(hallSensor[pos.y][pos.x+1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x +=1;
            return pos;
            }
      }

      if(chessPieces[pos.y][pos.x-1] == 0 && hallSensor[pos.y ][pos.x-1] == false){
        if(pos.x - 1 == -1){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x-=1;
            return pos;
        }
      }else if( chessPieces[pos.y][pos.x-1] >= BLACK_PAWN && chessPieces[pos.y][pos.x-1] <=BLACK_KING){
        if(hallSensor[pos.y][pos.x-1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x -=1;
            return pos;
            }
      }
      if(chessPieces[pos.y+1][pos.x] == 0 && hallSensor[pos.y + 1][pos.x] == false){
        if(pos.y + 1 == 4){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.y+=1;
            return pos;
        }
      }else if( chessPieces[pos.y+1][pos.x] >= BLACK_PAWN && chessPieces[pos.y+1][pos.x] <=BLACK_KING){
        if(hallSensor[pos.y+1][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
             pos.y+=1;
            return pos;
            }
      }
      if(chessPieces[pos.y-1][pos.x] == 0 && hallSensor[pos.y - 1][pos.x] == false){
        if(pos.y - 1 == -1){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.y-=1;
            return pos;
        }
      }else if( chessPieces[pos.y-1][pos.x] >= BLACK_PAWN && chessPieces[pos.y-1][pos.x] <=BLACK_KING){
        if(hallSensor[pos.y-1][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y -=1;
            return pos;
            }
      }
      if(chessPieces[pos.y+1][pos.x+1] == 0 && hallSensor[pos.y + 1][pos.x+1] == false){
        if(pos.x + 1 == 8 || pos.y + 1 == 4){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x+=1;
           pos.y+=1;
            return pos;
        }
      }else if( chessPieces[pos.y+1][pos.x+1] >= BLACK_PAWN && chessPieces[pos.y+1][pos.x+1] <=BLACK_KING){
        if(hallSensor[pos.y+1][pos.x+1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x +=1;
            pos.y +=1;
            return pos;
            }
      }
      if(chessPieces[pos.y-1][pos.x+1] == 0 && hallSensor[pos.y -1 ][pos.x + 1] == false){
        if(pos.x + 1 == 8 || pos.y - 1 == -1){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x+=1;
           pos.y-=1;
            return pos;
        }

      }else if(chessPieces[pos.y-1][pos.x+1]  >= BLACK_PAWN && chessPieces[pos.y-1][pos.x+1] <=BLACK_KING){
        if(hallSensor[pos.y-1][pos.x+1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x+=1;
           pos.y-=1;
            return pos;
            }
      }
      if(chessPieces[pos.y-1][pos.x-1] == 0 && hallSensor[pos.y - 1][pos.x - 1] == false){
        if(pos.x - 1 == -1 || pos.y - 1 == -1){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x-=1;
           pos.y-=1;
            return pos;
        }

      }else if(chessPieces[pos.y-1][pos.x-1]  >= BLACK_PAWN && chessPieces[pos.y-1][pos.x-1]<=BLACK_KING){
        if(hallSensor[pos.y-1][pos.x-1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x-=1;
           pos.y-=1;
            return pos;
            }
      }
      if(chessPieces[pos.y+1][pos.x-1] == 0&& hallSensor[pos.y + 1][pos.x -1] == false){
        if(pos.x - 1 == -1 || pos.y + 1 == 4){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x-=1;
           pos.y+=1;
            return pos;
        }
      }else if( chessPieces[pos.y+1][pos.x-1] >= BLACK_PAWN && chessPieces[pos.y +1][pos.x-1] <=BLACK_KING){
        if(hallSensor[pos.y+1][pos.x-1] == HIGH){
          isPlayed = 1;
              isLifted = 1;
              pos.x-=1;
           pos.y+=1;
            pos.x -=1;
            return pos;
            }
      }
    }else{
      return pos;
    }
  
  //BLACK turn
  //******************************************************************************
 }else if(turn == BLACK_TURN){
  if(piece == BLACK_PAWN){
    
      if(hallSensor[pos.y][pos.x-1] == false && chessPieces[pos.y][pos.x-1] == 0){

          isPlayed = 1;
          isLifted = 0;
          
          pos.x-=1;
          
          return pos;
        }else if(hallSensor[pos.y][pos.x-2] == false && chessPieces[pos.y][pos.x-2] == 0 && pos.x == 6){
         
          isPlayed = 1;
          isLifted = 0;
          pos.x -=2;
          return pos;
        }else if(chessPieces[pos.y-1][pos.x-1] >= WHITE_PAWN && chessPieces[pos.y-1][pos.x-1] <= WHITE_KING){
             if(hallSensor[pos.y-1][pos.x-1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=1;
                pos.y-=1;
              return pos;
            }
    }else if(chessPieces[pos.y+1][pos.x-1] >= WHITE_PAWN && chessPieces[pos.y+1][pos.x-1] <= WHITE_KING){
             if(hallSensor[pos.y+1][pos.x-1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=1;
                pos.y+=1;
              return pos;
            }
    }
  }else if(piece == BLACK_ROOK){
     for(int i = 1; i < 8-pos.x; i++){
          if(chessPieces[pos.y][pos.x+i] == 0 && hallSensor[pos.y][pos.x+i] == false){
            
            isPlayed = 1;
            isLifted = 0;
             pos.x+=i;
            return pos;
          }else if(chessPieces[pos.y][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y][pos.x+i] <=WHITE_KING){
            if(hallSensor[pos.y][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
             pos.x+=i;
            return pos;
            }
          }
      }
     for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.y][pos.x-i] == 0 && hallSensor[pos.y][pos.x-i] == false){
            
            isPlayed = 1;
            isLifted = 0;
             pos.x-=i;
            return pos;
          }else if(chessPieces[pos.y][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y][pos.x-i] <=WHITE_KING){
            if(hallSensor[pos.y][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
             pos.x-=i;
            return pos;
            }
         } 
     }
     for(int i = 1; i <  4;i++){
       if(pos.y + i == 4){
            break;
          }
        if(chessPieces[pos.y + i][pos.x] == 0 && hallSensor[pos.y + i][pos.x] == false){
          
          isPlayed = 1;
          isLifted = 0;
           pos.y +=i;
          return pos;
        }else if(chessPieces[pos.y +i][pos.x] >= WHITE_PAWN && chessPieces[pos.y +i][pos.x] <=WHITE_KING){
          if(hallSensor[pos.y+i][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y+=i;
            return pos;
            }
        }
      }
      for(int i = 1; i < pos.y + 1; i++){
        if(chessPieces[pos.y -i][pos.x] == 0 && hallSensor[pos.y - i][pos.x] == false){
          
          isPlayed = 1;
          isLifted = 0;
           pos.y -=i;
          return pos;
        }else if(chessPieces[pos.y -i][pos.x] >= WHITE_PAWN && chessPieces[pos.y -i][pos.x] <=WHITE_KING){
          if(hallSensor[pos.y-i][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y-=i;
            return pos;
            }
        }
      }
    }else if(piece == BLACK_BISHOP){
      for(int i = 1; i < 4 - pos.y  && pos.x + i != 8; i++){
          if(chessPieces[pos.y+i][pos.x+i] == 0 && hallSensor[pos.y + i][pos.x+i] == false){
            isPlayed = 1;
              isLifted = 0;
              pos.x+=i;
              pos.y+=i;
              return pos;
          }else if(chessPieces[pos.y+i][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x+i] <= WHITE_KING){
            if(hallSensor[pos.y+i][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=i;
                pos.y+=i;
              return pos;
            }
          }
        }
        for(int i = 1; i < pos.y +1; i++){
          if(chessPieces[pos.y-i][pos.x-i] == 0 && hallSensor[pos.y - i][pos.x-i] == false){
             isPlayed = 1;
              isLifted = 0;
              pos.x-=i;
              pos.y-=i;
              return pos;
          }else if(chessPieces[pos.y-i][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x-i] <= WHITE_KING){
            if(hallSensor[pos.y-i][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=i;
                pos.y-=i;
              return pos;
            }
            
          }
        }

         
        //DEze klopt wss niet 
        for(int i = 1; i < pos.y +1 && pos.x + i != 8 ; i++){
          if(chessPieces[pos.y-i][pos.x+i] == 0 && hallSensor[pos.y - i][pos.x+i] == false){
            isPlayed = 1;
              isLifted = 0;
              pos.x+=i;
              pos.y-=i;
              return pos;
          }else if(chessPieces[pos.y-i][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x+i] <= WHITE_KING){
            if(hallSensor[pos.y-i][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=i;
                pos.y-=i;
              return pos;
            }
          }
        }
        
        for(int i = 1; i < 4 - pos.y ; i++){
          if(chessPieces[pos.y+i][pos.x-i] == 0 && hallSensor[pos.y + i][pos.x-i] == false ){
           isPlayed = 1;
              isLifted = 0;
              pos.x-=i;
              pos.y+=i;
              return pos;
          }else if(chessPieces[pos.y+i][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x-i] <= WHITE_KING){
            if(hallSensor[pos.y+i][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=i;
                pos.y+= i;
              return pos;
            }
          }
        }
  }else if(piece == BLACK_QUEEN){
       for(int i = 1; i < 8-pos.x; i++){
          if(chessPieces[pos.y][pos.x+i] == 0 && hallSensor[pos.y][pos.x+i] == false){
            
            isPlayed = 1;
            isLifted = 0;
             pos.x+=i;
            return pos;
          }else if(chessPieces[pos.y][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y][pos.x+i] <=WHITE_KING){
            if(hallSensor[pos.y][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
             pos.x+=i;
            return pos;
            }
          }
      }
     for(int i = 1; i < pos.x+1; i++){
          if(chessPieces[pos.y][pos.x-i] == 0 && hallSensor[pos.y][pos.x-i] == false){
            
            isPlayed = 1;
            isLifted = 0;
             pos.x-=i;
            return pos;
          }else if(chessPieces[pos.y][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y][pos.x-i] <=WHITE_KING){
            if(hallSensor[pos.y][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
             pos.x-=i;
            return pos;
            }
         } 
     }
     for(int i = 1; i <  4;i++){
       if(pos.y + i == 4){
            break;
          }
        if(chessPieces[pos.y + i][pos.x] == 0 && hallSensor[pos.y + i][pos.x] == false){
          
          isPlayed = 1;
          isLifted = 0;
           pos.y +=i;
          return pos;
        }else if(chessPieces[pos.y +i][pos.x] >= WHITE_PAWN && chessPieces[pos.y +i][pos.x] <=WHITE_KING){
          if(hallSensor[pos.y+i][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y+=i;
            return pos;
            }
        }
      }
      for(int i = 1; i < pos.y + 1; i++){
        if(chessPieces[pos.y -i][pos.x] == 0 && hallSensor[pos.y - i][pos.x] == false){
          
          isPlayed = 1;
          isLifted = 0;
           pos.y -=i;
          return pos;
        }else if(chessPieces[pos.y -i][pos.x] >= WHITE_PAWN && chessPieces[pos.y -i][pos.x] <=WHITE_KING){
          if(hallSensor[pos.y-i][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y-=i;
            return pos;
            }
        }
      }
      for(int i = 1; i < 4 - pos.y  && pos.x + i != 8; i++){
          if(chessPieces[pos.y+i][pos.x+i] == 0 && hallSensor[pos.y + i][pos.x+i] == false){
            isPlayed = 1;
              isLifted = 0;
              pos.x+=i;
              pos.y+=i;
              return pos;
          }else if(chessPieces[pos.y+i][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x+i] <= WHITE_KING){
            if(hallSensor[pos.y+i][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=i;
                pos.y+=i;
              return pos;
            }
          }
        }
        for(int i = 1; i < pos.y +1; i++){
          if(chessPieces[pos.y-i][pos.x-i] == 0 && hallSensor[pos.y - i][pos.x-i] == false){
             isPlayed = 1;
              isLifted = 0;
              pos.x-=i;
              pos.y-=i;
              return pos;
          }else if(chessPieces[pos.y-i][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x-i] <= WHITE_KING){
            if(hallSensor[pos.y-i][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=i;
                pos.y-=i;
              return pos;
            }
            
          }
        }

         
        //DEze klopt wss niet 
        for(int i = 1; i < pos.y +1 && pos.x + i != 8 ; i++){
          if(chessPieces[pos.y-i][pos.x+i] == 0 && hallSensor[pos.y - i][pos.x+i] == false){
            isPlayed = 1;
              isLifted = 0;
              pos.x+=i;
              pos.y-=i;
              return pos;
          }else if(chessPieces[pos.y-i][pos.x+i] >= WHITE_PAWN && chessPieces[pos.y-i][pos.x+i] <= WHITE_KING){
            if(hallSensor[pos.y-i][pos.x+i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x+=i;
                pos.y-=i;
              return pos;
            }
          }
        }
        
        for(int i = 1; i < 4 - pos.y ; i++){
          if(chessPieces[pos.y+i][pos.x-i] == 0 && hallSensor[pos.y + i][pos.x-i] == false ){
           isPlayed = 1;
              isLifted = 0;
              pos.x-=i;
              pos.y+=i;
              return pos;
          }else if(chessPieces[pos.y+i][pos.x-i] >= WHITE_PAWN && chessPieces[pos.y+i][pos.x-i] <= WHITE_KING){
            if(hallSensor[pos.y+i][pos.x-i] == HIGH){
              isPlayed = 1;
              isLifted = 1;
              pos.x-=i;
                pos.y+= i;
              return pos;
            }
          }
        }
  }else if (piece == BLACK_KING){
     if(chessPieces[pos.y][pos.x+1] == 0 && hallSensor[pos.y ][pos.x+1] == false){
        if(pos.x + 1 == 8){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x+=1;
            return pos;
        }
      }else if( chessPieces[pos.y][pos.x+1] >= WHITE_PAWN && chessPieces[pos.y][pos.x+1] <=WHITE_KING){
        if(hallSensor[pos.y][pos.x+1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x +=1;
            return pos;
            }
      }

      if(chessPieces[pos.y][pos.x-1] == 0 && hallSensor[pos.y ][pos.x-1] == false){
        if(pos.x - 1 == -1){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x-=1;
            return pos;
        }
      }else if( chessPieces[pos.y][pos.x-1] >= WHITE_PAWN && chessPieces[pos.y][pos.x-1] <=WHITE_KING){
        if(hallSensor[pos.y][pos.x-1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x -=1;
            return pos;
            }
      }
      if(chessPieces[pos.y+1][pos.x] == 0 && hallSensor[pos.y + 1][pos.x] == false){
        if(pos.y + 1 == 4){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.y+=1;
            return pos;
        }
      }else if( chessPieces[pos.y+1][pos.x] >= WHITE_PAWN && chessPieces[pos.y+1][pos.x] <=WHITE_KING){
        if(hallSensor[pos.y+1][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
             pos.y+=1;
            return pos;
            }
      }
      if(chessPieces[pos.y-1][pos.x] == 0 && hallSensor[pos.y - 1][pos.x] == false){
        if(pos.y - 1 == -1){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.y-=1;
            return pos;
        }
      }else if( chessPieces[pos.y-1][pos.x] >= WHITE_PAWN && chessPieces[pos.y-1][pos.x] <=WHITE_KING){
        if(hallSensor[pos.y-1][pos.x] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.y -=1;
            return pos;
            }
      }
      if(chessPieces[pos.y+1][pos.x+1] == 0 && hallSensor[pos.y + 1][pos.x+1] == false){
        if(pos.x + 1 == 8 || pos.y + 1 == 4){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x+=1;
           pos.y+=1;
            return pos;
        }
      }else if( chessPieces[pos.y+1][pos.x+1] >= WHITE_PAWN && chessPieces[pos.y+1][pos.x+1] <=WHITE_KING){
        if(hallSensor[pos.y+1][pos.x+1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x +=1;
            pos.y +=1;
            return pos;
            }
      }
      if(chessPieces[pos.y-1][pos.x+1] == 0 && hallSensor[pos.y -1 ][pos.x + 1] == false){
        if(pos.x + 1 == 8 || pos.y - 1 == -1){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x+=1;
           pos.y-=1;
            return pos;
        }

      }else if(chessPieces[pos.y-1][pos.x+1]  >= WHITE_PAWN && chessPieces[pos.y-1][pos.x+1] <=WHITE_KING){
        if(hallSensor[pos.y-1][pos.x+1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x+=1;
           pos.y-=1;
            return pos;
            }
      }
      if(chessPieces[pos.y-1][pos.x-1] == 0 && hallSensor[pos.y - 1][pos.x - 1] == false){
        if(pos.x - 1 == -1 || pos.y - 1 == -1){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x-=1;
           pos.y-=1;
            return pos;
        }

      }else if(chessPieces[pos.y-1][pos.x-1]  >= WHITE_PAWN && chessPieces[pos.y-1][pos.x-1]<=WHITE_KING){
        if(hallSensor[pos.y-1][pos.x-1] == HIGH){
              isPlayed = 1;
              isLifted = 1;
            pos.x-=1;
           pos.y-=1;
            return pos;
            }
      }
      if(chessPieces[pos.y+1][pos.x-1] == 0&& hallSensor[pos.y + 1][pos.x -1] == false){
        if(pos.x - 1 == -1 || pos.y + 1 == 4){}else{
          isPlayed = 1;
            isLifted = 0;
           pos.x-=1;
           pos.y+=1;
            return pos;
        }
      }else if( chessPieces[pos.y+1][pos.x-1] >= WHITE_PAWN && chessPieces[pos.y +1][pos.x-1] <=WHITE_KING){
        if(hallSensor[pos.y+1][pos.x-1] == HIGH){
          isPlayed = 1;
              isLifted = 1;
              pos.x-=1;
           pos.y+=1;
            pos.x -=1;
            return pos;
            }
      }
  }else{
      return pos;
    }
 }

  return pos;
}
void takeMove(){

}
uint32_t shiftbit = 0; 
coordinate coord;
coordinate temp;

void loop(){
    /*leest de waarden van de hall-effect sensoren */
    readHall();
    /*toestand een*/
    if(isLifted == false && isPlayed == false){
      
      shiftbit = 0;
      for(uint8_t i = 0; i < 4; i++){
          for(uint8_t j = 0; j < 8;j++){
          if(turn == WHITE_TURN){
            if(chessPieces[i][j] >= WHITE_PAWN && chessPieces[i][j] <= WHITE_KING){
              if(hallSensor[i][j] == HIGH){
                coord.x  = j;
                coord.y = i;
                isLifted = true; 
              }
            }
          }else if(turn == BLACK_TURN){
            if(chessPieces[i][j] >= BLACK_PAWN && chessPieces[i][j] <= BLACK_KING){
              if(hallSensor[i][j] == HIGH){
                coord.x  = j;
                coord.y = i;
                isLifted = true; 
              }
            }
          }
        }
      }
    /*toestand twee*/
    }else if(isLifted == true && isPlayed == false){
      
      temp   = checkMove(chessPieces[coord.y][coord.x], coord);
      shiftbit = showMove(chessPieces[coord.y][coord.x],coord);
      //Serial.println(coord.x);
      if(hallSensor[coord.y][coord.x] == LOW){
        isLifted = false; 
        isPlayed = false;
      }
     
    /*toestand drie*/
    }else if(isLifted == true  && isLifted == true ){
      
      if(hallSensor[temp.y][temp.x] == LOW && hallSensor[coord.y][coord.x] == HIGH){
        isLifted = false;
        isPlayed = true; 
      }else if(hallSensor[temp.y][temp.x] == HIGH && hallSensor[coord.y][coord.x] == LOW){
        isLifted =false;
        isPlayed =false;
      }
    /*toestand drie*/
    }else if(isLifted == false && isPlayed == true){
      
      shiftbit = 0;
      if(hallSensor[temp.y][temp.x] == HIGH){
        isLifted = true; 
        isPlayed = false;
        
      }
      
      if(digitalRead(BUTTON) == true){
        isLifted = false;
        isPlayed = false; 
        turn = ((turn == WHITE_TURN) ? BLACK_TURN : WHITE_TURN);
        chessPieces[temp.y][temp.x] = chessPieces[coord.y][coord.x];
        chessPieces[coord.y][coord.x] = 0;
        //Serial.println("has Played");
      }
    }
  
  writeShift(shiftbit);
}








