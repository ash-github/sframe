
#ifndef SFRAME_CONFIG_SET_H
#define SFRAME_CONFIG_SET_H

#include <assert.h>
#include <string>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <utility>
#include <typeinfo>
#include "ConfigLoader.h"
#include "ConfigMeta.h"
#include "../util/FileHelper.h"
#include "../util/StringHelper.h"

namespace sframe {

// ���ü���
class ConfigSet
{
public:

	struct ConfUnit
	{
		virtual ~ConfUnit() {}
	};

	template<typename T>
	struct ConfUnitT : public ConfUnit
	{
		ConfUnitT()
		{
			val = std::make_shared<T>();
		}

		virtual ~ConfUnitT() {}

		std::shared_ptr<T> val;
	};

	typedef ConfigSet::ConfUnit * (ConfigSet::*Func_LoadConfig)(const std::string &, std::vector<std::string> *);

	typedef bool (ConfigSet::*Func_InitConfig)(ConfigSet::ConfUnit *);

	typedef const char *(*Func_GetConfigName)();

	struct ConfigLoadHelper 
	{
		int32_t conf_id;
		Func_LoadConfig func_load;
		Func_InitConfig func_init;
		std::string conf_file_name;
		std::string conf_type_name;
	};

	ConfigSet();

	virtual ~ConfigSet();

	// ����(ȫ���ɹ�����true, ֻҪ��һ��ʧ�ܶ��᷵��false)
	// ����ʱ��err_info�᷵�س����������Ϣ
	bool Load(const std::string & path, std::vector<std::string> * vec_err_msg = nullptr);


	///////////////////////// ��ѯ��ط��� ///////////////////////////

	// ��ȡ����ģ��
	template<typename T>
	std::shared_ptr<const T> GetConfigModule() const;

	// ��ȡ����
	template<typename T>
	std::shared_ptr<const typename CONFIG_MODEL_TYPE(T)> GetConfig() const;

	// ����key��ȡMap������Ŀ
	template<typename T>
	std::shared_ptr<const typename CONFIG_CONF_TYPE(T)> GetMapConfigItem(const typename CONFIG_KEY_TYPE(T) & key) const;

	// ע������
	// conf_file_name�������ļ���������ΪLoadʱ�������Ŀ¼·�������·����֧��ͨ����Ա�ʾ����ļ�
	template<typename T_ConfigLoader, typename T>
	void RegistConfigModule(const std::string & conf_file_name);

private:

	// ����
	template<typename T_ConfigLoader, typename T_Module>
	static bool LoadConfig(const std::string & conf_file_name, ConfUnitT<T_Module> * o);

	// ���أ������ļ���
	template<typename T_ConfigLoader, typename T>
	ConfigSet::ConfUnit * LoadConfig_OneFile(const std::string & conf_file_name, std::vector<std::string> * err_file_name);

	// ���أ����ļ���
	template<typename T_ConfigLoader, typename T>
	ConfigSet::ConfUnit * LoadConfig_MultiFile(const std::string & conf_file_name, std::vector<std::string> * err_file_name);

	// ��ʼ������
	template<typename T_Module>
	bool InitConfig(ConfigSet::ConfUnit * conf);

private:

	static const int kQuickFindArrLen = 128;

