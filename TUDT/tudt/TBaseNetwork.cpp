#include <iostream>
#include "TBaseNetwork.h"
#include <process.h>    /* _beginthread, _endthread */
#include <algorithm>
#include "debug.h"
#include "tudt.h"
#include "jmutexautolock.h"
using namespace UDT;
using namespace std;

P2PNetworkBase::P2PNetworkBase(int iUDPPort)
{
	m_pBase = new TBaseNetwork(iUDPPort);
}
P2PNetworkBase::P2PNetworkBase(const sockaddr* name, int namelen)
{
	m_pBase = new TBaseNetwork(name,namelen);
}
P2PNetworkBase::~P2PNetworkBase()
{
	Close();
}
int P2PNetworkBase::GetPort()
{
	return m_pBase->GetPort();
}
//设置网络消息侦听器
void P2PNetworkBase::SetNetworkListener(INetworkListener *pListener)
{
	m_pBase->SetNetworkListener(pListener);
}
//向某个地址尝试连接（连接如果成功，会触发 NL_CODE_NEWCONN 消息）
void P2PNetworkBase::ShootTo(const sockaddr* name, int namelen)
{
	m_pBase->ShootTo(name,namelen);
}
//发送数据。
//这是异步发送。
//发送成功会触发NL_CODE_SENDSUCCEED信息，否则触发NL_CODE_SENDFAILED消息。
//消息的param就是iMsgID。
int P2PNetworkBase::SendTo(int iConn,int iLen, const char *pData)
{
	return m_pBase->SendTo(iConn,iLen,pData);
}
//关闭连接
void P2PNetworkBase::CloseConn(int iConn)
{
	m_pBase->CloseConn(iConn);
}

//将外部创建的socket放入这里管理
void P2PNetworkBase::AddConn(UDTSOCKET u)
{
	m_pBase->AddConn(u);
}

//获取所有连接
void P2PNetworkBase::GetConns(std::set<UDTSOCKET> &setConns)
{
	m_pBase->GetConns(setConns);
}

//获取所有连接
UDTSOCKET P2PNetworkBase::CreateSocket(bool bRendezvous)
{
	return m_pBase->CreateSocket(bRendezvous);
}

//关闭退出。
//此函数会在析构的时候自动调用
void P2PNetworkBase::Close()
{
	if(m_pBase)
	{
		delete m_pBase;
		m_pBase = NULL;
	}
}

//侦听端口，等待连接。
#ifndef WIN32
void* ThreadWaitConnection(void* s)
#else
unsigned int WINAPI ThreadWaitConnection(LPVOID s)
#endif
{
	TBaseNetwork *pBase = (TBaseNetwork *)s;
	pBase->ThreadWaitConn();
#ifndef WIN32
	return NULL;
#else
	return 0;
#endif
}

//侦听端口，等待连接。
#ifndef WIN32
void* ThreadCheckDataStart(void* s)
#else
unsigned int WINAPI ThreadCheckDataStart(LPVOID s)
#endif
{
	TBaseNetwork *pBase = (TBaseNetwork *)s;
	pBase->ThreadCheckData();
	ATLTRACE("TBaseNetwork::ThreadCheckData EXITED.\n");
#ifndef WIN32
	return NULL;
#else
	return 0;
#endif
}

TBaseNetwork::TBaseNetwork(int iUDPPort)
{
	m_addrService.sin_family = AF_INET;
	m_addrService.sin_addr.s_addr = INADDR_ANY;
	m_addrService.sin_port = htons((unsigned short)iUDPPort);

	Init();

}

