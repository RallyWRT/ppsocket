//#include "stdafx.h"
#include "tudt.h"
#include "../src/debug.h"
#include ".\tconn.h"
#include <process.h>    /* _beginthread, _endthread */

using namespace UDT;

//=========================================================================================


TConnection::TConnection(TConnectMng *pMng,UDTSOCKET u)
{
	m_threadTryConnect = NULL;
	m_pMng = pMng;
	m_uSocket = u;
	if (UDT::INVALID_SOCK==m_uSocket)
	{
		return;
	}
	m_pMng->AddConn(this);
}
TConnection::~TConnection()
{
	if (m_threadTryConnect!=NULL)
	{
		CloseHandle((HANDLE)m_threadTryConnect);
	}
	if (UDT::INVALID_SOCK==m_uSocket)
	{
		return;
	}
	Close();
}

void TConnection::SetSocket(UDTSOCKET u)
{
	cout<<"TConnection::SetSocket "<<GetSocket()<<"-->"<<u<<endl;

	if (UDT::INVALID_SOCK!=m_uSocket)
	{
		m_pMng->RemoveConn(this);
	}

	m_uSocket = u;

	if (UDT::INVALID_SOCK!=m_uSocket)
	{
		m_pMng->AddConn(this);
	}
}
bool TConnection::IsReady()
{
	if(m_uSocket==UDT::INVALID_SOCK)
	{
		return false;
	}
	sockaddr sa;
	int len = 0;
	return 0==UDT::getpeername(m_uSocket,&sa,&len); //这个函数执行是否成功可以说明socket是否已经连接
}

#ifndef WIN32
void* UDT::ThreadTryConnect(void* s)
#else
unsigned int WINAPI UDT::ThreadTryConnect(LPVOID s)
#endif
{
	TConnection *pConn = (TConnection *)s;
	pConn->ThreadConnect();
#ifndef WIN32
	return NULL;
#else
	return 0;
#endif
}

bool TConnection::ConnectToEx(const sockaddr* name, int namelen)
{
	if (UDT::INVALID_SOCK!=m_uSocket)
	{
		return false;
	}
	ASSERT(namelen==sizeof(sockaddr));
	m_saDist = *name;
	m_threadTryConnect = _beginthreadex(NULL, 0, ThreadTryConnect, this, 0, NULL);
	return true;
}

bool  TConnection::ThreadConnect()
{
	if (UDT::INVALID_SOCK!=m_uSocket)
	{
		ASSERT(FALSE); //已经处于连接状态？
		return true;
	}
	UDTSOCKET u = m_pMng->CreateSocket(true);		//不用m_uSocket，避免被认为连接已建立
	if(0!=UDT::connect(u,&m_saDist,sizeof(m_saDist)))
	{
		if(IsReady())
		{
			return true;
		}
		OnConnected(false);
		std::cout<<"TConnection错误:"
			<<UDT::getlasterror().getErrorCode()
			<<","<<UDT::getlasterror().getErrorMessage()<<endl;
		return true;
	}
	if(IsReady())  //检查本地是否已经建立连接（可能是其它线程建立的），如果是，关掉这个刚完成的
	{
		UDT::close(u);
		return true;
	}

	std::cout<<"ThreadConnect:"<<u<<std::endl;
	SetSocket(u);
	return true;
};

int TConnection::GetPerformance(TRACEINFO &ti,bool bClear)
{
	return UDT::perfmon(m_uSocket,&ti,bClear);
}
int TConnection::Send(const char* pData,int iLen)
{
	return UDT::sendmsg(m_uSocket,pData,iLen);
};

UDTSOCKET TConnection::GetSocket()
{
	return m_uSocket;
}
//TConnectMng *TConnection::GetMng()
//{
//	return m_pMng;
//}

bool TConnection::Close()
{
	m_pMng->RemoveConn(this);
	m_pMng->GetBase()->CloseConn(m_uSocket);
	m_uSocket = INVALID_SOCK;
	return true;
}

void  TConnection::OnConnected(bool bSuccess){
	if (bSuccess)
	{
		cout<<"有新连接上线！"<<endl;
	}
	else
	{
		cout<<"连接失败"<<endl;
	}

	//异步模式下，连接后必须立即发送一个数据过去？
	//this->Send("TEST",5);
};
void  TConnection::OnDisConnected(){
	cout<<"连接断开！"<<endl;
	Close();//删除自身的时候，会自动从ConnMng中注销
}; 
void  TConnection::OnData(const char* pData, int ilen){
	ilen;
	cout<<"收到数据："<<pData<<endl;
};
//======================================================================================

