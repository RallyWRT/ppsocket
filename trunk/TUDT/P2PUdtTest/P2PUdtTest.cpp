// P2PUdtTest.cpp : �������̨Ӧ�ó������ڵ㡣
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
			cout<<"���ӳɹ���"<<endl;
		}
		else
		{
			cout<<"����ʧ�ܣ�"<<endl;
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

	cout<<"Echo Server ������ɣ��˿�:"<<iPort<<endl;
	cout<<"Ҫ���������������������Ͳ������ݣ�������connect ip port���"<<endl;

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
			cout<<"���ʹ�������"<<endl;
			if(!conn.Send("Hello,triger echo!",19))
			{
				cout<<"send error"<<endl;
				continue;
			}
			cout<<"�ȴ�10�롭��"<<endl;
			Sleep(10*1000);

			//conn.Close(); //���������������Զ�����
			cout<<"������ɣ����ӶϿ�"<<endl;
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
	//����UDT������5001�˿ڣ��������ݣ���10������ʾ��Ӧ��Ȼ��رա� 
	int iPort = 0;
	if(argc==2)
	{
		iPort = atoi(argv[1]);
	}
	else
	{
		cout<<"������Ҫʹ�õ�UDT�˿ںţ�"<<endl;
		cin>>iPort;
		if (iPort<0 || iPort>=65535)
		{
			cout<<"UDT�˿ں��������"<<endl;
			getchar();
			return 0;
		}
	}
	TestConn2(iPort);
	return 0;
}

