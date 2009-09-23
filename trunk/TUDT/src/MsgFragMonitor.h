#pragma once
#include <map>
#include <set>
#include "common.h"

class MsgFragMonitor
{
public:
	MsgFragMonitor(void);
	~MsgFragMonitor(void);

	int CreateMsg();
	void AddMsgFrag(int msgId,int fragId);
	std::pair<bool,int> OnFragACK(int fragId);
private:
	std::map<int, std::set<int> > m_mapMsgID;
	int m_currId;
	CGuardMutex m_ConnMutex;
};