ConnMng::ConnMng(UDT::TConnectMng *pParentMng,int iPort)
{
	m_pBase = new UDT::P2PNetworkBase(iPort);
	m_pBase->SetNetworkListener(this);

	m_pParentMng = pParentMng;
}

ConnMng::~ConnMng()
{
	if (m_pBase)
	{
		delete m_pBase;
	}
}
//
//Connection * ConnMng::ConnFactory( UDTSOCKET u )
//{
//	return new Connection(this,u);
//}

void ConnMng::AddConn( TConnection *pConn )
{
	ASSERT(pConn);
	{
		CGuard tmpGuard(m_Sock2ConnMutex.GetMutex());
		m_mapSock2Conn[pConn->GetSocket()] = pConn;
	}
	m_pBase->AddConn(pConn->GetSocket());

	OnConnectionMsg(pConn->GetSocket(),NL_CODE_NEWCONN,0);
}

void ConnMng::RemoveConn( TConnection *pConn )
{
	ASSERT(pConn);
	//ASSERT(UDT::INVALID_SOCK!=pConn->GetSocket());

	CGuard tmpGuard(m_Sock2ConnMutex.GetMutex());
	m_mapSock2Conn.erase(pConn->GetSocket());
}

void ConnMng::OnConnectionMsg( int iConn,enNetworkCode code,int param )
{
	param;
	TConnection *pConn = NULL;
	{

		CGuard tmpGuard(m_Sock2ConnMutex.GetMutex());
		map<UDTSOCKET,UDT::TConnection *>::iterator it = m_mapSock2Conn.find(iConn);
		if (it!=m_mapSock2Conn.end())
		{
			pConn = (*it).second;
		}
	}

	if (NL_CODE_NEWCONN==code)
	{
		if (NULL==pConn)
		{
			TConnection *pNewConn = m_pParentMng->ConnFactory(iConn);
			if (!pNewConn) //NULL代表拒绝连接
			{
				ASSERT(FALSE);
				UDT::close(iConn);
				return;
			}
			//tianzuo,2009-6-2,被动连接的，是不是也应该有OnConnected消息？
			//pNewConn->OnConnected(true);
			return;
		}
		else
		{
			pConn->OnConnected(true);
			return;
		}
	}
	else if (NL_CODE_BREAKDOWN==code)
	{
		if (!pConn)
		{
			return;
		}
		pConn->OnDisConnected();
	}
}

void ConnMng::OnData( int iConn,int iLen, const char *pData )
{
	TConnection *pConn = NULL;
	{

		CGuard tmpGuard(m_Sock2ConnMutex.GetMutex());
		pConn = m_mapSock2Conn[iConn];
	}
	if (NULL==pConn)
	{
		return;
	}
	pConn->OnData(pData,iLen);
}

UDT::P2PNetworkBase * ConnMng::GetBase()
{
	return m_pBase;
}

//=========================================================================================

TConnectMng::TConnectMng(int iPort , void* pParam)
:m_pParam(pParam)
{
	m_pConnMng = new ConnMng(this,iPort);
}
TConnectMng::~TConnectMng()
{
	delete m_pConnMng;
}

void TConnectMng::AddConn(TConnection *pConn)
{
	m_pConnMng->AddConn(pConn);
}
void TConnectMng::RemoveConn(TConnection *pConn)
{
	m_pConnMng->RemoveConn(pConn);
}

void TConnectMng::OnConnectionMsg(int iConn,enNetworkCode code,int param)
{
	m_pConnMng->OnConnectionMsg(iConn,code,param);
}
void TConnectMng::OnData(int iConn,int iLen, const char *pData)
{
	m_pConnMng->OnData(iConn,iLen,pData);
}
UDT::P2PNetworkBase *TConnectMng::GetBase()
{
	return m_pConnMng->GetBase();
}

UDTSOCKET TConnectMng::CreateSocket(bool bRendezvous)
{
	return m_pConnMng->GetBase()->CreateSocket(bRendezvous);
}
TConnection *TConnectMng::ConnFactory(UDTSOCKET u)
{
	//return m_pConnMng->ConnFactory(u);
	return new TConnection(this,u);
}
