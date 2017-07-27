
#ifndef SFRAME_SERVICE_SESSION_H
#define SFRAME_SERVICE_SESSION_H

#include <list>
#include "../util/Serialization.h"
#include "../net/net.h"
#include "../util/Singleton.h"
#include "../util/Timer.h"
#include "../util/Http.h"

namespace sframe {

class ProxyService;

// ����Ự����Ҫ�����������еķ����ͨ�ţ�
class ServiceSession : public TcpSocket::Monitor, public noncopyable, public SafeTimerRegistor<ServiceSession>
{
public:
	// �Ự״̬
	enum SessionState : int32_t
	{
		kSessionState_Initialize = 0,    // ��ʼ״̬
		kSessionState_WaitConnect,       // �ȴ�����
		kSessionState_Connecting,        // ��������
		kSessionState_Running,           // ������
	};

	static const int32_t kReconnectInterval = 3000;       // �Զ��������

public:
	ServiceSession(int32_t id, ProxyService * proxy_service, const std::string & remote_ip, uint16_t remote_port);

	ServiceSession(int32_t id, ProxyService * proxy_service, const std::shared_ptr<TcpSocket> & sock);

	virtual ~ServiceSession(){}

	void Init();

	// �ر�
	void Close();

	// �����ͷ�
	bool TryFree();

	// ������ɴ���
	void DoConnectCompleted(bool success);

	// ��������
	void SendData(const std::shared_ptr<ProxyServiceMessage> & msg);

	// ��������
	void SendData(const char * data, size_t len);

	// ��ȡ��ַ
	std::string GetRemoteAddrText() const;

	// ���յ�����
	// ����ʣ���������
	virtual int32_t OnReceived(char * data, int32_t len) override;

	// Socket�ر�
	// by_self: true��ʾ��������Ĺرղ���
	virtual void OnClosed(bool by_self, Error err) override;

	// ���Ӳ������
	virtual void OnConnected(Error err) override;

	// ��ȡSessionId
	int32_t GetSessionId()
	{
		return _session_id;
	}

	// ��ȡ״̬
	SessionState GetState() const
	{
		return _state;
	}

private:

	// ��ʼ���Ӷ�ʱ��
	void SetConnectTimer(int32_t after_ms);

	// ��ʱ������
	int32_t OnTimer_Connect();

	// ��ʼ����
	void StartConnect();

private:
	ProxyService * _proxy_service;
	std::shared_ptr<TcpSocket> _socket;
	int32_t _session_id;
	SessionState _state;
	TimerHandle _connect_timer;
	std::list<std::shared_ptr<ProxyServiceMessage>> _msg_cache;
	bool _reconnect;
	std::string _remote_ip;
	uint16_t _remote_port;
};


// ����Ự
class AdminSession : public ServiceSession
{
public:

	AdminSession(int32_t id, ProxyService * proxy_service, const std::shared_ptr<TcpSocket> & sock)
		: ServiceSession(id, proxy_service, sock) {}

	virtual ~AdminSession() {}

	// ���յ�����
	// ����ʣ���������
	virtual int32_t OnReceived(char * data, int32_t len) override;

private:
	sframe::HttpRequestDecoder _http_decoder;
};

}

#endif
