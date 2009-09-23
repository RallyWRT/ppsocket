#ifndef  QUEUE_H
#define  QUEUE_H
#include "jmutex.h"
#include "jmutexautolock.h"
template <class T> class Queue 
{
protected:
	T*                   m_pHead;     //队列的头指针
	T*                   m_pTail;     //队列的尾指针
	long                 m_nSize;
	JMutex				 m_lockerRes;     //队列的资源锁
	bool				 m_bIsExit;//是否退出
public:
	static Queue<T>& instance(){static Queue<T> obj; return obj;}
	void ReOpen()
	{
		m_bIsExit = false;
	}
	void exit()
	{
		if(m_bIsExit)
			return;
		JMutexAutoLock lock(m_lockerRes);
		m_bIsExit = true;
		//ReleaseSemaphore(m_hEvent, 1, NULL);
	}
	void push(T* pNode)
	{
		JMutexAutoLock lock(m_lockerRes);
		pNode->pNext=NULL;
		if(m_pHead==NULL)
		{
			m_pHead=pNode;
			m_pTail=pNode;
		}
		else
		{
			m_pTail->pNext=pNode;
			m_pTail=pNode;
		}
		m_nSize++;
		SetEvent(m_hEvent);

		//ReleaseSemaphore(m_hEvent, 1, NULL);
	}
	T*  GetQueue()
	{
		if( false == m_bIsExit)
		{
			if( m_nSize == 0 )
			{
				while ( WAIT_TIMEOUT == WaitForSingleObject(m_hEvent, 100 ) )
				{
					if( m_bIsExit )
						return NULL;

					if( m_nSize != 0 )
						break;
				}
			}
		}
		else
			return NULL;

		JMutexAutoLock lock(m_lockerRes);

		ResetEvent( m_hEvent );

		if( m_bIsExit )
			return NULL;

		T* pNode = m_pHead;

		if(m_pHead)
		{
			m_pHead=m_pHead->pNext;
			m_nSize--;
		}

		return pNode;
		
	}
	void clear()
	{
	     m_nSize = 0;
	     m_pHead = NULL;
	}
	size_t size()
	{
		JMutexAutoLock lock(m_lockerRes);
		return m_nSize;
	}
	bool empty()
	{
		JMutexAutoLock lock(m_lockerRes);
		return m_pHead==NULL ? true : false;
	}
	T*  Remove()
	{
		JMutexAutoLock lock(m_lockerRes);
		T* pNode = m_pHead;
		if(m_pHead)
		{
			m_pHead=m_pHead->pNext;
			m_nSize--;
		}
		return pNode;
	}
protected:
	Queue() : m_pHead(NULL), m_nSize(0)
	{
		m_hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
		m_bIsExit=false;
		
		m_lockerRes.Init();//初始化互斥量
	}
	~Queue(){ exit();clear();CloseHandle(m_hEvent);} 
private:
	HANDLE  m_hEvent;
};
#endif