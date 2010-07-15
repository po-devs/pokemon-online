#include "contextswitch.h"

QMutex ContextSwitcher::guardian;

ContextSwitcher::ContextSwitcher() : current_context(NULL), context_to_delete(NULL)
{
    create_context(&main_context);
}

ContextSwitcher::~ContextSwitcher()
{
    /* Normally, all contexts should have disappeared before though */
    foreach (ContextCallee *x, contexts) {
        delete x;
    }
    contexts.clear();

    coro_destroy(main_context);
}

void ContextSwitcher::run()
{
    forever {
        if (context_to_delete) {
            context_to_delete->ctx = NULL;
            context_to_delete = NULL;
        }

        streamController.acquire(1);

        ownGuardian.lock();

        if (scheduled.size() > 0) {
            ownGuardian.unlock();
            continue;
        }

        pair p = scheduled.takeFirst();

        ownGuardian.unlock();

        if (!contexts.contains(p.first) && p.second != Start) {
            continue;
        }

        switch (p.second) {
        case Cease: {
            contexts.remove(p.first);
            p.first->needsToExit = true;

            switch_context(p.first);
            break;
        }
        case Start: {
            contexts.insert(p.first);
            p.first->ctx = this;
            startpair sp(this, p.first);
            create_context(&p.first->context, &ContextSwitcher::runNewCalleeS, &sp, p.first->stack, p.first->stacksize);

            switch_context(p.first);
            break;
        }
        case Continue:
            switch_context(p.first);
            break;
        }
    }
}

void ContextSwitcher::switch_context(ContextCallee *new_context)
{
    current_context = new_context;
    coro_transfer(&main_context, &current_context->context);
}

void ContextSwitcher::runNewCalleeS(void *p)
{
    /* It is important to copy the pair here, because when we will switch back to the main
        context (within run()), the variable p will be deallocated */
    startpair sp = * ((startpair*) p);

    try {
        sp.second->run();
    } catch (ContextQuitEx) {
        /* We can't use the stack after we do sp.second->ctx = NULL.
           Not using the stack means not using sp.
           So we will do that in the main context instead of here */
        sp.first->contexts.remove(sp.second);
        sp.first->context_to_delete = sp.second;
        sp.second->yield();
    }
}


void ContextSwitcher::runNewCallee(ContextCallee *callee)
{
    ownGuardian.lock();
    scheduled.push_back(pair (callee, Start));
    ownGuardian.unlock();

    streamController.release(1);
}

void ContextSwitcher::schedule(ContextCallee *callee)
{
    ownGuardian.lock();
    scheduled.push_back(pair (callee, Continue));
    ownGuardian.unlock();

    streamController.release(1);
}

void ContextSwitcher::terminate(ContextCallee *callee)
{
    ownGuardian.lock();
    scheduled.push_back(pair (callee, Cease));
    ownGuardian.unlock();

    streamController.release(1);
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

ContextCallee::ContextCallee(long stacksize) : ctx(NULL), needsToExit(false)
{
    stack = malloc(stacksize);
}

ContextCallee::~ContextCallee()
{
    if (ctx) {
        qCritical() << "Context callee killed without being normally exited, will probably cause a crash or some kind of problem."
                " You need to wait() before calling the destructor, to make sure it's ended.";
    }
    /* Not needed unless you use PThreads, because it causes a warning otherwise it's been commented out :/. */
#ifdef CORO_PTHREAD
    coro_destroy(&context);
#endif
    free(stack);
}

void ContextCallee::start(ContextSwitcher &ctx)
{
    /* Adds ourselves to the stack */
    ctx.runNewCallee(this);
}

void ContextCallee::exit()
{
    throw ContextQuitEx();
}

void ContextCallee::schedule()
{
    ctx->schedule(this);
}

void ContextCallee::yield()
{
    ctx->yield();
    /* If for example the main thread or w/e requested the exit */
    if (needsToExit) {
        exit();
    }
}

void ContextCallee::wait()
{
    /* Qt does not provide public functions to wait so it might use 100% CPU */
    while (ctx) {
        ;
    }
}
