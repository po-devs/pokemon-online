/*
    Wrapper for SDL_Mutex in an SFML fashion.
    (see www.sfml-dev.org)

    coyotte508@hotmail.com
*/

#ifndef INTERMUTEX_HPP_INCLUDED
#define INTERMUTEX_HPP_INCLUDED

#include <SDL/SDL_mutex.h>
#include "utilities.hh"

namespace interface
{

/* yes, that's non-copyable. If you want to make something copyable,
    then wrap it! (wrapper<>) */
class Mutex : public NonCopyable
{
    SDL_mutex *handle;
public:
    Mutex();
    ~Mutex();

    void Lock();
    void Unlock();
};

} //namespace interface

#endif // INTERMUTEX_HPP_INCLUDED
