#pragma once
#include "jthread.h"
#include "../Include/udt.h"

class NPeerMng;
class ThreadConnnectTo :
	public JThread
{
public:
	ThreadConnnectTo(NPeerMng *pMng, unsigned int iDistPeerID,unsigned int iMyId, 
		UDTSOCKET u,const sockaddr *name,int namelen);
	~ThreadConnnectTo(void);

	void *Thread();
private:
	friend class NPeerMng;
	NPeerMng *m_pMng;
	unsigned int m_iDistPeerID;
	unsigned int m_iMyId;
	UDTSOCKET m_socket;
	sockaddr m_name;
	int m_namelen;
};