TBaseNetwork::TBaseNetwork( const sockaddr* name, int namelen )
{
	namelen;
	ASSERT(name);
	if (name)
	{
		m_addrService = *(sockaddr_in *)name;
	}
	Init();
}
TBaseNetwork::~TBaseNetwork(void)
{
	//把m_bExist设为true，等待一段时间，让线程退出。
	m_bExiting = true;
	::WaitForSingleObject((HANDLE)m_threadWaitConnection,INFINITE);
	::WaitForSingleObject((HANDLE)m_threadCheckDataStart,INFINITE);

	CloseHandle((HANDLE)m_threadWaitConnection);
	CloseHandle((HANDLE)m_threadCheckDataStart);

	{
		JMutexAutoLock tmpGuard(m_ConnMutex);
		//std::set<UDTSOCKET>::iterator it = m_setConns.begin();
		//for (;it!=m_setConns.end();it++)
		//{
		//	UDT::close(*it);
		//}
		m_setConns.clear();
	}
	UDT::cleanup();
	//Sleep(100);

	if(m_ServerSocket!=UDT::INVALID_SOCK) 
	{
		//UDT::close(m_ServerSocket);
		m_ServerSocket = INVALID_SOCK;
	}

	//UDT::cleanup();
	//Sleep(100);
}
int TBaseNetwork::GetPort()
{
	return ntohs(m_addrService.sin_port);
}
void TBaseNetwork::SetNetworkListener( UDT::INetworkListener *pListener )
{
	m_pListener = pListener;
}

void TBaseNetwork::ShootTo( const sockaddr* name, int namelen )
{
	UDTSOCKET u = CreateSocket();
	if(0==UDT::connect(u,name,namelen))
	{
		this->AddConnection(u);
	}
}

int TBaseNetwork::SendTo( int iConn,int iLen, const char *pData)
{
	return UDT::sendmsg(iConn,pData,iLen);
}

void TBaseNetwork::CloseConn( int iConn)
{
	{
		JMutexAutoLock tmpGuard(m_ConnMutex);
		m_setConns.erase(iConn);
	}

	UDT::close(iConn);
}

//将外部创建的socket放入这里管理
void TBaseNetwork::AddConn(UDTSOCKET u)
{
	ASSERT(INVALID_SOCK!=u);
	JMutexAutoLock tmpGuard(m_ConnMutex);
	m_setConns.insert(u);
}

//获取所有连接
void TBaseNetwork::GetConns(std::set<UDTSOCKET> &setConns)
{
	//setConns = m_setConns;
	JMutexAutoLock tmpGuard(m_ConnMutex);
	setConns.insert(m_setConns.begin(),m_setConns.end());
}

//===============================
void TBaseNetwork::AddConnection(UDTSOCKET socketNew)
{
	if (m_pListener)
	{
		m_pListener->OnConnectionMsg((int)socketNew,INetworkListener::NL_CODE_NEWCONN,0);
	}

	{
		JMutexAutoLock tmpGuard(m_ConnMutex);

		bool bRes = m_setConns.insert(socketNew).second;
		ASSERT(bRes);
	}

}
void TBaseNetwork::RemoveConnection(UDTSOCKET socketNew)
{
	{
		JMutexAutoLock tmpGuard(m_ConnMutex);

		m_setConns.erase(socketNew);
	}

	if (m_pListener)
	{
		m_pListener->OnConnectionMsg((int)socketNew,INetworkListener::NL_CODE_BREAKDOWN,0);
		return;
	}
}

