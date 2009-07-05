/*
    Wrapper for SDL_Thread in an SFML fashion.
    (see www.sfml-dev.org)

    coyotte508@hotmail.com
*/

#ifndef INTERTHREAD_HPP_INCLUDED
#define INTERTHREAD_HPP_INCLUDED

#include <SDL/SDL_thread.h>

namespace interface
{

class Thread
{
private:
    /* This class is made non-copyable because YOU HAVE TO FREE THE
        THREADED RESSOURCES even if the thread is already finished.

        So this class' destructor Wait for its own thread to finish
    */
    Thread (const Thread &other) {;}
    SDL_Thread *handle; /* PRIVATE!! */
public:
    typedef int(*FuncType )(void *);

    /* Does nothing */
    Thread();
    /* Launches the thread with those parameters */
    Thread(FuncType function, void *data);
    /* Waits and free memory occupied by the thread */
    Thread::~Thread();

    /* Launches the thread with those parameters
        -- beware, waits for previous thread to finish! */
    void Launch(FuncType function, void *data);

    /* Wait for it to finish */
    void Wait();

    /* Terminate it (dangerous...) */
    void Terminate();
};

}//namespace interface

#endif // INTERTHREAD_HPP_INCLUDED
