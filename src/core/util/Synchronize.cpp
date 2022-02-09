/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "Synchronize.h"
#include "LuceneThread.h"

namespace Lucene {

Synchronize::Synchronize() {
    lockThread = 0;
    recursionCount = 0;
}

Synchronize::~Synchronize() {
}

void Synchronize::createSync(SynchronizePtr& sync) {
    static rt::Spin lockMutex;
    rt::ScopedLock<rt::Spin> syncLock(&lockMutex);

    if (!sync) {
        sync = newInstance<Synchronize>();
    }
}

void Synchronize::lock() {
    if (holdsLock()) {
        ++recursionCount;
	return;
    }
    mutexSynchronize.Lock();
    lockThread = LuceneThread::currentId();
    ++recursionCount;
}

void Synchronize::unlock() {
    if (--recursionCount > 0) {
        return;
    }

    lockThread = 0;
    mutexSynchronize.Unlock();
}

int32_t Synchronize::unlockAll() {
    int32_t count = recursionCount;

    recursionCount = 0;
    lockThread = 0;
    mutexSynchronize.Unlock();

    return count;
}

bool Synchronize::holdsLock() {
    return (lockThread == LuceneThread::currentId() && recursionCount > 0);
}

SyncLock::SyncLock(const SynchronizePtr& sync) {
    this->sync = sync;
    lock();
}

SyncLock::~SyncLock() {
    if (sync) {
        sync->unlock();
    }
}

void SyncLock::lock() {
    if (sync) {
        sync->lock();
    }
}

}
