#pragma once
#include "tudt.h"

class TClientPeer
	:public UDT::TPeer
{
public:
	TClientPeer(UDT::TPeerMng *pMng,T_PEERID iPeerId=0,UDTSOCKET u=UDT::INVALID_SOCK);
	~TClientPeer(void);

	//»Øµ÷º¯Êý
	virtual void  OnPeerConnected(bool bSuccess);           
	virtual void  OnPeerDisConnected();
	virtual void  OnPeerData(const char* pData, int ilen);
};

extern TClientPeer *g_pCurrPeer;
