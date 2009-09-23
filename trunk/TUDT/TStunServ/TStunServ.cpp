// FileTester.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "tudt.h"
#include "debug.h"
#include "StrFuns.h"
#include "TStunPeer.h"
#include "TPeerProtocol.h"

#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//TStunServ的功能如下：
//1.侦听端口，接受所有的连接；
//2.UDT连接建立之后，执行交换ID流程，确认TPEER连接，并且保持这个长连接；
//3.维护映射表：peer_id->可能的地址列表;
//4.接受用户的保活消息，当用户断线的时候删除连接(UDT自带此功能，不用自己做）
//5.接受用户的地址查询请求，返回地址列表
//6.接受用户的打孔请求，向客户端发送打孔命令
int _tmain(int argc, _TCHAR* argv[])
{
	{
		//new char[1000]; //检测内存泄漏
		int iPort = 0;
		int iMyID = 5000;
		if(argc==3)
		{
			iPort = atoi(argv[1]);
			iMyID = atoi(argv[2]);
		}
		else
		{
			cout<<"输入您要使用的UDT端口号："<<endl;
			cin>>iPort;
			cout<<"输入您要使用的PeerID："<<endl;
			cin>>iMyID;
		}

		SOCKADDR_IN sa;
		KIpv4Addr((unsigned int)0,iPort).GetAddr(sa);
		UDT::TPeerMngTemplate<TStunPeer> mng(iMyID,(sockaddr*)&sa,sizeof(sockaddr));
		cout<<"Stun Server port="<<iPort<<" id="<<mng.GetId()<<endl;

		string strCmd;
		char buf[128];

		TStunPeer *g_pCurrPeer = NULL;
		while(strCmd!="q")
		{
			cin.getline(buf,128);

			vector<string> vecParams;
			StrFuns::Tokenize(buf,vecParams," ");
			if(vecParams.empty())
			{
				continue;
			}
			if(vecParams[0]=="q")
			{
				break;
			}
			else
			{
				cout<<"Unknow cmd:"<<vecParams[0]<<endl;
				cout<<"cmd list:"<<endl;
				cout<<"\tq"<<endl;
			}

		}
		cout<<"\texited."<<endl;

		mng.Close();
	}
	_CrtDumpMemoryLeaks();
	return 0;
}

