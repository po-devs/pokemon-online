#include "interthread.hpp"

namespace interface
{

/* Does nothing */
Thread::Thread()
    : handle (NULL)
{
    ;
}

/* Launches the thread with those parameters */
Thread::Thread(FuncType function, void *data)
    : handle (SDL_CreateThread(function, data))
{
    ;
}

Thread::~Thread()
{
    /* We HAVE TO wait, even if the thread is finished,
        to free memory */
    if (handle)
        SDL_WaitThread(handle, NULL);
}

/* Launches the thread with those parameters
    -- beware, waits for previous thread to finish! */
void Thread::Launch(FuncType function, void *data)
{
    if (handle)
        SDL_WaitThread(handle, NULL);

    handle = SDL_CreateThread(function, data);
}

/* Wait for it to finish */
void Thread::Wait()
{
    if (handle)
        SDL_WaitThread(handle, NULL);

    handle = NULL;
}

/* Terminate it (dangerous...) */
void Thread::Terminate()
{
    if (handle)
        SDL_KillThread(handle);

    handle = NULL;
}

}//namespace interface
