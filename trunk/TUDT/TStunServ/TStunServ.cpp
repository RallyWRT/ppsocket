// FileTester.cpp : �������̨Ӧ�ó������ڵ㡣
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

//TStunServ�Ĺ������£�
//1.�����˿ڣ��������е����ӣ�
//2.UDT���ӽ���֮��ִ�н���ID���̣�ȷ��TPEER���ӣ����ұ�����������ӣ�
//3.ά��ӳ���peer_id->���ܵĵ�ַ�б�;
//4.�����û��ı�����Ϣ�����û����ߵ�ʱ��ɾ������(UDT�Դ��˹��ܣ������Լ�����
//5.�����û��ĵ�ַ��ѯ���󣬷��ص�ַ�б�
//6.�����û��Ĵ��������ͻ��˷��ʹ������
int _tmain(int argc, _TCHAR* argv[])
{
	{
		//new char[1000]; //����ڴ�й©
		int iPort = 0;
		int iMyID = 5000;
		if(argc==3)
		{
			iPort = atoi(argv[1]);
			iMyID = atoi(argv[2]);
		}
		else
		{
			cout<<"������Ҫʹ�õ�UDT�˿ںţ�"<<endl;
			cin>>iPort;
			cout<<"������Ҫʹ�õ�PeerID��"<<endl;
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

