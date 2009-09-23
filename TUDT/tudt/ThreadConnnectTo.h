#pragma once
#include "jthread.h"
#include "../Include/tudt.h"

class NPeerMng;
class ThreadConnnectTo :
	public JThread
{
public:
	ThreadConnnectTo(NPeerMng *pMng, T_PEERID iDistPeerID,T_PEERID iMyId, 
		UDTSOCKET u,const sockaddr *name,int namelen);
	~ThreadConnnectTo(void);

	void *Thread();
private:
	friend class NPeerMng;
	NPeerMng *m_pMng;
	T_PEERID m_iDistPeerID;
	T_PEERID m_iMyId;
	UDTSOCKET m_socket;
	sockaddr m_name;
	int m_namelen;
};
