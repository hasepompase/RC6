/*
 *  RC6 Arduino Library
 *  
 */

#ifndef RC6_h
#define RC6_h

#if( ARDUINO >= 100 )
   #include "Arduino.h"
#else
   #include "WProgram.h"
#endif

class RC6
{
private:
    volatile uint32_t edgeTime;
    volatile bool     rxDone;
    uint8_t           rxPin;
    volatile uint8_t  TArray[ 44 ];
    volatile uint8_t  TIdx;
    volatile uint8_t  TSum;


    bool decode( uint32_t* code );
    
  
 public:

         RC6( uint8_t rxPin );
    bool read( uint32_t* code );
    bool read( uint8_t *toggle, uint8_t *mode, uint8_t *ctrl, uint8_t *cmd );
    void edgeCapture( void );
};

#endif // RC6_h
