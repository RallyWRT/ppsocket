// UploaderTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
using namespace std;

#include "HttpUploader.h"
#include "HttpBlockDownloader.h"
#include "IUploader.h"
#include "tudt.h"
#include "UdtUploader.h"

#include "atldef.h"

#include "RaProtocolStruct.h"
#include "RaProtoValue.h"

class UdtTestPeer
	: public UDT::TPeer
{
public:
	UdtTestPeer(UDT::TPeerMng *pMng,T_PEERID iPeerId)
		: UDT::TPeer(pMng,iPeerId)
	{

	}
	//回调函数
	virtual void  OnPeerConnected(bool bSuccess)
	{

	}
	virtual void  OnPeerDisConnected(){

	}
	virtual void  OnPeerData(const char* pData, int ilen)
	{
		cout<<GetPeerId()<<":("<<ilen<<"):"<< pData <<endl;
	}
private:
};

int test2(int argc, _TCHAR* argv[])
{

	if (argc!=4)
	{
		cout<<"ustage: UdtTest <thread_count> <server_ip> <server_port>"<<endl;
		return 0;
	}
	sockaddr_in addrStunServer;
	addrStunServer.sin_family = AF_INET;
	addrStunServer.sin_addr.s_addr = inet_addr(argv[2]);
	addrStunServer.sin_port = htons(atoi(argv[3]));

	DWORD iIDBase = GetTickCount();
	iIDBase *= 100;
	iIDBase -= iIDBase%100;
	typedef UDT::TPeerMngTemplate<UdtTestPeer> T_TestConn;
	//vector<T_TestConn*> vecConns;
	int iThreadCount = atoi(argv[1]);
	for (int i=0;i<iThreadCount;i++)
	{

		sockaddr_in addrClient;
		addrClient.sin_family = AF_INET;
		addrClient.sin_addr.s_addr = ADDR_ANY;//inet_addr("127.0.0.1");
		addrClient.sin_port = htons(0);

		T_TestConn *pConn = new T_TestConn(iIDBase + i,
			(sockaddr*)&addrClient,
			sizeof(sockaddr_in));

		pConn->ConnectToServer(5000,(sockaddr*)&addrStunServer,sizeof(sockaddr_in));
		int iTimeOut = 0;
		while(!pConn->IsServerConnectReady())
		{
			iTimeOut++;
			if(iTimeOut>10)
			{
				break;
			}
			cout<<".";
			Sleep(100);
		}
		string strData = "ECHO hello,world";
		pConn->SendPeerMsg(5000,strData.c_str(),strData.size()+1);
		Sleep(200);
		delete pConn;
	}
	return 0;
}

int test3(int argc, _TCHAR* argv[])
{

	if (argc!=4)
	{
		cout<<"ustage: UdtTest <thread_count> <server_ip> <server_port>"<<endl;
		return 0;
	}
	{
		sockaddr_in addrStunServer;
		addrStunServer.sin_family = AF_INET;
		addrStunServer.sin_addr.s_addr = inet_addr(argv[2]);
		addrStunServer.sin_port = htons(atoi(argv[3]));

		DWORD iIDBase = GetTickCount();
		iIDBase *= 100;
		iIDBase -= iIDBase%100;
		typedef UDT::TPeerMngTemplate<UdtTestPeer> T_TestConn;
		sockaddr_in addrClient;
		addrClient.sin_family = AF_INET;
		addrClient.sin_addr.s_addr = ADDR_ANY;//inet_addr("127.0.0.1");
		addrClient.sin_port = htons(0);

		T_TestConn *pConn = new T_TestConn(iIDBase,// + i,
			(sockaddr*)&addrClient,
			sizeof(sockaddr_in));
		pConn->ConnectToServer(5000,(sockaddr*)&addrStunServer,sizeof(sockaddr_in));
		int iTimeOut = 0;
		while(!pConn->IsServerConnectReady())
		{
			iTimeOut++;
			if(iTimeOut>10)
			{
				break;
			}
			cout<<".";
			Sleep(100);
		}

		//vector<T_TestConn*> vecConns;
		string strData = "ECHO hello,world";
		int iThreadCount = atoi(argv[1]);
		for (int i=0;i<iThreadCount;)
		{
			if(pConn->SendPeerMsg(5000,strData.c_str(),strData.size()+1)<=0)
			{
				cout<<"Send blocked. Waiting..."<<endl;
				Sleep(200);
			}
			else
			{
				Sleep(10);
				i++;//Sleep(10);
			}
		}
		Sleep(3000);
		delete pConn;
	}
	
	_CrtDumpMemoryLeaks();
	return 0;
}
int _tmain(int argc, _TCHAR* argv[])
{
	return test3(argc,argv);
	if (argc!=4)
	{
		cout<<"ustage: UdtTest <thread_count> <server_ip> <server_port>"<<endl;
		return 0;
	}
	sockaddr_in addrStunServer;
	addrStunServer.sin_family = AF_INET;
	addrStunServer.sin_addr.s_addr = inet_addr(argv[2]);
	addrStunServer.sin_port = htons(atoi(argv[3]));

	DWORD iIDBase = GetTickCount();
	iIDBase *= 100;
	iIDBase -= iIDBase%100;
	typedef UDT::TPeerMngTemplate<UdtTestPeer> T_TestConn;
	vector<T_TestConn*> vecConns;
	int iThreadCount = atoi(argv[1]);
	for (int i=0;i<iThreadCount;i++)
	{

		sockaddr_in addrClient;
		addrClient.sin_family = AF_INET;
		addrClient.sin_addr.s_addr = ADDR_ANY;//inet_addr("127.0.0.1");
		addrClient.sin_port = htons(0);

		 T_TestConn *pConn = new T_TestConn(iIDBase + i,
			(sockaddr*)&addrClient,
			sizeof(sockaddr_in));
		 vecConns.push_back(pConn);

		 pConn->ConnectToServer(5000,(sockaddr*)&addrStunServer,sizeof(sockaddr_in));
		 Sleep(100);
	}

	cout<<"press anykey to send test data..."<<endl;
	//getchar();

	char pSenderBuf[1024];
	for (int i=0;i<20;i++)
	{ 
		vector<T_TestConn*>::iterator it = vecConns.begin();
		for (int index=0;it!=vecConns.end();++it,++index)
		{
			T_TestConn* pConn = *it;
			sprintf(pSenderBuf,"hello:%d,%d",index,pConn->GetId());
			ATLASSERT(pConn->GetId()>0);
			cout<<pSenderBuf<<endl;
			pConn->SendPeerMsg(5000,pSenderBuf,strlen(pSenderBuf)+1);
			Sleep(100);
		}
	}

	cout<<"press anykey to disconnect..."<<endl;
	//getchar();

	vector<T_TestConn*>::iterator it = vecConns.begin();
	for (;it!=vecConns.end();++it)
	{
		T_TestConn* pConn = *it;
		pConn->CloseServerConnection();
		delete pConn;
	}
	vecConns.clear();

	return 0;
}

