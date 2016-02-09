// #include "SedProtRTS.h"
#ifndef  SED_PROT_RTS_H
#define SED_PROT_RTS_H

#define RTS_PIN         PD2
#define RTS_DDR         DDRD
#define RTS_PORT        PORTD

#define RTS_INIT        \
    do { \
        RTS_DDR |= _BV( RTS_PIN ); \
        RTS_PORT &= ~( _BV( RTS_PIN ) ); \
    } while( 0 );

#define RTS_HIGH        \
    do { \
RTS_PORT |= _BV( RTS_PIN ); \
    } while( 0 );

#define RTS_LOW         \
    do { \
        RTS_PORT &= ~( _BV( RTS_PIN ) ); \
    } while( 0 );

#endif
