#ifndef CONTEXTSWITCH_H
#define CONTEXTSWITCH_H

#include <QtCore>
#include "coro.h"

/*
  This file is for co-routines. Check on wikipedia what that is.

  Coroutines are like threads, but without concurrent access.

  A function / object that is supposed to run in a new coroutine (as with threads)
  will be given a reference to this ContextSwitcher below. Let's call that reference
  self. The function will be executed normally, until the function calls self.yield().
  When it does so, the context switcher will swap back to the main context. Then
  whatever manager/scheduler will decide to continue the function, and so the execution
  of the function will continue until it calls yield() again. Basically it's the same as a
  thread which would call sem.acquire(), and stay blocked until the main thread calls sem.release().

  The reason for having this on PO is to prevent having a high number of threads, because the battles
  used to be each in a new thread and then on the Beta Server where over 80 battles would be running
  it would cause serious problems.

  This was developped specifically for PO so it's not as generic as you would like.
*/

class ContextCallee;

class ContextSwitcher
{
    friend class ContextCallee;
public:
    ContextSwitcher();
    ~ContextSwitcher();

    /* Thread safe */
    void terminateCallee(ContextCallee *c);
private:
    /* Creating contexts is not even reentrant, but with a mutex
       it's fine */
    static QMutex guardian;

    coro_context main_context;
    QSet<ContextCallee *> contexts;
    ContextCallee *current_context;

    void create_context(coro_context *c, coro_func function=NULL, void *param=NULL, void *stack=NULL, long stacksize=0);
protected:
    /* Adds the callee and runs it */
    void runNewCallee(ContextCallee *callee);

    void schedule(ContextCallee *c);
    void yield();
    void exit();
};


/* Internal structure used */
struct Context
{
    coro_context context;
    void *stack;
    long stacksize;

    Context(long stacksize = 100*1000);
    ~Context();
};


class ContextCallee
{
    friend class ContextSwitcher;
public:
    ContextCallee(long stacksize = 100*1000);
    ~ContextCallee();

    void start(ContextSwitcher &ctx);

    virtual void run() = 0;

protected:
    /* Gives the hand back to the Context Switcher */
    void yield();
    /* Asks to be rescheduled asap. Thread safe. */
    void schedule();
    /* Terminate the context, but doesn't delete the class. In short it just exits of the run() function and cleanly at that.
        Though in practice you do not need to call it if you don't need it */
    void exit();
private:
    ContextSwitcher *ctx;

    long stacksize;
    void *stack;
    coro_context context;
};

#endif // CONTEXTSWITCH_H