//在约会模式下的UDT无法正常accept。
void TBaseNetwork::ThreadWaitConn()
{
	UDT::listen(m_ServerSocket,10);
	sockaddr sa;
	int len;
	while (!m_bExiting)
	{
		if (m_ServerSocket==UDT::INVALID_SOCK)
		{
			std::cout<<"等待连接线程退出"<<std::endl;
			return ;
		}
		
		UDTSOCKET u = UDT::accept(m_ServerSocket,&sa,&len);
		if (UDT::INVALID_SOCK==u)
		{
			if (UDT::ERRORINFO::EASYNCRCV==UDT::getlasterror().getErrorCode())
			{
				Sleep(10);
				continue;
			}
			Sleep(10);
			//std::cout<<"waitconnect错误："<<UDT::getlasterror().getErrorMessage();
			continue;
		}

		AddConnection(u);
	}
	ATLTRACE("TBaseNetwork::ThreadWaitConn EXITED.\n");
}
#ifdef UDT_VERSION_4_4
void TBaseNetwork::ThreadCheckData()
{
	const int BUFFER_READ_SIZE = 1024*100;
	char bufRead[BUFFER_READ_SIZE];

	while(!m_bExiting)
	{
		std::set<UDTSOCKET> readfds;
		{
			JMutexAutoLock tmpGuard(m_ConnMutex);
			readfds = this->m_setConns;
		}
		if(readfds.empty())
		{
			Sleep(50);
			continue;
		}
		//std::set<UDTSOCKET> exceptfds;

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		int res = UDT::select(0, &readfds, NULL, NULL, &tv);
		
		if (res == UDT::ERROR)
		{
			//tianzuo,2009-6-1，检查所有handle是否合法，如果不是，发出删除通知
			//std::cout<<"recvmsg错误1:"<<UDT::getlasterror().getErrorMessage()<<endl;
			TRACE("recvmsg错误1:%s\n",UDT::getlasterror().getErrorMessage());

			//tianzuo，2009-6-1，注释掉continue一句以解决某些handle错误导致无法select的问题。
			//后面遍历的时候会删掉损坏的handle
			//Sleep(50);
			//continue;
		}
		std::set<UDTSOCKET>::iterator it = readfds.begin();
		for(;it!=readfds.end();it++)
		{
			if (m_bExiting)
			{
				return;
			}
			int iRec = UDT::recvmsg(*it,bufRead,BUFFER_READ_SIZE);
			if(UDT::ERROR==iRec)
			{
				//int iErrcode = UDT::getlasterror().getErrorCode();

				m_setConns.erase(*it);
				if (m_pListener)
				{
					m_pListener->OnConnectionMsg(*it,INetworkListener::NL_CODE_BREAKDOWN,0);
				}
				continue;
			}
			if (m_pListener)
			{
				m_pListener->OnData(*it,iRec,bufRead);
			}
		}
	}
}
#else
//selectex带句柄集的实现
void TBaseNetwork::ThreadCheckData()
{
	const int BUFFER_READ_SIZE = 1024*100;
	char bufRead[BUFFER_READ_SIZE];
	std::vector<UDTSOCKET> vecReadFds,vecExceptFds;
	while(!m_bExiting)
	{
		{
			JMutexAutoLock tmpGuard(m_ConnMutex);
			UDT::selectEx(m_setConns, &vecReadFds,NULL,&vecExceptFds, 0);
		}
		if (vecReadFds.empty() && vecReadFds.empty())
		{
			Sleep(10);
		}

		std::vector<UDTSOCKET>::const_iterator itRead = vecReadFds.begin();
		for (;itRead!=vecReadFds.end();++itRead)
		{
			if (m_bExiting)
			{
				return;
			}
			UDTSOCKET u = *itRead;

			int iRec = UDT::recvmsg(u,bufRead,BUFFER_READ_SIZE);
			if(UDT::ERROR==iRec)
			{
				HandleSocketBread(u);
				continue;
			}
			if (m_pListener)
			{
				m_pListener->OnData(u,iRec,bufRead);
			}
		}

		std::vector<UDTSOCKET>::const_iterator itExcept = vecExceptFds.begin();
		for (;itExcept!=vecExceptFds.end();++itExcept)
		{
			if (m_bExiting)
			{
				return;
			}
			HandleSocketBread(*itExcept);
		}
	}
}
#endif
//
////使用新的高效接口的读数据函数
//void TBaseNetwork::ThreadCheckData()
//{
//	const int BUFFER_READ_SIZE = 1024*100;
//	char bufRead[BUFFER_READ_SIZE];
//	std::vector<UDTSOCKET> vecFds,vecReadFds,vecExceptFds;
//
//	while(!m_bExiting)
//	{
//		UDT::selectEx(vecFds,&vecReadFds,NULL,&vecExceptFds,1000);
//
//		std::vector<UDTSOCKET>::const_iterator itRead = vecReadFds.begin();
//		for (;itRead!=vecReadFds.end();++itRead)
//		{
//			if (m_bExiting)
//			{
//				return;
//			}
//			UDTSOCKET u = *itRead;
//
//			int iRec = UDT::recvmsg(u,bufRead,BUFFER_READ_SIZE);
//			if(UDT::ERROR==iRec)
//			{
//				HandleSocketBread(u);
//				continue;
//			}
//			if (m_pListener)
//			{
//				m_pListener->OnData(u,iRec,bufRead);
//			}
//		}
//
//		std::vector<UDTSOCKET>::const_iterator itExcept = vecExceptFds.begin();
//		for (;itExcept!=vecExceptFds.end();++itExcept)
//		{
//			if (m_bExiting)
//			{
//				return;
//			}
//			HandleSocketBread(*itExcept);
//		}
//	}
//}