	ConfUnit * _quick_find_arr[kQuickFindArrLen];
	std::unordered_map<int32_t, ConfUnit *> _config;
	std::unordered_map<int32_t, ConfigLoadHelper> _config_load_helper;
	std::vector<ConfigLoadHelper> _temporary_config_load_helper;
	std::string _config_dir;
};


// ��ȡ����ģ��
template<typename T>
std::shared_ptr<const T> ConfigSet::GetConfigModule() const
{
	ConfUnit * conf_unit = nullptr;
	int32_t config_id = GET_CONFIGID(T);

	if (config_id >= 0 && config_id < kQuickFindArrLen)
	{
		conf_unit = _quick_find_arr[config_id];
	}
	else
	{
		auto it = _config.find(config_id);
		if (it != _config.end())
		{
			conf_unit = it->second;
		}
	}

	if (!conf_unit)
	{
		return nullptr;
	}

	ConfUnitT<T> * config_ele = dynamic_cast<ConfUnitT<T> *>(conf_unit);
	if (!config_ele)
	{
		assert(false);
		return nullptr;
	}

	return config_ele->val;
}

// ��ȡ����
template<typename T>
std::shared_ptr<const typename CONFIG_MODEL_TYPE(T)> ConfigSet::GetConfig() const
{
	int32_t config_id = GET_CONFIGID(T);
	std::shared_ptr<const T> conf_module = GetConfigModule<T>();
	if (!conf_module)
	{
		return nullptr;
	}

	return conf_module->Obj();
}

// ����key��ȡMap������Ŀ
template<typename T>
std::shared_ptr<const typename CONFIG_CONF_TYPE(T)> ConfigSet::GetMapConfigItem(const typename CONFIG_KEY_TYPE(T) & key) const
{
	std::shared_ptr<const T> map_conf = GetConfigModule<T>();
	if (!map_conf)
	{
		return nullptr;
	}

	return map_conf->GetConfigItem(key);
}

// ע������
// conf_file_name�������ļ���������ΪLoadʱ�������Ŀ¼·�������·����֧��ͨ����Ա�ʾ����ļ�
template<typename T_ConfigLoader, typename T>
void ConfigSet::RegistConfigModule(const std::string & conf_file_name)
{
	static_assert(std::is_base_of<ConfigModule, T>::value, "T must derive from ConfigModule");

	if (conf_file_name.empty())
	{
		assert(false);
		return;
	}

	int32_t config_id = GET_CONFIGID(T);
	ConfigLoadHelper load_helper;
	load_helper.conf_id = config_id;
	load_helper.func_init = &ConfigSet::InitConfig<T>;
	load_helper.conf_file_name = conf_file_name;
	load_helper.conf_type_name = ReadTypeName(typeid(T).name());

	// ���ļ������Ƿ���ͨ���(*)�����еĻ������ö��ļ����ط�ʽ
	if (load_helper.conf_file_name.find('*') == std::string::npos)
	{
		load_helper.func_load = &ConfigSet::LoadConfig_OneFile<T_ConfigLoader, T>;
	}
	else
	{
		load_helper.func_load = &ConfigSet::LoadConfig_MultiFile<T_ConfigLoader, T>;
	}

	// ����
	if (config_id < 0)
	{
		_temporary_config_load_helper.push_back(load_helper);
	}
	else
	{
		assert(_config_load_helper.find(config_id) == _config_load_helper.end());
		_config_load_helper[config_id] = load_helper;
	}
}

// ����
template<typename T_ConfigLoader, typename T_Module>
bool ConfigSet::LoadConfig(const std::string & conf_file_name, ConfUnitT<T_Module> * o)
{
	if (conf_file_name.empty() || !o)
	{
		assert(false);
		return false;
	}

	return T_ConfigLoader::Load(conf_file_name, *(o->val->Obj().get()));
}

// ���أ������ļ���
template<typename T_ConfigLoader, typename T>
ConfigSet::ConfUnit * ConfigSet::LoadConfig_OneFile(const std::string & conf_file_name, std::vector<std::string> * err_file_name)
{
	ConfUnitT<T> * o = new ConfUnitT<T>();
	std::string file_full_name = _config_dir + conf_file_name;

	if (!LoadConfig<T_ConfigLoader, T>(file_full_name, o))
	{
		if (err_file_name)
		{
			err_file_name->push_back(std::move(file_full_name));
		}
		delete o;
		return nullptr;
	}

	return o;
}

// ���أ����ļ���
template<typename T_ConfigLoader, typename T>
ConfigSet::ConfUnit * ConfigSet::LoadConfig_MultiFile(const std::string & conf_file_name, std::vector<std::string> * err_file_name)
{
	ConfUnitT<T> * o = new ConfUnitT<T>();
	std::vector<std::string> vec_files = FileHelper::ExpandWildcard(conf_file_name, _config_dir);
	bool ret = true;
	for (std::string & file_name : vec_files)
	{
		if (!LoadConfig<T_ConfigLoader, T>(file_name, o))
		{
			if (err_file_name)
			{
				err_file_name->push_back(std::move(file_name));
			}
			ret = false;
		}
	}

	if (!ret)
	{
		delete o;
		return nullptr;
	}

	return o;
}

// ��ʼ������
template<typename T_Module>
bool ConfigSet::InitConfig(ConfigSet::ConfUnit * conf)
{
	ConfUnitT<T_Module> * config_ele = dynamic_cast<ConfUnitT<T_Module>*>(conf);
	if (!config_ele)
	{
		assert(false);
		return false;
	}

	return ConfigInitializer::Initialize<T_Module, ConfigSet>((*(config_ele->val.get())), *this);
}

}

#endif
