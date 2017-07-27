
#ifndef PUBDEF_SERVER_CONFIG_H
#define PUBDEF_SERVER_CONFIG_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <set>
#include "conf/ConfigDef.h"
#include "util/Singleton.h"

struct NetAddrInfo
{
	bool ParseFormString(const std::string & data);

	std::string ip;           // IP
	uint16_t port;            // �˿�
};

struct ListenAddrInfo
{
	bool ParseFormString(const std::string & data);

	std::string desc;         // ����������ʶ��
	NetAddrInfo addr;         // ��ַ
};

struct ServiceInfo
{
	bool ParseFormString(const std::string & data);

	int32_t sid;
	std::string service_type_name;   // ������������
	bool is_local_service;           // true Ϊ���ط���falseΪԶ�̷���
	NetAddrInfo remote_addr;         // Զ�̵�ַ������local_serviceΪfalse����Ч
};

struct ServerConfig : public sframe::singleton<ServerConfig>
{
	bool Load(const std::string & filename);

	void Fill(const json11::Json & reader);

	bool HaveLocalService(const std::string & serv_type_name);

	std::string server_name;                  // ����������
	std::string res_path;                     // ��ԴĿ¼
	int32_t thread_num;                       // �߳�����
	std::shared_ptr<NetAddrInfo> listen_service;                                 // Զ�̷��������ַ
	std::shared_ptr<NetAddrInfo> listen_admin;                                 // �����ַ
	std::unordered_map<int32_t, std::shared_ptr<ServiceInfo>> services;          // ������Ϣ��sid -> ������Ϣ��
	std::unordered_map<std::string, std::unordered_map<int32_t, std::shared_ptr<ServiceInfo>>> type_to_services;  // ����->���������з�����Ϣ
	std::unordered_map<std::string, std::vector<ListenAddrInfo>> listen_custom;  // �Զ������
};


#endif
