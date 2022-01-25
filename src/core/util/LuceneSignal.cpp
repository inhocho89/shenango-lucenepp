/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "LuceneSignal.h"
#include "Synchronize.h"

namespace Lucene {

LuceneSignal::LuceneSignal(const SynchronizePtr& objectLock) {
    this->objectLock = objectLock;
}

LuceneSignal::~LuceneSignal() {
}

void LuceneSignal::createSignal(LuceneSignalPtr& signal, const SynchronizePtr& objectLock) {
    static rpc::Mutex lockMutex;
    //rt::ScopedLock<rpc::Mutex> syncLock(&lockMutex);
    lockMutex.Lock();
    if (!signal) {
        signal = newInstance<LuceneSignal>(objectLock);
    }
    lockMutex.Unlock();
}

void LuceneSignal::wait() {
    int32_t relockCount = objectLock ? objectLock->unlockAll() : 0;
    //rt::ScopedLock<rt::Mutex> waitLock(&waitMutex);
    waitMutex.Lock();
    signalCondition.Wait(&waitMutex);

    for (int32_t relock = 0; relock < relockCount; ++relock) {
        objectLock->lock();
    }
    waitMutex.Unlock();
}

void LuceneSignal::wait(int32_t timeout) {
    int32_t relockCount = objectLock ? objectLock->unlockAll() : 0;
    //rt::ScopedLock<rt::Mutex> waitLock(&waitMutex);
    waitMutex.Lock();
    signalCondition.TimedWait(&waitMutex, (uint64_t)timeout * 1000);

    for (int32_t relock = 0; relock < relockCount; ++relock) {
        objectLock->lock();
    }
    waitMutex.Unlock();
}

void LuceneSignal::notifyAll() {
    signalCondition.SignalAll();
}

}
