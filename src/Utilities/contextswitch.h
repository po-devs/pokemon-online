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

  This was developped specifically for PO so it's not as generic as you would like. And if not used with
  threads it could fail badly, if you want info / examples drop on #po on irc.freenode.net, or better, on
  our forums.

  If you would like a concrete example of coroutines to understand how it works, create a main.cpp with that:

#include <iostream>

extern "C" {
#include "coro.h"
}

using namespace std;

coro_context a, b, init;

void foo(void *)
{
    for (int i = 0; i < 10; i++) {
        cout << i * 2 << endl;
        coro_transfer(&a, &b);
    }

    // Returning to main
    coro_transfer(&a, &init);
}

void foo2(void *)
{
    for (int i = 0; i < 10; i++) {
        cout << (i * 2)+1 << endl;
        coro_transfer(&b, &a);
    }

    //Never reached but safety mesure
    coro_transfer(&b, &init);
}

int main()
{
    coro_create(&init, NULL, NULL, NULL, 0);

    void *stack1 = malloc(256*256);
    void *stack2 = malloc(256*256);
    coro_create(&a, foo, NULL, stack1, 256*256);
    coro_create(&b, foo2, NULL, stack2, 256*256);
    coro_transfer(&init, &a);

    free(stack1);
    free(stack2);

    cout << "Hello world!" << endl;
    return 0;
}

  And figure out why it displays all numbers in order =p

  (Dont forget to include coro.c and coro.h in your project)

*/

class ContextCallee;

class ContextQuitEx {

};

class ContextSwitcher : public QThread
{
    friend class ContextCallee;
public:
    enum Scheduling {
        Start = 0,
        Continue = 1,
        Cease = 2
    };

    typedef QPair<ContextCallee *, Scheduling> pair;
    typedef QPair<ContextSwitcher *, ContextCallee *> startpair;

    ContextSwitcher();
    ~ContextSwitcher();

    void finish();

    /* Starts the main loop. */
    void run();

    /* pause/unpause the whole thing thread. pausing may lock while the current context
     * finishes its task */
    void pause();
    void unpause();

    /* Thread safe. Ends the run() by throwing an exception, that is caught. It will be executed in the ContextSwitcher thread
        so it might not execute directly, but will do as soon as the current ContextCallee yields. */
    void terminate(ContextCallee *c);
private:
    /* Creating contexts is not even reentrant, but with a mutex
       it's fine */
    static QMutex guardian;
    QMutex ownGuardian;
    QSemaphore streamController, pauseController;

    coro_context main_context;
    QSet<ContextCallee *> contexts;
    ContextCallee *current_context;
    ContextCallee *context_to_delete;

    QList<pair> scheduled;
    bool finished;

    void create_context(coro_context *c, coro_func function=NULL, void *param=NULL, void *stack=NULL, long stacksize=0);
    void switch_context(ContextCallee *new_context);
protected:
    /* Adds the callee and runs it */
    void runNewCallee(ContextCallee *callee);

    /* Takes a QPair<ContextSwitcher*, ContextCalle*> * as a parameter. Freeing the arg if dynamicly allocated
       is the responsability of the caller */
    static void runNewCalleeS(void *);

    void schedule(ContextCallee *c);
    void yield();
};

class ContextCallee : public QObject
{
    friend class ContextSwitcher;
public:
    ContextCallee(long stacksize = 500*1024);
    ~ContextCallee();

    void start(ContextSwitcher &ctx);

    virtual void run() = 0;

    /* If called in a different thread, will wait till the context stuff ended. If in the same
       thread, you're badly screwed. */
    void wait();

    /* Asks to be rescheduled asap. Thread safe. */
    void schedule();

    bool finished();

    /* Used to be terminated. Thread safe */
    void terminate();

protected:
    /* Gives the hand back to the Context Switcher */
    void yield();
    /* Terminate the context, but doesn't delete the class. In short it just exits of the run() function and cleanly at that.
        Though in practice you do not need to call it if you don't need it. And only callable from the main context, or you'll have
        big problems */
    void exit();
private:
    ContextSwitcher *ctx;

    long stacksize;
    void *stack;
    bool needsToExit;
    volatile bool _finished;
    coro_context context;
};

#endif // CONTEXTSWITCH_H
