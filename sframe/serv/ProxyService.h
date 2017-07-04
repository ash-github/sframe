
#ifndef SFRAME_PROXY_SERVICE_H
#define SFRAME_PROXY_SERVICE_H

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "ServiceSession.h"
#include "Service.h"
#include "../util/RingQueue.h"
#include "ProxyServiceMsg.h"

namespace sframe {

// �������
class ProxyService : public Service
{
public:
	static const int kMaxSessionNumber = 1024;

public:

	ProxyService();

	virtual ~ProxyService();

	void Init() override;

	void OnDestroy() override;

	bool IsDestroyCompleted() const override;

	int32_t GetCyclePeriod() const override
	{
		return 1000;
	}

	// �������ڶ�ʱ��
	void OnCycleTimer() override;

	// �����ӵ���
	void OnNewConnection(const ListenAddress & listen_addr_info, const std::shared_ptr<sframe::TcpSocket> & sock) override;

	// ���������Ϣ
	void OnProxyServiceMessage(const std::shared_ptr<ProxyServiceMessage> & msg) override;

	// ע��Ự
	// ���ػỰID������0��������������Ϊʧ��
	int32_t RegistSession(int32_t sid, const std::string & remote_ip, uint16_t remote_port);

	// ��ȡ�Ự
	ServiceSession * GetServiceSessionById(int32_t session_id);

private:

	void OnMsg_SessionClosed(bool by_self, int32_t session_id);

	void OnMsg_SessionRecvData(int32_t session_id, const std::shared_ptr<std::vector<char>> & data);

	void OnMsg_SessionConnectCompleted(int32_t session_id, bool success);

private:
	ServiceSession * _session[kMaxSessionNumber + 1];
	int32_t _session_num;
	std::unordered_map<int64_t, int32_t> _session_addr_to_sessionid;            // �����������ӵ�Session��Ŀ���ַ��sessionid��ӳ��
	std::unordered_map<int32_t, int32_t> _sid_to_sessionid;                     // Զ�̷���IDӳ�䵽�ỰID
	std::unordered_map<int32_t, std::unordered_set<int32_t>> _sessionid_to_sid; // �ỰIDӳ�䵽����ID
	RingQueue<int32_t> _session_id_queue;
	bool _listening;           // �Ƿ����ڼ���
	TimerManager _timer_mgr;   // ��ʱ������
};

}

#endif
