
#include "ServiceDispatcher.h"
#include "ProxyService.h"
#include "../net/SocketAddr.h"
#include "../util/Log.h"
#include "../util/TimeHelper.h"

using namespace sframe;

ProxyService::ProxyService() :  _session_num(0), _session_id_queue(kMaxSessionNumber), _listening(false)
{
	memset(_session, 0, sizeof(_session));

	// ��ʼ��session_id����
	for (int i = 1; i <= kMaxSessionNumber; i++)
	{
		_session_id_queue.Push(i);
	}
}

ProxyService::~ProxyService()
{
	for (int i = 1; i <= kMaxSessionNumber; i++)
	{
		if (_session[i] != nullptr)
		{
			delete _session[i];
		}
	}
}

void ProxyService::Init()
{
	// ע����Ϣ������
	this->RegistInsideServiceMessageHandler(kProxyServiceMsgId_SessionClosed, &ProxyService::OnMsg_SessionClosed, this);
	this->RegistInsideServiceMessageHandler(kProxyServiceMsgId_SessionRecvData, &ProxyService::OnMsg_SessionRecvData, this);
	this->RegistInsideServiceMessageHandler(kProxyServiceMsgId_SessionConnectCompleted, &ProxyService::OnMsg_SessionConnectCompleted, this);
}

void ProxyService::OnDestroy()
{
	for (int i = 1; i <= kMaxSessionNumber; i++)
	{
		if (_session[i] != nullptr)
		{
			_session[i]->Close();
		}
	}
}

bool ProxyService::IsDestroyCompleted() const
{
	return (_session_num <= 0);
}

// �������ڶ�ʱ��
void ProxyService::OnCycleTimer()
{
	_timer_mgr.Execute();
}

// �����ӵ���
void ProxyService::OnNewConnection(const ListenAddress & listen_addr_info, const std::shared_ptr<sframe::TcpSocket> & sock)
{
	Error err = sock->SetTcpNodelay(true);
	if (err)
	{
		LOG_WARN << "Set tcp nodelay error|" << err.Code() << "|" << sframe::ErrorMessage(err).Message() << ENDL;
	}

	int32_t session_id = -1;
	if (!_session_id_queue.Pop(&session_id))
	{
		sock->Close();
		LOG_ERROR << "ServiceSession number upper limit|session number|" << _session_num << ENDL;
		return;
	}

	assert(_session[session_id] == nullptr);
	_session[session_id] = new ServiceSession(session_id, this, sock);
	_session_num++;
	_session[session_id]->SetTimerManager(&_timer_mgr);
	_session[session_id]->Init();
}

// ���������Ϣ
void ProxyService::OnProxyServiceMessage(const std::shared_ptr<ProxyServiceMessage> & msg)
{
	auto it = _sid_to_sessionid.find(msg->dest_sid);
	if (it == _sid_to_sessionid.end())
	{
		return;
	}

	int32_t sessionid = it->second;
	assert(sessionid > 0 && sessionid <= kMaxSessionNumber && _session[sessionid]);
	// ���÷���
	_session[sessionid]->SendData(msg);
}

#define MAKE_ADDR_INFO(ip, port) ((((int64_t)(ip) & 0xffffffff) << 16) | ((int64_t)(port) & 0xffff))

// ע��Ự
// ���ػỰID��С��0ʧ��
int32_t ProxyService::RegistSession(int32_t sid, const std::string & remote_ip, uint16_t remote_port)
{
	auto it_sid_to_sessionid = _sid_to_sessionid.find(sid);
	if (it_sid_to_sessionid != _sid_to_sessionid.end())
	{
		assert(it_sid_to_sessionid->second > 0);
		return it_sid_to_sessionid->second;
	}

	int32_t session_id = -1;
	SocketAddr sock_addr(remote_ip.c_str(), remote_port);
	int64_t addr_info = MAKE_ADDR_INFO(sock_addr.GetIp(), sock_addr.GetPort());
	auto it_session_id = _session_addr_to_sessionid.find(addr_info);
	if (it_session_id == _session_addr_to_sessionid.end())
	{
		// ��û����ͬĿ�ĵ�ַ��session���½�һ��
		if (!_session_id_queue.Pop(&session_id))
		{
			return -1;
		}

		assert(_session[session_id] == nullptr);
		_session[session_id] = new ServiceSession(session_id, this, remote_ip, remote_port);
		_session_num++;
		_session[session_id]->SetTimerManager(&_timer_mgr);
		_session[session_id]->Init();
		_session_addr_to_sessionid[addr_info] = session_id;
	}
	else
	{
		session_id = it_session_id->second;
	}

	assert(session_id > 0 && session_id <= kMaxSessionNumber && _session[session_id]);
	// ��ӻỰ�����ķ���
	_sessionid_to_sid[session_id].insert(sid);
	// ���sid��sessionid��ӳ��
	_sid_to_sessionid[sid] = session_id;

	return session_id;
}

