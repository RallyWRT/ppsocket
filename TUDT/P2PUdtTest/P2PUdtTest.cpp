// P2PUdtTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <vector>
using namespace std;

#include "udt.h"
//#include "TNetworkListener.h"
#include "StrFuns.h"

//#include "TConn.h"

class ConnHandler:
	public UDT::TConnection
{
public:
	ConnHandler(UDT::TConnectMng *pMng,UDTSOCKET u=UDT::INVALID_SOCK)
		:TConnection(pMng,u)
	{};
	virtual void  OnConnected(bool bSuccess)
	{
		if (bSuccess)
		{
			cout<<"连接成功！"<<endl;
		}
		else
		{
			cout<<"连接失败！"<<endl;
		}
	}
	virtual void  OnData(const char* pData, int ilen)
	{
		cout<<"OnData:"<<pData<<endl;
		if(this->Send(pData,ilen))
		{
			cout<<"Echo Succeed."<<endl;
		}
	}
protected:
private:
};

void TestConn2(int iPort)
{
	UDT::TConnectMngTemplate<ConnHandler> mng(iPort,NULL);

	cout<<"Echo Server 启动完成，端口:"<<iPort<<endl;
	cout<<"要主动连接其它服务器发送测试数据，请输入connect ip port命令。"<<endl;

	string strCmd;
	char buf[128];
	while(strCmd!="exit")
	{
		cin.getline(buf,128);

		vector<string> vecParams;
		StrFuns::Tokenize(buf,vecParams," ");
		if(vecParams.empty())
		{
			continue;
		}
		if(vecParams[0]=="exit")
		{
			break;
		}
		else if(vecParams[0]=="connect")
		{
			if(vecParams.size()!=3)
			{
				cout<<"param count should be 2."<<endl;
				continue;
			}
			cout<<"connecting to :"<<vecParams[1]<<endl;
			//ShootTo(*pNetbase,vecParams[1],atoi(vecParams[2].c_str()));

			sockaddr_in dist;
			dist.sin_family = AF_INET;
			dist.sin_addr.s_addr = inet_addr(vecParams[1].c_str());
			dist.sin_port = htons(atoi(vecParams[2].c_str()));

			ConnHandler conn(&mng);
			if(!conn.ConnectToEx((sockaddr *)&dist,sizeof(dist)))
			{
				cout<<"connect failed."<<endl;
				continue;
			}
			int i = 0;
			while (!conn.IsReady() && i<1000)
			{
				Sleep(10);
				i++;
			}
			//conn.ConnectToEx((sockaddr *)&dist,sizeof(dist));
			cout<<"发送触发数据"<<endl;
			if(!conn.Send("Hello,triger echo!",19))
			{
				cout<<"send error"<<endl;
				continue;
			}
			cout<<"等待10秒……"<<endl;
			Sleep(10*1000);

			//conn.Close(); //可以由析构函数自动调用
			cout<<"测试完成，连接断开"<<endl;
		}

		else
		{
			cout<<"Unknow cmd:"<<vecParams[0]<<endl;
			cout<<"cmd list:"<<endl;
			cout<<"\tconnect <addr> <port>"<<endl;
		}

	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	//启动UDT，连接5001端口，发送数据，等10秒钟显示回应，然后关闭。 
	int iPort = 0;
	if(argc==2)
	{
		iPort = atoi(argv[1]);
	}
	else
	{
		cout<<"输入您要使用的UDT端口号："<<endl;
		cin>>iPort;
		if (iPort<0 || iPort>=65535)
		{
			cout<<"UDT端口号输入错误"<<endl;
			getchar();
			return 0;
		}
	}
	TestConn2(iPort);
	return 0;
}

