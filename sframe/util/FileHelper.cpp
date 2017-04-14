
#include <stdio.h>
#include "FileHelper.h"

#ifndef __GNUC__
#include <shlwapi.h>
#pragma comment(lib, "shlwapi")
#pragma warning(disable:4996)
#else
#include <sys/types.h> 
#include <sys/stat.h>
#include <dirent.h>
#endif

using namespace sframe;

// ��ȡ�ļ���������
bool FileHelper::ReadFile(const std::string & full_name, std::string & content)
{
	FILE * f = fopen(full_name.c_str(), "r");
	if (!f)
	{
		return false;
	}

	if (fseek(f, 0L, SEEK_END) < 0)
	{
		fclose(f);
		return false;
	}

	long file_size = ftell(f);
	if (file_size <= 0)
	{
		fclose(f);
		return false;
	}

	std::string temp(file_size, '\0');
	char * p = &temp[0];

	if (fseek(f, 0L, SEEK_SET) < 0)
	{
		fclose(f);
		return false;
	}

	size_t s = fread(p, sizeof(char), file_size, f);
	if (s < (size_t)file_size)
	{
		temp.erase(temp.begin() + s, temp.end());
	}
	fclose(f);

	content = std::move(temp);

	return true;
}

// д���ļ�
size_t FileHelper::WriteFile(const std::string & full_name, std::string & content)
{
	FILE * f = fopen(full_name.c_str(), "w");
	if (!f)
	{
		return 0;
	}

	size_t s = fwrite(content.c_str(), sizeof(char), content.length(), f);
	fclose(f);

	return s;
}

// ��ȫ·���л�ȡ�ļ���
std::string FileHelper::GetFileName(const char * fullname)
{
	const char * file = fullname;
	const char * p = file;
	while (*p != '\0')
	{
		if (*p == '\\' || *p == '/')
		{
			file = p + 1;
		}

		p++;
	}

	return std::string(file);
}

// ȥ���ļ���չ��
std::string FileHelper::RemoveExtension(const std::string & name)
{
	size_t pos = name.find_last_of('.');
	return (pos == std::string::npos ? name : name.substr(0, pos));
}

// Ŀ¼�Ƿ����
bool FileHelper::DirectoryExisted(const std::string & path)
{
	bool result = false;

#ifndef __GNUC__
	result = PathIsDirectoryA(path.c_str()) ? true : false;
#else
	DIR * pdir = opendir(path.c_str());
	if (pdir != nullptr)
	{
		closedir(pdir);
		result = true;
	}
#endif

	return result;
}

// ����Ŀ¼
bool FileHelper::MakeDirectory(const std::string & path)
{
#ifndef __GNUC__
	return CreateDirectoryA(path.c_str(), nullptr) ? true : false;
#else
	return (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0);
#endif
}

// ȷ��·������
bool FileHelper::MakeDirectoryRecursive(const std::string & path)
{
	if (DirectoryExisted(path))
	{
		return true;
	}

	std::string::size_type pos = path.find('/');

	while (pos != std::string::npos)
	{
		std::string cur = path.substr(0, pos - 0);

		if (cur.length() > 0 && !DirectoryExisted(cur))
		{
			if (!MakeDirectory(cur))
			{
				return false;
			}
		}

		pos = path.find('/', pos + 1);
	}

	return true;
}