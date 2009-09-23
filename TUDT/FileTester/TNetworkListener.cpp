#include "StdAfx.h"
#include <iostream>
#include <string>
#include <assert.h>
using namespace std;

#include "TNetworkListener.h"
#include "StrFuns.h"

const int BufferSize = 102400;

TNetworkListener::TNetworkListener(void)
{
}

TNetworkListener::~TNetworkListener(void)
{
}


void TNetworkListener::OnConnectionMsg(int iConn,enNetworkCode code,int param)
{
	cout<<"OnConnectionMsg:"<<iConn<<","<<code<<","<<param<<endl;

	if (NL_CODE_NEWCONN==code)
	{
		SOCKADDR_IN addr;
		int iLen = 0;
		if(UDT::ERROR==UDT::getpeername(iConn,(sockaddr *)&addr,&iLen))
		{
			assert(false);
			cout<<"OnConnectionMsg ERROR:"<<UDT::getlasterror().getErrorMessage()<<endl;
			return;
		}
		cout<<"peer online:"<<inet_ntoa(addr.sin_addr)<<" "<<ntohs(addr.sin_port)<<endl;
		string msg;
		for (int i=0;i<3;i++)
		{
			msg += "HELLO?WELLCOME,udt tester!\n";
		}
		UDT::sendmsg(iConn,msg.c_str(),(int)msg.size()+1);

		m_setConns.insert(iConn);
	}
	else if (NL_CODE_BREAKDOWN==code)
	{
		cout<<"connection breakdown."<<endl;
	}
	else
	{
		cout<<"MSG:"<<code<<endl;
	}
}
void TNetworkListener::OnData(int iConn,int iLen, const char *pData)
{
	//显示收到的数据
	cout<<"OnData:"<<iConn<<","<<iLen<<endl;//<<":";
	//cout.write(pData,iLen);
	//cout<<endl;

	//如果这些数据是一些命令，处理它
	vector<string> vecCmd;
	StrFuns::Tokenize(string(pData,iLen),vecCmd," ");
	if (vecCmd.empty())
	{
		return;
	}
	else if (vecCmd[0]=="TUDTCMD:FILE") //接收文件命令
	{
		if(vecCmd.size()!=3) //TUDTCMD:FILE file_name file_size
		{
			cout<<"received file cmd, but param is error."<<endl;
			return;
		}
		int file_size = atoi(vecCmd[2].c_str());
		if (file_size<=0)
		{
			cout<<"received file cmd, but filesize is 0."<<endl;
			return;
		}
		string file_path = "C:\\udt\\" + vecCmd[1];
		cout << "receive a file from "<<iConn
			<<", size="<<file_size
			<<"saving to"<<file_path<<endl;
		//ofstream ofs(file_path.c_str(), ios::out | ios::binary | ios::trunc);
		//if (ofs.fail() || ofs.bad())
		//{
		//	cout<<"open file error."<<endl;
		//	return ;
		//}
		FILE *f = fopen(file_path.c_str(),"wb");
		if (!f)
		{
			cout<<"open file error."<<endl;
			assert(false);
			return;
		}

		char buf[BufferSize]; //后面也用到
		int recvsize = 0;
		int totalRecv = 0;
		DWORD dwRecvTime = GetTickCount();
		while (true)
		{
			recvsize = UDT::recvmsg(iConn,buf,BufferSize);
			if (UDT::ERROR == recvsize)
			{
				if (GetTickCount() - dwRecvTime<10*1000) //不能连续10秒收不到一个数据包
				{
					Sleep(10);
					continue;
				}
				
				cout << "recvfile error: " << UDT::getlasterror().getErrorMessage() << endl;
				fclose(f);
				return ;
			}
			fwrite(buf,1,recvsize,f);
			totalRecv += recvsize;
			cout<<"recv "<<recvsize<<" bytes(total "<<totalRecv<<")"<<endl;
			if (totalRecv>=file_size)
			{
				break;
			}
			dwRecvTime = GetTickCount();
		}
		if (totalRecv!=file_size)
		{
			cout<< "error: recvsize="<<recvsize<<endl;
			return;
		}
		//ofs.close();
		fflush(f);
		fclose(f);
		cout << "recv file succeed."<<endl;

	}
	else if (vecCmd[0]=="TUDTCMD:GET_FILE") //请求文件
	{
		if(vecCmd.size()!=2) //TUDTCMD:GET_FILE file_path
		{
			cout<<"received file cmd, but param is error."<<endl;
			return;
		}
		string file_path = vecCmd[1] + "\0";
		cout<< "prepare sending file :"<< file_path <<endl;

		//ifstream ifs(file_path.c_str(), ios::in | ios::binary);		
		FILE *f = fopen(file_path.c_str(),"rb");
		if (!f)
		{
			cout<<"open file error."<<endl;
			//assert(false);
			fclose(f);
			return;
		}

		//ifs.seekg(0, ios::end);
		//int64_t size = ifs.tellg();
		//ifs.seekg(0, ios::beg);
		fseek(f,0,SEEK_END);
		int size = ftell(f);
		fseek(f,0,SEEK_SET);

		//TUDTCMD:FILE file_name file_size
		vector<string> vecNames;
		StrFuns::Tokenize(vecCmd[1],vecNames,"\\");
		if (vecNames.empty())
		{
			cout<<"received file cmd, but param is error2."<<endl;
			fclose(f);
			return;
		}
		char buf[BufferSize]; //后面也用到
		string strCmd = "TUDTCMD:FILE " + vecNames.back() + " " + itoa((int)size,buf,10) + "\0";
		UDT::sendmsg(iConn,strCmd.c_str(),(int)strCmd.size(),0);

		cout << "sending file..."<< endl;

		int pktlen = 0;
		int totolsend = 0;
		while (true)
		{
			//if (ifs.bad() || ifs.fail() || ifs.eof())
			//	break;
			//ifs.read(buf,BufferSize);
			pktlen = (int)fread(buf,1,BufferSize,f);
			if (pktlen<=0)
			{
				break;
			}
			
			//if ((pktlen = ifs.gcount()) <= 0)
			//	break;

			totolsend += pktlen;
			DWORD dwBgn = GetTickCount();
			while(UDT::ERROR == UDT::sendmsg(iConn,buf,pktlen,-1,true)) //发送错误，只是缓冲区不够
			{
				if (GetTickCount()-dwBgn>10000)
				{
					cout << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
					return;
				}
				cout<<"total sent "<<totolsend<<" bytes"<<endl;
				Sleep(100);
			}

		}
		if (totolsend!=size)
		{
			cout<<"文件发送完毕，但文件大小错误："<<totolsend<<endl;
		}
		fclose(f);

		cout << "sendfile finished."<< endl;

	}
	else
	{
		//cout.write(pData,iLen);
		//cout<<endl;
	}
	
}