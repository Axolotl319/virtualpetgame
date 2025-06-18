#define COIN_TIME 4
#define COIN_AMT  5

#ifndef COINS_GLOBAL
#define COINS_GLOBAL
extern int coins;
#endif

extern void * increment_coins( void * arg );
extern int decrement_coins( int amount );
