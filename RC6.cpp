#include "RC6.h"


/*  Global instance for calling the class method from the interrupt service routine
 */

RC6* RC6Instance = NULL;


/**  
 *  @brief Interrupt service routine
 */

void isrChange( void )
{
    RC6Instance->edgeCapture();
}


/**
 *  @brief Constructor 
 *  
 *  @param pin Input pin. Must be a digital pin usable for interrupts
 */

RC6::RC6( uint8_t pin )
:   edgeTime( 0 ),
    rxDone( false ),
    rxPin( pin ),
    TIdx( 0 ),
    TSum( 0 )
{
    RC6Instance = this;
    pinMode( rxPin, INPUT_PULLUP );
    attachInterrupt( digitalPinToInterrupt( rxPin ), isrChange, CHANGE );
}
    

/**
 *  @brief Decodes the received pulses in to a code word
 *  
 *  @param code Code word
 *  @return True on success
 */

bool RC6::decode( uint32_t* code )
{
    rxDone = false;
    TSum   = 0;
    TIdx   = 0;
    
    // Check start sequence 
    if( TArray[ 0 ] != 6 || TArray[ 1 ] != 2 || TArray[ 2 ] != 1 ) {
        attachInterrupt( digitalPinToInterrupt( rxPin ), isrChange, CHANGE );
        return false;
    }

    uint8_t TPos  = 0;
    uint8_t PIdx  = 0;
    uint8_t State = 0x01;
    
    /*  Shorten toggle bit pulses. 
     *  The toggle bit has a (middle) edge at 18T. Reduce the pulse length by 1T
     */
     
    while( TPos < 18 ) {
        TPos += TArray[ PIdx++ ];
        if( TPos == 18 ) {
            TArray[ PIdx - 1 ]--;
            TArray[ PIdx ]--;
        }
    }

    /*  Convert pulse lengths in to bits. Each pulse end marks a level change.
     *  Odd positions are in the middle of a bit.There the previous level is taken as bit.
     *  
     *  Starting after the leader symbol (8T) and stopping in the middle of the last cmd bit (49T)
     */

    *code = 0;
    TPos  = 8;
    PIdx  = 2;

    do {
        TPos += TArray[ PIdx++ ];
        if( TPos % 2 ) {
            *code <<= 1;
            *code  |= State;
        }
        State ^= 0x01;
    } while( TPos < 49 );

    attachInterrupt( digitalPinToInterrupt( rxPin ), isrChange, CHANGE );
    return true;
}


/**
 *  @brief Edge capture. Is called by interrupts
 *  
 */

void RC6::edgeCapture()
{
    /* Carrier frequency 36 kHz
     * 
     * Manchester encoded data, pulse lenghts:
     * - 1T = 16 carrier pulses,  444.4 µs
     * - 2T = 32 carrier pulses,  888.9 µs
     * 
     * Additional pulse lengths in the header results in 2 additional lenghts:
     * - 3T = 48 carrier pulses, 1333.3 µs
     * - 6T = 96 carrier pulses, 2666.7 µs
     * 
     * RC-6 code (mode 0) consists of:
     * - 20T Header
     * - 16T Group byte
     * - 16T Command byte
     * 
     * Header 20T 
     * ######__#_M0M1M2TTTT
     * - Leader symbol 6T high, 2T low
     * - Start bit     1T high, 1T low
     * - 3 Mode bits   6T
     * - Toggle bit    4T double pulse length
     * 
     * The actual pulse lengths deviate from the theoretical values. For the
     * classification of the measured values, larger deviations are assumed. 
     * Testing only with an upper limit.
     * 
     * Pulse lengths are stored in TArray.
     */
    
    const uint32_t now      = micros();
    const uint8_t  plsLen   = ( now - edgeTime ) >> 5; 
    uint8_t        nT;
                   edgeTime = now;
    
    if( plsLen < 20 ) {
        // 1T = 444.4 µs < 20 * 32 + 31 µs = 671 µs
        nT = 1;
    } else if( plsLen < 34 ) {
        // 2T = 888.9 µs < 34 * 32 + 31 µs = 1119 µs
        nT = 2;
    } else if( plsLen < 62 ) {
        // 3T = 1333.3 µs < 62 * 32 + 31 µs = 2015 µs
        nT = 3;
    } else if( plsLen < 90 ) {
        // 6T = 2666.7 µs < 90 * 32 + 31 µs = 2911 µs
        TArray[ 0 ] = 6;
        TSum   = 6;
        TIdx   = 1;
        return;
    } else {
        TSum = 0;
        TIdx = 0;
        return;
    }
  
    TArray[ TIdx++ ] = nT;
    TSum += nT;
    
  
    if( TSum > 50 ) {
        rxDone = true;
        detachInterrupt( digitalPinToInterrupt( rxPin ) );
    }
}


/**
 *  @brief Reads a RC6 code
 *  
 *  @param toggle Toggle bit
 *  @param mode   Mode byte, 3 bits that indicate the mode
 *  @param group  Command group byte
 *  @param cmd    Command byte
 *  @return True on success
 */

bool RC6::read( uint8_t *toggle, uint8_t *mode, uint8_t *group, uint8_t *cmd )
{
    uint32_t code;
    if( read( &code ) ) {
        *cmd    = code & 0xFF;
        code  >>= 8;
        *group  = code & 0xFF;
        code  >>= 8;
        *toggle = code & 0x01;
        *mode   =  ( code >> 1 ) & 0x07;
        return true;
    } else {
        return false;
    }
}


/**
 *  @brief Reads a RC6 code
 *  
 *  @param code RC6 code. Consists of a start bit (always 1), 3 mode bits, 1 toggle bit, a command group byte and a command byte
 *  @return True on success
 */
 
bool RC6::read( uint32_t* code )
{
    if( rxDone ) {
        return decode( code );
    } else {
        return false;
    }
}
