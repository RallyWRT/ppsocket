#pragma once
#include <set>
#include <list>

#define WIN32_LEAN_AND_MEAN
#include "jthread.h"
#include "jmutex.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//多个JThread的容器。
//容器析构时，会delete掉所有的JThread
//容器本身是线程安全的
class JThreadGroup : 
	public JThread
{
public:
	JThreadGroup(void);
	~JThreadGroup(void);

	JThread *AddThread(JThread *pThread);

	//注意，RemoveThread的时候也会删除线程
	void RemoveThread(JThread *pThread,bool bDelete=true);
	void KillAll();
private:
	std::set<JThread *> m_setThread;
	JMutex m_ThreadGroupLock;
	HANDLE m_hEvent;

	std::list<JThread *> m_deleteThread;
	BOOL m_bIsExit;
public:
	void *Thread();
};
