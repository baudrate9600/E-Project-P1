/*Constantes voor de Shiftregister afkomstig van de datasheet
  http://www.ti.com/lit/ds/symlink/sn74ahc595.pdf
*/
#include "Arduino.h"

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

//Schuift 8 Bits in de shift-register

void writeShift(uint16_t );
void writeShift1(uint16_t );
void writeShift32(uint32_t );
void mux(uint8_t );