void TBaseNetwork::HandleSocketBread( UDTSOCKET u )
{
	{
		JMutexAutoLock tmpGuard(m_ConnMutex);
		if(1!=m_setConns.erase(u))  //except有可能多次报告
		{
			return;
		}
	}
	if (m_pListener)
	{
		m_pListener->OnConnectionMsg(u,INetworkListener::NL_CODE_BREAKDOWN,0);
	}
}

UDTSOCKET TBaseNetwork::CreateSocket(bool bRendezvous)
{
	bRendezvous;
	UDTSOCKET client = UDT::socket(AF_INET, SOCK_DGRAM, 0);

	//TBaseNetwork必须的特征：约会模式、地址复用、异步收发
	bool bParam = true;
	//UDT::setsockopt(client, 0, UDT_RENDEZVOUS, &bRendezvous, sizeof(bool));
	UDT::setsockopt(client, 0, UDT_REUSEADDR, &bParam, sizeof(bool));

	//server模式，bRendezvous=false。server模式下用同步看看？
	bParam = false;
	UDT::setsockopt(client, 0, UDT_SNDSYN, &bParam, sizeof(bool));
	UDT::setsockopt(client, 0, UDT_RCVSYN, &bParam, sizeof(bool));

	//根据公网环境的实际情况，调整UDT的默认参数：缓冲区大小，窗口大小
	int iParam = 2*1024*1024;
	UDT::setsockopt(client, 0, UDP_SNDBUF, &iParam, sizeof(int));
	UDT::setsockopt(client, 0, UDP_RCVBUF, &iParam, sizeof(int));
	iParam = 2*1024*1024;
	UDT::setsockopt(client, 0, UDT_SNDBUF, &iParam, sizeof(int));
	UDT::setsockopt(client, 0, UDT_RCVBUF, &iParam, sizeof(int));
	iParam = 2048;
	UDT::setsockopt(client, 0, UDT_FC, &iParam, sizeof(int));
	linger l;
	l.l_linger = 1;  //默认的linger时间居然是180秒，太过了
	l.l_onoff = 0;
	UDT::setsockopt(client, 0, UDT_LINGER, (int*)&l, sizeof(int));


	if(UDT::ERROR==UDT::bind(client,(sockaddr *)&m_addrService,sizeof(m_addrService)))
	{
		return UDT::INVALID_SOCK;
	}

	//重新赋值m_addrService，这样能获得实际绑定的端口号
	//设定的端口号是0的情况
	int iLen =0;
	UDT::getsockname(client,(sockaddr *)&m_addrService,&iLen);

	return client;
}

void TBaseNetwork::Init()
{
	UDT::startup();
	m_ServerSocket = CreateSocket(false);
	if(UDT::INVALID_SOCK==m_ServerSocket)
	{
		throw std::exception("TBaseNetwork CreateSocket error");
	}

	m_pListener = NULL;
	m_bExiting = false;
	m_threadWaitConnection = _beginthreadex(NULL, 0, ThreadWaitConnection, this, 0, NULL);
	m_threadCheckDataStart = _beginthreadex(NULL, 0, ThreadCheckDataStart, this, 0, NULL);

	if (m_threadCheckDataStart==0 || m_threadWaitConnection==0)
	{
		throw std::exception("TBaseNetwork CreateThread error");
	}
}
