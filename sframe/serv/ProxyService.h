
#ifndef SFRAME_PROXY_SERVICE_H
#define SFRAME_PROXY_SERVICE_H

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "AdminCmd.h"
#include "ServiceSession.h"
#include "Service.h"
#include "../util/RingQueue.h"
#include "ProxyServiceMsg.h"

namespace sframe {

// �������
class ProxyService : public Service
{
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

	// ע������������
	void RegistAdminCmd(const std::string & cmd, const AdminCmdHandleFunc & func);

private:

	int32_t GetNewSessionId();

	ServiceSession * GetServiceSession(int32_t session_id);

	void AddServiceSession(int32_t session_id, ServiceSession * session);

	void DeleteServiceSession(int32_t session_id);

	void OnMsg_SessionClosed(bool by_self, int32_t session_id);

	void OnMsg_SessionRecvData(int32_t session_id, const std::shared_ptr<std::vector<char>> & data);

	void OnMsg_SessionConnectCompleted(int32_t session_id, bool success);

	void OnMsg_AdminCommand(int32_t admin_session_id, const std::shared_ptr<sframe::HttpRequest> & http_req);

	void OnMsg_SendAdminCommandResponse(int32_t admin_session_id, const std::string & data, const  std::shared_ptr<sframe::HttpRequest> & http_req);

private:

	static const int kQuickFindSessionArrLen = 512;

	ServiceSession * _quick_find_session_arr[kQuickFindSessionArrLen];          // ��session_idС��kQuickFindSessionArrLen��session����һ�ݣ����ڿ��ٲ���
	std::unordered_map<int32_t, ServiceSession*> _all_sessions;                 // ����ServiceSession
	bool _have_no_session;
	std::unordered_map<int64_t, int32_t> _session_addr_to_sessionid;            // �����������ӵ�Session��Ŀ���ַ��sessionid��ӳ��
	std::unordered_map<int32_t, int32_t> _sid_to_sessionid;                     // Զ�̷���IDӳ�䵽�ỰID
	std::unordered_map<int32_t, std::unordered_set<int32_t>> _sessionid_to_sid; // �ỰIDӳ�䵽����ID
	bool _listening;                                                            // �Ƿ����ڼ���
	TimerManager _timer_mgr;                                                    // ��ʱ������
	int32_t _cur_max_session_id;
	bool _session_id_first_loop;
	std::unordered_map<std::string, AdminCmdHandleFunc> _map_admin_cmd_func;    // �����������
};

}

#endif
