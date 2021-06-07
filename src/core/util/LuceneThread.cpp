/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "LuceneThread.h"

#include "cc/timer.h"

namespace Lucene {

#if defined(_WIN32) || defined(_WIN64)
const int32_t LuceneThread::MAX_THREAD_PRIORITY = THREAD_PRIORITY_HIGHEST;
const int32_t LuceneThread::NORM_THREAD_PRIORITY = THREAD_PRIORITY_NORMAL;
const int32_t LuceneThread::MIN_THREAD_PRIORITY = THREAD_PRIORITY_LOWEST;
#else
const int32_t LuceneThread::MAX_THREAD_PRIORITY = 2;
const int32_t LuceneThread::NORM_THREAD_PRIORITY = 0;
const int32_t LuceneThread::MIN_THREAD_PRIORITY = -2;
#endif

LuceneThread::LuceneThread() {
    running = false;
    detached = false;
}

LuceneThread::~LuceneThread() {
    if (thread && !detached) {
        thread->Join();
	thread.reset();
    }
}

void LuceneThread::start(bool detach) {
    setRunning(false);
    if (thread && !detached) {
        thread->Join();
	thread.reset();
    }
    thread = newInstance<rt::Thread>(std::bind(&LuceneThread::runThread, this));
    if (detach) {
        thread->Detach();
	detached = true;
    }
    setRunning(true);
}

void LuceneThread::runThread() {
    LuceneThreadPtr threadObject(this->shared_from_this());
    try {
        threadObject->run();
    } catch (...) {
    }
    threadObject->setRunning(false);
    threadObject.reset();
}

void LuceneThread::setRunning(bool running) {
    SyncLock syncLock(this);
    this->running = running;
}

bool LuceneThread::isRunning() {
    SyncLock syncLock(this);
    return running;
}

bool LuceneThread::isAlive() {
    return (thread && isRunning());
}

void LuceneThread::setPriority(int32_t priority) {
#if defined(_WIN32) || defined(_WIN64)
    if (thread) {
        SetThreadPriority(thread->native_handle(), priority);
    }
#endif
}

int32_t LuceneThread::getPriority() {
#if defined(_WIN32) || defined(_WIN64)
    return thread ? GetThreadPriority(thread->native_handle()) : NORM_THREAD_PRIORITY;
#else
    return NORM_THREAD_PRIORITY;
#endif
}

void LuceneThread::join() {
    if (thread) {
        thread->Join();
	thread.reset();
    }
}

int64_t LuceneThread::currentId() {
#if defined(_WIN32) || defined(_WIN64)
    return (int64_t)GetCurrentThreadId();
#else
    return reinterpret_cast<int64_t>(thread_self());
#endif
}

void LuceneThread::threadSleep(int32_t time) {
    // Note: time in milliseconds
    rt::Sleep((uint64_t)(time) * 1000);
}

void LuceneThread::threadYield() {
    rt::Yield();
}

}
