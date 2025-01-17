/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENESIGNAL_H
#define LUCENESIGNAL_H

#include "cc/sync.h"
#include "Lucene.h"

namespace Lucene {

/// Utility class to support signaling notifications.
class LPPAPI LuceneSignal {
public:
    LuceneSignal(const SynchronizePtr& objectLock = SynchronizePtr());
    virtual ~LuceneSignal();

protected:
    rt::Mutex waitMutex;
    rt::CondVar signalCondition;
    SynchronizePtr objectLock;

public:
    /// create a new LuceneSignal instance atomically.
    static void createSignal(LuceneSignalPtr& signal, const SynchronizePtr& objectLock);

    /// Wait for signal
    void wait();

    /// wait for signal with timeout
    void wait(int32_t timeout);

    /// Notify all threads waiting for signal.
    void notifyAll();
};

}

#endif
