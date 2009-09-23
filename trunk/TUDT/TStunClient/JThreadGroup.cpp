#include ".\jthreadgroup.h"
#include "jmutexautolock.h"

JThreadGroup::JThreadGroup(void)
{
}

JThreadGroup::~JThreadGroup(void)
{
	KillAll();
}

JThread *JThreadGroup::AddThread( JThread *pThread )
{
	JMutexAutoLock autolock(m_ThreadGroupLock);

	m_setThread.insert(pThread);
	return pThread;
}

void JThreadGroup::RemoveThread( JThread *pThread ,bool bDelete)
{
	m_setThread.erase(pThread);
	if (bDelete)
	{
		delete pThread;
	}
}

void JThreadGroup::KillAll()
{
	JMutexAutoLock autolock(m_ThreadGroupLock);
	std::set<JThread *>::iterator it = m_setThread.begin();
	for (;it!=m_setThread.end();it++)
	{
		JThread *pThread = *it;
		pThread->Kill();
		delete pThread;
	}

	m_setThread.clear();
}