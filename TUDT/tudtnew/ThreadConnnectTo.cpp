#include ".\threadconnnectto.h"
#include "debug.h"
#include "NPeerMng.h"

ThreadConnnectTo::ThreadConnnectTo(NPeerMng *pMng,T_PEERID iDistPeerID,T_PEERID iMyId, 
								   UDTSOCKET u,const sockaddr *name,int namelen)
{
	m_pMng = pMng;
	ASSERT(m_pMng);

	m_iDistPeerID = iDistPeerID;
	m_iMyId = iMyId;
	m_socket = u;
	m_name = *name;
	m_namelen = namelen;
}

ThreadConnnectTo::~ThreadConnnectTo(void)
{
}

void * ThreadConnnectTo::Thread()
{
	ThreadStarted();

	int iRes = UDT::connect(m_socket,&m_name,m_namelen);
	m_pMng->OnConnectExFinish(this,iRes);
	return NULL;
}