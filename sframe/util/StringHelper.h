
#ifndef SFRAME_STRING_HELPER_H
#define SFRAME_STRING_HELPER_H

#include <string>
#include <vector>

namespace sframe {

// �ָ��ַ���
std::vector<std::string> SplitString(const std::string & str, const std::string & sep);

// �����ַ����ַ���������������ִ���
int32_t GetCharMaxContinueInString(const std::string & str, char c);

// �����Ӵ�
int32_t FindFirstSubstr(const char * str, int32_t len, const char * sub_str);

// ���ַ���ת��Ϊ��д
void UpperString(std::string & str);

// ���ַ���ת��ΪСд
void LowerString(std::string & str);

// ���ַ���ת��Ϊ��д
std::string ToUpper(const std::string & str);

// ���ַ���ת��ΪСд
std::string ToLower(const std::string & str);

// ȥ��ͷ���մ�
std::string TrimLeft(const std::string & str, char c = ' ');

// ȥ��β���մ�
std::string TrimRight(const std::string & str, char c = ' ');

// ȥ�����߿մ�
std::string Trim(const std::string & str, char c = ' ');

// wstring -> string
std::string WStrToStr(const std::wstring & src);

// string -> wstring
std::wstring StrToWStr(const std::string & src);

// utf8 -> wchar
size_t UTF8ToWChar(const char * str, size_t len, wchar_t * wc);

// wchar -> utf8
size_t WCharToUTF8(wchar_t wc, char * buf, size_t buf_size);

// �Ƿ�Ϸ���utf8
bool IsValidUTF8(const std::string & str);

// �Ƿ�Ϸ���utf8
bool IsValidUTF8(const char * str, size_t len);

// wstring -> utf8
std::string WStrToUTF8(const std::wstring & src);

// utf8 -> wstring
std::wstring UTF8ToWStr(const std::string & src);

// �Ƿ�ƥ��ͨ�����?��*��
// str     :   ����ͨ������ַ���
// match   :   ��ͨ������ַ���
bool MatchWildcardStr(const std::string & real_name, const std::string & wildcard_name, bool ignore_case = false);

// �����������ƣ�ת��Ϊ A::B::C ����ʽ��
std::string ReadTypeName(const char * name);

bool ParseCommandLine(const std::string & data, std::string & cmd_name, std::vector<std::string> & cmd_param);

}

#endif
