#include ".\jthreadgroup.h"
#include "jmutexautolock.h"

JThreadGroup::JThreadGroup(void)
{
	m_bIsExit = FALSE;
	m_hEvent = CreateEvent( NULL ,TRUE , FALSE , NULL );
	Start();
}


JThreadGroup::~JThreadGroup(void)
{
	m_bIsExit = TRUE;
	SetEvent(m_hEvent);
	KillAll();

	CloseHandle(m_hEvent);
}

JThread *JThreadGroup::AddThread( JThread *pThread )
{
	JMutexAutoLock autolock(m_ThreadGroupLock);

	m_setThread.insert(pThread);
	return pThread;
}

void JThreadGroup::RemoveThread( JThread *pThread ,bool bDelete)
{
	bDelete;
	pThread->SetClosing();
	JMutexAutoLock autolock(m_ThreadGroupLock);
	m_setThread.erase(pThread);	
	m_deleteThread.push_back( pThread );
	SetEvent(m_hEvent);
}

void * JThreadGroup::Thread()
{
	ThreadStarted();
	JThread *pThread;
	while( !m_bIsExit )
	{
		WaitForSingleObject( m_hEvent , INFINITE );
		ResetEvent(m_hEvent);
		JMutexAutoLock autolock(m_ThreadGroupLock);
		std::list<JThread *>::iterator itQ = m_deleteThread.begin();
		for( ; itQ != m_deleteThread.end() ; itQ++ )
		{
			pThread = *itQ;
			pThread->SetClosing();
			//һ��Ҫ���߳���Ȼ�˳�
			while(pThread->IsRunning())
			{
				Sleep(100);
			}
			delete pThread;
		}
		m_deleteThread.clear();
	}
	return 0;
}
void JThreadGroup::KillAll()
{
	Kill();

	//�߳���ʹ��lock�����̱߳�kill����ʱ��lock��û���ͷŵģ�����������
	JMutexAutoLock autolock(m_ThreadGroupLock);

	std::set<JThread *>::iterator it = m_setThread.begin();
	for (;it!=m_setThread.end();it++)
	{
		JThread *pThread = *it;
		pThread->SetClosing();
		//һ��Ҫ���߳���Ȼ�˳�
		while(pThread->IsRunning())
		{
			Sleep(100);
		}
		delete pThread;
	}

	m_setThread.clear();
}