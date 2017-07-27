
#ifndef __FILE_HELPER_H__
#define __FILE_HELPER_H__

#include <vector>
#include <string>

namespace sframe {

class FileHelper
{
public:

	// ��ȡ�ļ���������
	static bool ReadFile(const std::string & full_name, std::string & content);

	// д���ļ�
	static size_t WriteFile(const std::string & full_name, std::string & content);

	// ��ȫ·���л�ȡ�ļ���
	static std::string GetFileName(const char * fullname);

	// ��ȫ·���л�ȡ�ļ���
	static std::string GetFileName(const std::string & fullname)
	{
		return GetFileName(fullname.c_str());
	}

	// ȥ���ļ���չ��
	static std::string RemoveExtension(const std::string & name);

	// Ŀ¼�Ƿ����
	static bool DirectoryExisted(const std::string & path);

	// ����Ŀ¼
	static bool MakeDirectory(const std::string & path);

	// �ݹ鴴��
	static bool MakeDirectoryRecursive(const std::string & path);

	enum ScanType
	{
		kScanType_All,
		kScanType_OnlyDirectory,
		kScanType_OnlyNotDirectory,
	};

	// ɨ��Ŀ¼
	// dir_path   :   Ŀ¼·�������ܰ���ͨ���
	// match_name :   ����ƥ�䣬֧��ͨ���������Ҫɨ�� /data Ŀ¼�£����з���*.cpp�����ֵ����ݣ�����ScanDirectory("/data", "*.cpp")
	static std::vector<std::string> ScanDirectory(const std::string & dir_path, const std::string & match_name = "", ScanType scan_type = kScanType_All);

	// չ��ͨ���(* ?)
	// path: ·��������Ƿ���/��β����ʾ�ļ�������ΪĿ¼
	// parent_dir: ����Ŀ¼
	static std::vector<std::string> ExpandWildcard(const std::string & path, const std::string & parent_dir = "");
};

}

#endif
