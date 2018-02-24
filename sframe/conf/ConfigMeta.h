
#ifndef SFRAME_CONFIG_META_H
#define SFRAME_CONFIG_META_H

#include <inttypes.h>
#include <vector>
#include <set>
#include <map>

namespace sframe {

class ConfigModule
{
public:

	ConfigModule() {}

	virtual ~ConfigModule() {}

};

// ����ģ�����
template<typename T_ConfigModel, int Conf_Id>
class ConfigModuleT : public ConfigModule
{
public:

	typedef T_ConfigModel ModelType;

	static int GetConfigId()
	{
		return Conf_Id;
	}

	ConfigModuleT()
	{
		_conf_obj = std::make_shared<ModelType>();
	}

	const std::shared_ptr<ModelType> & Obj() const
	{
		return _conf_obj;
	}

private:
	std::shared_ptr<ModelType> _conf_obj;
};

// ��һ��������ģ��
template<typename T_Config, int Conf_Id>
class ObjectConfigModule : public ConfigModuleT<T_Config, Conf_Id>
{
public:
	typedef T_Config ConfType;
};

// vector����ģ��
template<typename T_Config, int Conf_Id>
class VectorConfigModule : public ConfigModuleT<std::vector<std::shared_ptr<T_Config>>, Conf_Id>
{
public:
	typedef T_Config ConfType;
};

// set����ģ��
template<typename T_Config, int Conf_Id>
class SetConfigModule : public ConfigModuleT<std::set<std::shared_ptr<T_Config>>, Conf_Id>
{
public:
	typedef T_Config ConfType;
};

// map����ģ��
template<typename T_Key, typename T_Config, int Conf_Id>
class MapConfigModule : public ConfigModuleT<std::map<T_Key, std::shared_ptr<T_Config>>, Conf_Id>
{
public:

	typedef T_Key KeyType;

	typedef T_Config ConfType;

	std::shared_ptr<ConfType> GetConfigItemNotConst(const KeyType & k) const
	{
		auto it = this->Obj()->find(k);
		return (it == this->Obj()->end() ? nullptr : it->second);
	}

	std::shared_ptr<const T_Config> GetConfigItem(const KeyType & k) const
	{
		auto it = this->Obj()->find(k);
		return (it == this->Obj()->end() ? nullptr : it->second);
	}

};


class GetConfigObjKey_Warpper
{
	template<typename R, typename T, R(T::*)() const>
	struct MethodMatcher;

	template<typename R, typename T>
	static std::true_type match(MethodMatcher<R, T, &T::GetKey>*) {}

	template<typename R, typename T>
	static std::false_type match(...) {}

	template<typename R, typename T>
	inline static R call(std::false_type, T & obj)
	{
		return R();
	}

	template<typename R, typename T>
	inline static R call(std::true_type, T & obj)
	{
		return obj.GetKey();
	}

public:
	template<typename R, typename T>
	inline static R GetKey(T & obj)
	{
		return call<R, T>(decltype(match<R, T>(nullptr))(), obj);
	}
};

template<typename R, typename T>
inline R GetConfigObjKey(T & obj)
{
	return GetConfigObjKey_Warpper::GetKey<R>(obj);
}

template<typename R, typename T>
inline R GetConfigObjKey(std::shared_ptr<T> & obj)
{
	return GetConfigObjKey_Warpper::GetKey<R>(*obj);
}

}


// ������һ����ģ�͵�����
// OBJ_CONFIG_MODULE(����ģ����, ���ýṹ����, ����ID)
#define OBJ_CONFIG_MODULE(module, conf, conf_id) class module : public sframe::ObjectConfigModule<conf, conf_id> {};

// ����Vetorģ�͵�����
// VECTOR_CONFIG_MODULE(����ģ����, �ṹ����, ����ID)
#define VECTOR_CONFIG_MODULE(module, conf, conf_id) class module : public sframe::VectorConfigModule<conf, conf_id> {};

// ����Setģ�͵�����
// SET_CONFIG_MODULE(����ģ����, �ṹ����, ����ID)
#define SET_CONFIG_MODULE(module, conf, conf_id) class module : public sframe::SetConfigModule<conf, conf_id> {};

// ����Mapģ�͵�����
// MAP_CONFIG_MODULE(����ģ����, key����, �ṹ����, �ṹ��������key�ĳ�Ա������, ����ID)
#define MAP_CONFIG_MODULE(module, k, conf, conf_id) class module : public sframe::MapConfigModule<k, conf, (conf_id)> {};

// ΪMAP����������ָ����Ϊkey���ֶ�
#define KEY_FIELD(k_type, k_field) k_type GetKey() const {return k_field;}

// ��ȡ����key����
#define CONFIG_KEY_TYPE(module) module::KeyType

// ��ȡ����ģ������
#define CONFIG_MODEL_TYPE(module) module::ModelType

// ��ȡ��������
#define CONFIG_CONF_TYPE(module) module::ConfType

// ��ȡ����ID
#define GET_CONFIGID(module) module::GetConfigId()

#endif