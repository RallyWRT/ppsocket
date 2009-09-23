#include ".\tclientpeer.h"
#include <iostream>
using namespace std;
using namespace UDT;

TClientPeer *g_pCurrPeer = NULL;

TClientPeer::TClientPeer(TPeerMng *pMng,T_PEERID iPeerId,UDTSOCKET u)
:TPeer(pMng,iPeerId)
{
}

TClientPeer::~TClientPeer(void)
{
}

void TClientPeer::OnPeerConnected(bool bSuccess)
{
	cout<<"OnPeerConnected:"<<GetPeerId()<<endl;

	if (!bSuccess)
	{
		Close();
		return;
	}
	g_pCurrPeer = this;

}

void TClientPeer::OnPeerDisConnected()
{
	cout<<"OnPeerDisConnected:"<<GetPeerId()<<endl;

	Close();
}

void TClientPeer::OnPeerData( const char* pData, int ilen )
{
	cout<<"PEER MSG "<<GetPeerId()<<":"<<pData<<endl;
}