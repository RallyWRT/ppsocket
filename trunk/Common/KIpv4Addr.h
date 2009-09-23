#pragma once
#include <winsock.h>
#include <string>

#pragma warning(disable:4200) //微软非标准扩展
#pragma pack(1)
// 常数定义
// 预定义数据长度
typedef unsigned __int8  t_uchar;	// 长度为一字节的无符号类型
typedef unsigned __int16 t_ushort;	// 长度为二字节的无符号类型
typedef unsigned __int32 t_uint;	// 长度为四字节的无符号类型
typedef unsigned __int64 t_uword;	// 长度为八字节的无符号类型

// IPv4的地址包
struct KIpv4Addr
{
	t_uint uiIp;		// IP地址
	t_ushort usPort;	// 端口号

public:
	//=================================================
	//构造函数
	//KIpv4Addr();
	//KIpv4Addr(t_uint uiIp,t_ushort usPort);
	//KIpv4Addr(const char *strIp,t_ushort usPort);
	//KIpv4Addr(SOCKADDR_IN addr);
	//KIpv4Addr(const KIpv4Addr &addr);

	////计算该地址的各种表示形式
	//void GetAddr(SOCKADDR_IN &addr) const;
	//void GetAddr(t_uint &uiIp,t_ushort &usPort) const;
	//void GetAddr(std::string &strIp,t_ushort &usPort) const;
	//std::string ToString() const;
	KIpv4Addr::KIpv4Addr(t_uint uiIp,t_ushort usPort)
	{
		this->uiIp = uiIp;
		this->usPort = usPort;
	}
	KIpv4Addr::KIpv4Addr(const char *strIp,t_ushort usPort)
	{
		this->uiIp = ntohl(inet_addr(strIp));
		this->usPort = usPort;
	}
	KIpv4Addr::KIpv4Addr(SOCKADDR_IN addr)
	{
		this->uiIp = ntohl(addr.sin_addr.S_un.S_addr);
		this->usPort = ntohs(addr.sin_port);
	}
	KIpv4Addr::KIpv4Addr(const KIpv4Addr &addr)
	{
		this->uiIp = addr.uiIp;
		this->usPort = addr.usPort;
	}

	KIpv4Addr::KIpv4Addr()
	{
		uiIp = 0;
		usPort = 0;
	}
	void KIpv4Addr::GetAddr(SOCKADDR_IN &addr) const
	{
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(uiIp);
		addr.sin_port = htons(usPort);
	}
	void KIpv4Addr::GetAddr(t_uint &uiIp,t_ushort &usPort) const
	{
		uiIp = this->uiIp;
		usPort = this->usPort;
	}
	void KIpv4Addr::GetAddr(std::string &strIp,t_ushort &usPort) const
	{
		SOCKADDR_IN addr;
		GetAddr(addr);
		strIp = inet_ntoa(addr.sin_addr);
		usPort = this->usPort;
	}
	bool KIpv4Addr::operator == (const KIpv4Addr &addr) const
	{
		return (this->uiIp==addr.uiIp) && (this->usPort==addr.usPort);
	}

	bool KIpv4Addr::operator != (const KIpv4Addr &addr) const
	{
		return (this->uiIp!=addr.uiIp) || (this->usPort!=addr.usPort);
	}

	const KIpv4Addr & KIpv4Addr::operator = (const KIpv4Addr &addr)
	{
		this->uiIp = addr.uiIp;
		this->usPort = addr.usPort;
		return *this;
	}
	std::string KIpv4Addr::ToString() const
	{
		std::string strIp;
		t_ushort usPort;
		GetAddr(strIp,usPort);
		char buf[16];
		//_itoa_s(usPort,buf,16,10);
		itoa(usPort,buf,10);
		return strIp + ":" + buf;
	}

};




#pragma pack () //字节对齐方式设回默认值 