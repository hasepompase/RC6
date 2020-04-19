#include <RC6.h>  

/* 
 *  RX_PIN must be a digital pin usable for interrupts:
 *  ATmega328-based:       2, 3
 *  ATmega32u4-based:      0, 1, 2, 3, 7
 *  ATMega1280/2560-based: 2, 3, 18, 19, 20, 21
 * 
 *  Check https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
 *     
 *     SFH506                SFH5110
 *     TSOP1736              TSOP1836 
 *     __________             _________ 
 *    |   ____   |           |   ___   |
 *    |  /    \  |           |  /   \  |
 *    |  |    |  |           |  \___/  |
 *    |  |    |  |           |_________|
 *    |  |    |  |            _|  |  |_
 *    |__|____|__|           |    |    |
 *     |   |    |            |    |    |
 *     |   |    |            |    |    |
 *     |   |    |            |    |    |
 *     |   |    |            |    |    |
 *     |   |    |            |    |    |
 *    GND V +  Out          Out  GND  V +
 */
 
#define RX_PIN 2


RC6 rc( RX_PIN );


void setup() {
    Serial.begin( 115200 );
}


void loop() {
    uint8_t toggle, mode, group, cmd;
    if( rc.read( &toggle, &mode, &group, &cmd ) ) {
        Serial.print( toggle );
        Serial.print( " " );
        Serial.print( mode, HEX );
        Serial.print( " " );
        Serial.print( group, HEX );
        Serial.print( " " );
        Serial.println( cmd, HEX );
    }
}
