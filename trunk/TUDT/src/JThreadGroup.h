#pragma once
#include <set>
#define WIN32_LEAN_AND_MEAN
#include "jthread.h"
#include "jmutex.h"

//���JThread��������
//��������ʱ����delete�����е�JThread
//�����������̰߳�ȫ��
class JThreadGroup
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
};
