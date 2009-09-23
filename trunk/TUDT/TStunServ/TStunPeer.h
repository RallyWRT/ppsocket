#pragma once
#include "tudt.h"

class TStunPeer :
	public UDT::TPeer
{
public:
	TStunPeer(UDT::TPeerMng *pMng,T_PEERID iPeerId=0,UDTSOCKET u=UDT::INVALID_SOCK);
	~TStunPeer(void);

	//�ص�����
	virtual void  OnPeerConnected();           
	virtual void  OnPeerDisConnected();
	virtual void  OnPeerData(const char* pData, int ilen);
private:
	static int m_iMsgSequence;
};
