/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ThreadPool.h"

namespace Lucene {

Future::~Future() {
}

ThreadPool::ThreadPool() {
    running = true;
    running_threads = THREADPOOL_SIZE;
    for (int32_t i = 0; i < THREADPOOL_SIZE; ++i) {
        threads[i] = rt::Thread(ThreadPool::run);
    }
}

ThreadPool::~ThreadPool() {
    worksLock.Lock();
    running = false;
    worksCV.SignalAll();
    worksLock.Unlock();
    // wait for all thread
    for (int32_t i = 0; i < THREADPOOL_SIZE; ++i) {
        threads[i].Join();
    }
}

ThreadPoolPtr ThreadPool::getInstance() {
    static ThreadPoolPtr threadPool;
    if (!threadPool) {
        threadPool = newLucene<ThreadPool>();
        CycleCheck::addStatic(threadPool);
    }
    return threadPool;
}

void ThreadPool::_run() {
    while (true) {
        worksLock.Lock();
        while (works.empty() && running) {
            running_threads--;
            worksCV.Wait(&worksLock);
	    running_threads++;
        }

	if (!running) {
            running_threads--;
            worksLock.Unlock();
	    return;
	}

	// retrieve a work
	std::function<void()> f = works.front();
	works.pop();
	worksLock.Unlock();

	// execute a work
	f();
    }
}
}
