

#include <Windows.h>

#include <nudf\rwlock.hpp>


using namespace nudf::util;

CRwLock::CRwLock() throw()
{
    InitializeSRWLock(&m_srwLock);
}

CRwLock::~CRwLock() throw()
{
}

CRwExclusiveLocker::CRwExclusiveLocker(_In_ CRwLock* lock) throw() : m_pLock(lock)
{
    if(NULL != m_pLock) {
        AcquireSRWLockExclusive(m_pLock->GetLock());
    }
}

CRwExclusiveLocker::~CRwExclusiveLocker() throw()
{
    if(NULL != m_pLock) {
        ReleaseSRWLockExclusive(m_pLock->GetLock());
    }
}

CRwSharedLocker::CRwSharedLocker(_In_ CRwLock* lock) throw() : m_pLock(lock)
{
    if(NULL != m_pLock) {
        AcquireSRWLockShared(m_pLock->GetLock());
    }
}

CRwSharedLocker::~CRwSharedLocker() throw()
{
    if(NULL != m_pLock) {
        ReleaseSRWLockShared(m_pLock->GetLock());
    }
}