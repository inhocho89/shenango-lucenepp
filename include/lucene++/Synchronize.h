/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SYNCHRONIZE_H
#define SYNCHRONIZE_H

#include "cc/sync.h"
#include "breakwater/sync++.h"
#include "Lucene.h"

namespace Lucene {

/// Utility class to support locking via a mutex.
class LPPAPI Synchronize {
public:
    Synchronize();
    virtual ~Synchronize();

protected:
    rpc::Mutex mutexSynchronize;
    int64_t lockThread;
    int32_t recursionCount;

public:
    /// create a new Synchronize instance atomically.
    static void createSync(SynchronizePtr& sync);

    /// Lock mutex
    void lock();

    /// Unlock mutex.
    void unlock();

    /// Unlock all recursive mutex.
    int32_t unlockAll();

    /// Returns true if mutex is currently locked by current thread.
    bool holdsLock();

    /// get Mutex queueing delay
    uint64_t QueueUS();

    bool isCongested();
};

template <class OBJECT>
void checkQueueUS(OBJECT object, const char *str) {
    uint64_t qus = object->getSync()->QueueUS();
    if (qus > 5000) {
        printf("[%s] congested: %lu us\n", str, qus);
    }
}

template <class OBJECT>
bool SyncIsCongested(OBJECT object) {
    return object->getSync()->isCongested();
}

/// Utility class to support scope locking.
class LPPAPI SyncLock {
public:
    SyncLock(const SynchronizePtr& sync);

    template <class OBJECT>
    SyncLock(OBJECT object) {
        this->sync = object->getSync();
        lock();
    }

    virtual ~SyncLock();

protected:
    SynchronizePtr sync;

protected:
    void lock();
};

}

#endif
