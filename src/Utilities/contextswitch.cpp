#include "contextswitch.h"

QMutex ContextSwitcher::guardian;

ContextSwitcher::ContextSwitcher() : current_context(NULL), context_to_delete(NULL), finished(false)
{
    connect(this, SIGNAL(finished()), SLOT(deleteLater()));

    pauseController.release(1000);
}

ContextSwitcher::~ContextSwitcher()
{
    qDebug() << "Deleting a context switcher ";
    /* Normally, all contexts should have disappeared before though */
    finish();

    //to suppress "no effect" warning
    (void) coro_destroy(&main_context);

    qDebug() << "End Deleting a context switcher ";
}

void ContextSwitcher::finish()
{
    foreach(ContextCallee *context, contexts) {
        terminate(context);
    }
    contexts.clear();
    finished = true;
    streamController.release(1);
}

void ContextSwitcher::run()
{
#ifdef CORO2
    coro_create(&main_context);
    coro_main(&main_context);
#else
    create_context(&main_context);
#endif
    pauseController.acquire(1000);
    forever {
        if (context_to_delete) {
            /* That literally finishes the deleting operation by allowing wait()
               to finish */
            context_to_delete->_finished = true;
            context_to_delete = NULL;
        }

        /* If an exterior program wants to lock the thread,
         * they will acquire() the pause controller and lock the loop
         * right here */
        pauseController.release(1000);
        /* Pauses the thread until a new task is scheduled */
        streamController.acquire(1);
        pauseController.acquire(1000);

        if (finished) {
            goto end;
        }

        ownGuardian.lock();

        if (scheduled.size() == 0) {
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
            startpair sp(this, p.first);
#ifdef CORO2
            coro_create(&p.first->context);
            current_context = p.first;
            coro_start(&main_context, &(p.first->context), &ContextSwitcher::runNewCalleeS, &sp);
#else
            create_context(&p.first->context, &ContextSwitcher::runNewCalleeS, &sp, p.first->stack, p.first->stacksize);
            switch_context(p.first);
#endif
            break;
        }
        case Continue:
            switch_context(p.first);
            break;
        }
    }
    end:;
    pauseController.release(1000);
}

void ContextSwitcher::pause()
{
    pauseController.acquire();
}

void ContextSwitcher::unpause()
{
    pauseController.release();
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

    }
    /* We can't use the stack after we set finished() to true, because then this might get deleted.
       Not using the stack means not using sp.
       So we will do that in the main context instead of here */
    sp.first->contexts.remove(sp.second);
    sp.first->context_to_delete = sp.second;
    /* Gets back to the main context */
    sp.first->yield();
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
    if (!current_context) {
        qCritical() << "Context Switcher: No current context while yielding!";
        /* asdf! Crash the fool who called that! But no, just return :( */
        return;
    }

    ContextCallee *tmp = current_context;
    current_context = NULL;
    coro_transfer(&tmp->context, &main_context);
}

void ContextSwitcher::create_context(coro_context *c, coro_func function, void *param, void *stack, long stacksize)
{
    (void) c;
    (void) function;
    (void) param;
    (void) stack;
    (void) stacksize;
#ifndef CORO2
    guardian.lock();
    coro_create(c, function, param, stack, stacksize);
    guardian.unlock();
#endif
}

ContextCallee::ContextCallee(long stacksize) : ctx(NULL), stacksize(stacksize), needsToExit(false), _finished(false)
{
    stack = malloc(stacksize);
}

ContextCallee::~ContextCallee()
{
    //qDebug() << "Destroying context callee " << this;
    if (!finished()) {
        qCritical() << "Context callee killed without being normally exited, will probably cause a crash or some kind of problem."
                " You need to wait() before calling the destructor, to make sure it's ended.";
    }
    /* Not needed unless you use PThreads, because it causes a warning otherwise it's been warning'd out :/. */
    (void) coro_destroy(&context);

    free(stack);
    //qDebug() << "Destroyed context callee " << this;
}

void ContextCallee::start(ContextSwitcher &ctx)
{
    /* To tell we are not finished */
    this->ctx = &ctx;
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

void ContextCallee::terminate()
{
    /* Fixme: find out why that can happen */
    if (!ctx) {
        qDebug() << "Critical terminate w/o starting";
        abort();
    }

    ctx->terminate(this);
    //ctx = NULL; //removed in case our own context is running in parallel and calls yield();
}

void ContextCallee::wait()
{
    /* Qt does not provide public functions to wait so it might use 100% CPU */
    while (!finished()) {
        ;
    }
}

bool ContextCallee::finished()
{
    return _finished;
}
