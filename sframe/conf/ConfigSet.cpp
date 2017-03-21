
#include <memory.h>
#include <assert.h>
#include "ConfigSet.h"

using namespace sframe;


// ����(ȫ���ɹ�����true, ֻҪ��һ��ʧ�ܶ��᷵��false)
// ����ʱ��err_info�᷵�س����������Ϣ
bool ConfigSet::Load(const std::string & path, std::vector<ConfigError> * vec_err_info)
{
	if (path.empty() || !_config.empty())
	{
		return false;
	}

	_config_dir = path;

	std::map<int32_t, std::shared_ptr<ConfigBase>> map_conf;
	bool ret = true;

	for (auto & pr : _config_load_helper)
	{
		ConfigLoadHelper & load_helper = pr.second;
		auto conf = LoadAndInitConfig(load_helper, vec_err_info);
		if (!conf)
		{
			ret = false;
			continue;
		}

		_config[load_helper.conf_id] = conf;
	}

	for (auto & load_helper : _temporary_config_load_helper)
	{
		auto conf = LoadAndInitConfig(load_helper, vec_err_info);
		if (!conf)
		{
			ret = false;
			continue;
		}
	}

	return ret;
}

// ���ز���ʼ��һ������
std::shared_ptr<ConfigSet::ConfigBase> ConfigSet::LoadAndInitConfig(const ConfigLoadHelper & load_helper, std::vector<ConfigError> * err_info)
{
	// ����
	auto conf = (this->*load_helper.func_load)(load_helper.conf_file_name);
	if (!conf)
	{
		if (err_info)
		{
			ConfigError err;
			err.err_type = ConfigError::kLoadConfigError;
			err.config_type = load_helper.conf_id;
			err.config_file_name = load_helper.conf_file_name;
			err_info->push_back(err);
		}

		return NULL;
	}

	// ��ʼ��
	if (!(this->*load_helper.func_init)(conf))
	{
		if (err_info)
		{
			ConfigError err;
			err.err_type = ConfigError::kInitConfigError;
			err.config_type = load_helper.conf_id;
			err.config_file_name = load_helper.conf_file_name;
			err_info->push_back(err);
		}

		return NULL;
	}

	return conf;
}