
#ifndef __FILE_HELPER_H__
#define __FILE_HELPER_H__

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
};

}

#endif
