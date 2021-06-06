/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SYNCHRONIZE_H
#define SYNCHRONIZE_H

#include "cc/sync.h"
#include "Lucene.h"

namespace Lucene {

/// Utility class to support locking via a mutex.
class LPPAPI Synchronize {
public:
    Synchronize();
    virtual ~Synchronize();

protected:
    rt::Mutex mutexSynchronize;
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
};

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
