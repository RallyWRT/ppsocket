// FileTester.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "udt.h"
#include "TNetworkListener.h"
#include "StrFuns.h"

void ShootTo(UDT::P2PNetworkBase &netbase,const string &strDist, unsigned short usPort)
{
	sockaddr_in dist;
	dist.sin_family = AF_INET;
	dist.sin_addr.s_addr = inet_addr(strDist.c_str());
	dist.sin_port = htons(usPort);
	netbase.ShootTo((sockaddr *)&dist,sizeof(sockaddr_in));
}

int Param2PeerId( const string &paramId, UDT::P2PNetworkBase * pNetbase )
{
	int peer_id = atoi(paramId.c_str());
	if (peer_id==0)
	{
		std::set<UDTSOCKET> setConns;
		pNetbase->GetConns(setConns);
		if (setConns.empty())
		{
			cout<<"No connection exist!"<<endl;
			//return 0;
		}
		peer_id = (*setConns.begin());
	}
	return peer_id;
}

void ShowHelp()
{
	cout<<"FileTester 1.0"<<endl;
	cout<<"此程序用于测试P2PNetworkBase接口，也可用以辅助TConnection、TConnectMng接口的测试。"<<endl;
	cout<<"程序启动后，即能接受连接、接收字符串、处理文件下载请求。"<<endl;
	cout<<"命令列表:"<<endl;
	cout<<"connect <addr> <port>\n\t- 与指定地址连接，成功后会向对方发送helloworld"<<endl;
	cout<<"send <conn_id> <string>\n\t- 向指定连接（conn_id，可用0代表第一个连接）发送字符串"<<endl;
	cout<<"get <conn_id> <dist_path>\n\t- 向指定连接（conn_id，可用0代表第一个连接）请求下载dist_path文件，文件保存在c:\\udt目录下"<<endl;
	cout<<"exit\n\t- 退出程序"<<endl;
}
int _tmain(int argc, _TCHAR* argv[])
{
	ShowHelp();
	int iPort = 0;
	if(argc==2)
	{
		iPort = atoi(argv[1]);
	}
	else
	{
		cout<<"输入您要使用的UDT端口号："<<endl;
		cin>>iPort;
	}
	UDT::P2PNetworkBase *pNetbase = NULL;
	try
	{
		pNetbase = new UDT::P2PNetworkBase(iPort);
	}
	catch (...)
	{
		cout<<"P2PNetworkBase 创建失败。请检查是否端口被占用。"<<endl;
		cout<<"press any key to continue..."<<endl;
		int c;
		cin>>c;
		return 0;
	}
	cout<<"网络层启动成功，端口号："<<pNetbase->GetPort()<<endl;
	TNetworkListener listener;
	pNetbase->SetNetworkListener(&listener);

	string strCmd;
	char buf[128];
	while(strCmd!="exit")
	{
		cin.getline(buf,128);

		vector<string> vecParams;
		//cout<<"CMD:"<<strCmd<<endl;
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
			ShootTo(*pNetbase,vecParams[1],atoi(vecParams[2].c_str()));
			cout<<"connect finished."<<endl;
		}
		else if(vecParams[0]=="send")
		{
			if(vecParams.size()!=3)
			{
				cout<<"param count should be 2."<<endl;
				continue;
			}
			int peer_id = Param2PeerId(vecParams[1], pNetbase);
			pNetbase->SendTo(peer_id,(int)vecParams[2].size()+1,vecParams[2].c_str());
		}
		else if(vecParams[0]=="get")
		{
			if(vecParams.size()!=3) //get peer_id file_path local_path
			{
				cout<<"param count should be 2."<<endl;
				continue;
			}
			int peer_id = Param2PeerId(vecParams[1], pNetbase);

			string strCmd = "TUDTCMD:GET_FILE " + vecParams[2];
			cout<<"send cmd:"<<strCmd<<endl;
			pNetbase->SendTo(peer_id,(int)strCmd.size(),strCmd.c_str());
		}
		else
		{
			cout<<"Unknow cmd:"<<vecParams[0]<<endl;
			ShowHelp();

		}

	}
	delete pNetbase;
	return 0;
}

