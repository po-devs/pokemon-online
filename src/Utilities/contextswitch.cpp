#include "contextswitch.h"

QMutex ContextSwitcher::guardian;

ContextSwitcher::ContextSwitcher() : current_context(NULL)
{
    create_context(&main_context);
}

ContextSwitcher::~ContextSwitcher()
{
    foreach (ContextCallee *x, contexts) {
        delete x;
    }
    contexts.clear();
}

void ContextSwitcher::schedule(ContextCallee *c)
{

}

void ContextSwitcher::yield()
{
    if (!current_context)
        /* asdf! Crash the fool who called that! But no, just return :( */
        return;

    ContextCallee *tmp = current_context;
    current_context = NULL;
    coro_transfer(&tmp->context, &main_context);
}


void ContextSwitcher::create_context(coro_context *c, coro_func function, void *param, void *stack, long stacksize)
{
    guardian.lock();
    coro_create(c, function, param, stack, stacksize);
    guardian.unlock();
}

ContextCallee::ContextCallee(long stacksize)
{
    stack = malloc(stacksize);
}

ContextCallee::~ContextCallee()
{
    free(stack);
}
