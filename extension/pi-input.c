#include "DEV_Config.h"
#include "GUI_Paint.h"
#include <stdlib.h>



/*void *take_input( void *arg ) {
        bool *running = (bool *)arg;

        //set input flags to be non-blocking
        //need this so that the program can terminate (doesn't hang on getchar)
        int flags = fcntl(STDIN_FILENO, F_GETFL, O_NONBLOCK);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

        while(*running) {
                int c = toupper(getchar());

                switch(c) {
                        case 'S':
                                print_hearts();
                                break;
                        case 'F':
                                feed();
                                break;
                        case 'C':
                                clean();
                                break;
                        case 'P':
                                play();
                                break;
                        case 'G':
                                gift();
                                break;
                }       
        }

        //restore the original flags
        fcntl(STDIN_FILENO, F_SETFL, flags);

        return NULL;
}*/
