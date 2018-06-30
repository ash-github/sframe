﻿
#ifndef __SSMSG_H__
#define __SSMSG_H__

#include <inttypes.h>
#include <vector>
#include <memory>
#include "util/Serialization.h"

enum GateMsgId : uint16_t
{
	kGateMsg_Start = 1,

	kGateMsg_SessionClosed = kGateMsg_Start,
	kGateMsg_SessionRecvData,
	kGateMsg_SendToClient,

	kGateMsg_End = 100
};

enum WorkMsgId : uint16_t
{
	kWorkMsg_Start = 101,

	kWorkMsg_ClientData = kWorkMsg_Start,
	kWorkMsg_EnterWorkService,
	kWorkMsg_QuitWorkService,

	kWorkMsg_End
};

enum HttpMsgId : uint16_t
{
	kHttpMsg_Start = 201,

	kHttpMsg_HttpRequest = kHttpMsg_Start,
	kHttpMsg_HttpSessionClosed,

	kHttpMsg_End
};

struct GateMsg_SendToClient
{
	DECLARE_SERIALIZE;

	int64_t session_id;
	std::shared_ptr<std::vector<char>> client_data;
};

struct WorkMsg_ClientData
{
	DECLARE_SERIALIZE;

	int32_t gate_sid;
	int64_t session_id;
	std::shared_ptr<std::vector<char>> client_data;
};

#endif