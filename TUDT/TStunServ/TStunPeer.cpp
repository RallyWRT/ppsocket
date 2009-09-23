#include "StdAfx.h"
#include ".\tstunpeer.h"
#include "debug.h"

int TStunPeer::m_iMsgSequence = 0;

TStunPeer::TStunPeer(UDT::TPeerMng *pMng,T_PEERID iPeerId,UDTSOCKET u)
:UDT::TPeer(pMng,iPeerId)
{
	m_iMsgSequence = 0;
}

TStunPeer::~TStunPeer(void)
{
}

void TStunPeer::OnPeerConnected()
{
	cout<<"Peer上线：id="<< GetPeerId() <<endl;
	if (GetPeerId()==GetPeerMng()->GetId())
	{
		cout<<"错误：对方ID与自己ID相同！"<<endl;
		Close();
		return;
	}
	cout<<"现有连接用户列表："<<endl;

	std::set<T_PEERID> setIds;
	GetPeerMng()->GetPeerSet(setIds);
	std::set<T_PEERID>::iterator it = setIds.begin();
	for (;it!=setIds.end();it++)
	{
		T_PEERID id = *it;
		typedef boost::shared_ptr<TStunPeer> MyPeerPtr;
		//MyPeerPtr pPeer = GetPeerMng()->FindPeer(id);
		TStunPeer *pPeer = (TStunPeer *)GetPeerMng()->FindPeer(id).get();
		ASSERT(pPeer);
		cout<<id<<","<<pPeer->m_iMsgSequence<<"\t";
	}
	cout<<endl;
}

void TStunPeer::OnPeerDisConnected()
{
	cout<<"Peer断开连接：id="<< GetPeerId() <<endl;
	//TPeer::OnPeerDisConnected();
	//delete this;
	this->Close();
}

void TStunPeer::OnPeerData( const char* pData, int ilen )
{
	//cout<<GetPeerId()<<":("<<ilen<<"):"<< pData <<endl;

	if(0!=memcmp(pData,"ECHO ",5)) //对于ECHO开头的数据包，直接回复之
	{
		return;
	}
	char buf[20];
	string strReply = string(itoa(m_iMsgSequence++,buf,10)) + pData;
	SendPeerMsg(strReply.data(),(int)strReply.size()+1);
}