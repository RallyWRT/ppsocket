#pragma once
#include <set>
#include <list>

#define WIN32_LEAN_AND_MEAN
#include "jthread.h"
#include "jmutex.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//���JThread��������
//��������ʱ����delete�����е�JThread
//�����������̰߳�ȫ��
class JThreadGroup : 
	public JThread
{
public:
	JThreadGroup(void);
	~JThreadGroup(void);

	JThread *AddThread(JThread *pThread);

	//ע�⣬RemoveThread��ʱ��Ҳ��ɾ���߳�
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