ServiceSession * ProxyService::GetServiceSessionById(int32_t session_id)
{
	assert(session_id > 0 && session_id <= kMaxSessionNumber);
	return _session[session_id];
}


void ProxyService::OnMsg_SessionClosed(bool by_self, int32_t session_id)
{
	assert(_session[session_id]);

	// �Ƿ�Ҫɾ��session
	if (!_session[session_id]->TryFree())
	{
		return;
	}

	// ɾ��session
	delete _session[session_id];
	_session[session_id] = nullptr;
	_session_num--;
	_session_id_queue.Push(session_id);

	// ɾ��������¼
	auto it_sid = _sessionid_to_sid.find(session_id);
	if (it_sid != _sessionid_to_sid.end())
	{
		for (int32_t rm_sid : it_sid->second)
		{
			auto it_sid_to_sessionid = _sid_to_sessionid.find(rm_sid);
			if (it_sid_to_sessionid != _sid_to_sessionid.end() && it_sid_to_sessionid->second == session_id)
			{
				_sid_to_sessionid.erase(rm_sid);
			}
			else
			{
				assert(false);
			}
		}

		_sessionid_to_sid.erase(session_id);
	}
}

void ProxyService::OnMsg_SessionRecvData(int32_t session_id, const std::shared_ptr<std::vector<char>> & data)
{
	assert(_session[session_id]);

	std::vector<char> & vec_data = *data;
	char * p = &vec_data[0];
	uint32_t len = (uint32_t)vec_data.size();
	StreamReader reader(p, len);

	// ��ȡ��Ϣͷ��
	int32_t src_sid = 0;
	int32_t dest_sid = 0;
	int64_t msg_session_key = 0;
	uint16_t msg_id = 0;
	if (!AutoDecode(reader, src_sid, dest_sid, msg_session_key, msg_id))
	{
		LOG_ERROR << "decode net service message error" << std::endl;
		return;
	}

	// Դ����ID�Ƿ�ͱ��ط���ID��ͻ
	if (ServiceDispatcher::Instance().IsLocalService(src_sid))
	{
		LOG_ERROR << "service message, src_id conflict with local service|" << src_sid << std::endl;
		return;
	}

	// �����Ƿ���Ŀ�����
	if (!ServiceDispatcher::Instance().IsLocalService(dest_sid))
	{
		LOG_ERROR << "service message, have no local service|" << dest_sid << std::endl;
		return;
	}

	// ����Զ�̷����Ƿ��Ѿ�������session������û�й��������������
	auto it_sid_to_sessionid = _sid_to_sessionid.find(src_sid);
	if (it_sid_to_sessionid == _sid_to_sessionid.end())
	{
		_sid_to_sessionid[src_sid] = session_id;
		_sessionid_to_sid[session_id].insert(src_sid);
	}

	// ��װ��Ϣ�����͵�Ŀ�걾�ط���
	int32_t data_len = (int32_t)len - (int32_t)reader.GetReadedLength();
	assert(data_len >= 0);
	std::shared_ptr<NetServiceMessage> msg = std::make_shared<NetServiceMessage>();
	msg->dest_sid = dest_sid;
	msg->src_sid = src_sid;
	msg->session_key = msg_session_key;
	msg->msg_id = msg_id;
	msg->data = std::move(vec_data);
	// ���͵�Ŀ�����
	ServiceDispatcher::Instance().SendMsg(dest_sid, msg);
}

void ProxyService::OnMsg_SessionConnectCompleted(int32_t session_id, bool success)
{
	assert(_session[session_id]);
	_session[session_id]->DoConnectCompleted(success);
}
