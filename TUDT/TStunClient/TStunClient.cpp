// FileTester.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "tudt.h"
#include "debug.h"
#include "StrFuns.h"
#include "TClientPeer.h"
#include "TPeerProtocol.h"

//#include "NPeerMng.h"

T_PEERID g_iRemotePeerID = UDT::INVALID_PEER_ID;
//
//void OnConnect(int iRes)
//{
//	cout<<"OnConnect:"<<iRes<<endl;
//}

//TStunServ�Ĺ������£�
//1.�����˿ڣ��������е����ӣ�
//2.UDT���ӽ���֮��ִ�н���ID���̣�ȷ��TPEER���ӣ����ұ�����������ӣ�
//3.ά��ӳ���peer_id->���ܵĵ�ַ�б�;
//4.�����û��ı�����Ϣ�����û����ߵ�ʱ��ɾ������(UDT�Դ��˹��ܣ������Լ�����
//5.�����û��ĵ�ַ��ѯ���󣬷��ص�ַ�б�
//6.�����û��Ĵ��������ͻ��˷��ʹ������
int _tmain(int argc, _TCHAR* argv[])
{
	int iMyID = 10005;
	int iMyPort = 0;
	char *strServerIp = "127.0.0.1";
	short int iServerPort = 0;
	int iServerID = 5000;
	if(argc==3)
	{
		iMyID = atoi(argv[1]);
		iMyPort = atoi(argv[2]);
	}
	else if(argc==6)
	{
		iMyID = atoi(argv[1]);
		iMyPort = atoi(argv[2]);
		strServerIp = argv[3];
		iServerPort = atoi(argv[4]);
		iServerID = atoi(argv[5]);
	}
	else
	{
		cout<<"������������"<<argc<<endl;
		cout<<"�÷���TStunClient <myID> <myPort> <server_ip> <server_port> <server_id>"<<endl;
		return 0;
	}
	try
	{

		//UDT::TPeerMngTemplate<TClientPeer> mng(iMyID,0,NULL);
		KIpv4Addr addr((t_uint)0,iMyPort);
		SOCKADDR_IN si;
		addr.GetAddr(si);
		UDT::TPeerMngTemplate<TClientPeer> mng(iMyID,(sockaddr *)&si,sizeof(si));

		cout<<"Stun client ������� id="<<mng.GetId()<<endl;

		//cout<<"��������STUN�����������Ժ򡭡�"<<endl;

		//���������������
		sockaddr_in dist;
		dist.sin_family = AF_INET;
		dist.sin_addr.s_addr = inet_addr(strServerIp);
		dist.sin_port = htons(iServerPort);
		if (dist.sin_port!=0)  //�������˿ں�Ϊ�㼴��������������
		{
			
			cout<<"���ڳ�������STUN��������"<< KIpv4Addr(dist).ToString()<<endl;
			mng.ConnectToServer(iServerID,(sockaddr *)&dist,sizeof(dist));
			for (int i=0;i<100;i++)
			{
				if(mng.IsPeerReady(iServerID))
				{
					break;
				}
				Sleep(600);
			}
			if (!mng.IsPeerReady(iServerID))
			{
				cout<<"����STUN��������ʱ��"<<endl;
				return 0;
			}
		}

		string strCmd;
		const int c_iBufLen = 4096;
		char buf[c_iBufLen];

		//TClientPeer *g_pCurrPeer = NULL;
		while(strCmd!="exit")
		{
			cin.getline(buf,c_iBufLen-1);

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
			else if (vecParams[0]=="sendandexit")
			{
				int iPeer = atoi(vecParams[1].c_str());
				if (iPeer<=0)
				{
					cout<<"peer id error."<<endl;
					continue;
				}
				//TClientPeer *pPeer = (TClientPeer *)mng.FindPeer(iPeer);

				if (!mng.IsPeerReady(iPeer))
				{
					cout<<"peer not found."<<endl;
					continue;
				}

				buf[strlen(buf)+1] = '\0';

				int iCmdLen = (int)vecParams[0].size() + (int)vecParams[1].size() + 2;
				int iTotalLen = (int)strlen(buf)-iCmdLen+1;
				for (int i=0;i<1000;)
				{
					char sendbuf[1024*4];
					sprintf_s(sendbuf,1024*4,"MSG %d:\t%s\n",i,buf + iCmdLen);
					if(mng.SendPeerMsg(iPeer,sendbuf,strlen(sendbuf)+1)>0)
					{
						cout<<(int)strlen(buf)-4<<" bytes sent."<<endl;
						i++;
					}
					else
					{
						cout<<"send error."<<endl;
						Sleep(100);
					}
				}
				//return 0;
				continue;
			}
			else if (vecParams[0]=="send")
			{
				int iPeer = atoi(vecParams[1].c_str());
				if (iPeer<=0)
				{
					cout<<"peer id error."<<endl;
					continue;
				}
				//TClientPeer *pPeer = (TClientPeer *)mng.FindPeer(iPeer);

				if (!mng.IsPeerReady(iPeer))
				{
					cout<<"peer not found."<<endl;
					continue;
				}

				buf[strlen(buf)+1] = '\0';

				int iCmdLen = (int)vecParams[0].size() + (int)vecParams[1].size() + 2;
				int iTotalLen = (int)strlen(buf)-iCmdLen+1;
				if(mng.SendPeerMsg(iPeer,buf + iCmdLen,iTotalLen)==iTotalLen)
				{
					cout<<(int)strlen(buf)-4<<" bytes sent."<<endl;
				}
				else
				{
					cout<<"send error."<<endl;
				}
			}
			else if(vecParams[0]=="peer")
			{
				if(vecParams.size()!=2)
				{
					cout<<"param count should be 1."<<endl;
					continue;
				}
				T_PEERID uiPeer = atoi(vecParams[1].c_str());
				if (uiPeer<=0)
				{
					cout<<"id error:"<<uiPeer<<endl;
					continue;
				}
				if (uiPeer==mng.GetId())
				{
					cout<<"����������Լ���ID:"<<uiPeer<<endl;
					continue;
				}

				if (mng.IsPeerExist(uiPeer))
				{
					cout<<"�����Ѿ�����:"<<uiPeer<<endl;
					g_iRemotePeerID = uiPeer;
					if (!mng.IsPeerReady(uiPeer))
					{
						cout<<"���Ӵ��ڵ����ڶϿ�״̬,�����������ӡ���"<<endl;
						if(mng.PeerConnect(uiPeer,5000))
						{
							cout<<"���ӳɹ�"<<endl;
						}
						else
						{
							cout<<"����ʧ��"<<endl;
						}
					}
					continue;
				}
				if (g_iRemotePeerID!=UDT::INVALID_PEER_ID)
				{
					//NPeer *pOld = mng.ClosePeer(g_iRemotePeerID);
					//if (NULL!=pOld)
					//{
					//	delete pOld;
					//}
					mng.ClosePeer(g_iRemotePeerID);
				}
				g_iRemotePeerID = uiPeer;

				//NPeer *pPeer = new NPeer(&mng,g_iRemotePeerID);
				UDT::TPeerPtr pPeer = mng.GetPeerInstance(g_iRemotePeerID);

				int iWaitTime = 5000;
				if(!pPeer->PeerConnect(iWaitTime))
				{
					cout<<"����ʧ��"<<endl;
					continue;
				}
				cout<<"���ӳɹ������Ͳ������ݡ�"<<endl;
				if(!pPeer->SendPeerMsg("Hello,triger echo!",19))
				{
					cout<<"send error"<<endl;
					continue;
				}
				//cout<<"�ȴ�10�롭��"<<endl;
				//Sleep(10*1000);

				//conn.Close(); //���������������Զ�����
				//cout<<"������ɣ����ӶϿ�"<<endl;
			}
			else if(vecParams[0]=="connect")
			{
				if(vecParams.size()!=4)
				{
					cout<<"param count should be 3."<<endl;
					continue;
				}
				cout<<"connecting to :"<<vecParams[1]<<endl;
				//ShootTo(*pNetbase,vecParams[1],atoi(vecParams[2].c_str()));

				sockaddr_in dist;
				dist.sin_family = AF_INET;
				dist.sin_addr.s_addr = inet_addr(vecParams[1].c_str());
				dist.sin_port = htons(atoi(vecParams[2].c_str()));
				int iPeerID = atoi(vecParams[3].c_str());

				UDT::TPeerPtr ptrPeer = mng.GetPeerInstance(iPeerID);
				if(!ptrPeer->PeerConnect((sockaddr *)&dist,sizeof(dist)))
				{
					cout<<"connect failed."<<endl;
					continue;
				}
				for(int i=0;i<300;i++)
				{
					if(!ptrPeer->IsReady())
					{
						Sleep(10);
					}
				}

				if(!ptrPeer->IsReady())
				{
					cout<<"����ʧ��"<<endl;
					continue;
				}
				cout<<"���ʹ�������"<<endl;
				if(!ptrPeer->SendPeerMsg("Hello,triger echo!",19))
				{
					cout<<"send error"<<endl;
					continue;
				}
				cout<<"�ȴ�10�롭��"<<endl;
				Sleep(10*1000);

				//conn.Close(); //���������������Զ�����
				cout<<"������ɣ����ӶϿ�"<<endl;
				mng.ClosePeer(iPeerID);

			}

			else
			{
				cout<<"Unknow cmd:"<<vecParams[0]<<endl;
				cout<<"cmd list:"<<endl;
				cout<<"\tsend <message>"<<endl;
				cout<<"\tpeer <id>"<<endl;
				cout<<"\tconnect <ip> <port>"<<endl;
				cout<<"\texit"<<endl;
			}

		}
	}
	catch (std::exception &e)
	{
		cout<<"�����쳣�˳�������˿��Ƿ��ѱ�ռ��:"<<e.what()<<endl;
	}
	catch (...) {

		cout<<"�����쳣�˳�������˿��Ƿ��ѱ�ռ�á�"<<endl;
	}
	return 0;
}

