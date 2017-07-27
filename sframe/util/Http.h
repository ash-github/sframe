
#ifndef SFRAME_HTTP_H
#define SFRAME_HTTP_H

#include <assert.h>
#include <inttypes.h>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <unordered_map>

namespace sframe {

enum HttpType
{
	kHttpType_Request = 1,      // Http����
	kHttpType_Response = 2,     // Http��Ӧ
};

class Http
{
public:

	typedef std::unordered_map<std::string, std::string> Param;

	typedef std::unordered_map<std::string, std::vector<std::string>> Header;

	// ��׼��ͷ������Key
	static std::string StandardizeHeaderKey(const std::string & key);

	// URL����
	static std::string UrlEncode(const std::string & str);

	// URL����
	static std::string UrlDecode(const std::string & str);

	// ����HTTP����
	static Http::Param ParseHttpParam(const std::string para_str);

	// HttpParamת��Ϊstring
	static std::string HttpParamToString(const Http::Param & para);



	Http() {}

	virtual ~Http() {}

	virtual HttpType GetHttpType() const = 0;

	const std::string & GetHeader(const std::string & key) const;

	const std::vector<std::string> & GetHeaders(const std::string & key) const;

	void SetHeader(const std::string & key, const std::string &value);

	const std::string & GetContent() const;

	void SetContent(const std::string & data);

	void SetContent(std::string && data);

	std::string ToString() const;

private:

	virtual void WriteFirstLine(std::ostringstream & oss) const = 0;

protected:
	Header _header;
	std::string _content;
};

class HttpRequest : public Http
{
public:

	HttpRequest();

	~HttpRequest() {}

	HttpType GetHttpType() const
	{
		return kHttpType_Request;
	}

	void SetMethod(const std::string & method);

	const std::string & GetMethod() const;

	void SetRequestUrl(const std::string & req_url);

	const std::string & GetRequestUrl() const;

	void SetRequestParam(const std::string & req_param);

	const std::string & GetRequestParam() const;

	void SetProtoVersion(const std::string & proto_version);

	const std::string & GetProtoVersion() const;

private:

	void WriteFirstLine(std::ostringstream & oss) const override;

private:
	std::string _method;
	std::string _req_url;
	std::string _req_param;
	std::string _proto_ver;
};

class HttpResponse : public Http
{
public:

	HttpResponse();

	~HttpResponse() {}

	HttpType GetHttpType() const
	{
		return kHttpType_Response;
	}

	void SetProtoVersion(const std::string & proto_version);

	const std::string & GetProtoVersion() const;

	void SetStatusCode(int32_t status_code);

	int32_t GetStatusCode() const;

	void SetStatusDesc(const std::string & status_desc);

	const std::string & GetStatusDesc() const;

private:

	void WriteFirstLine(std::ostringstream & oss) const override;

private:
	std::string _proto_ver;
	int32_t _status_code;
	std::string _status_desc;
};


class HttpDecoder
{
public:

	enum DecodeState
	{
		kDecodeState_FirstLine = 0,       // ���ڵ�һ��
		kDecodeState_HttpHeader = 1,      // ���ڽ���ͷ������
		kDecodeState_Content = 2,         // ���ڽ������ݲ���
		kDecodeState_Completed = 3,       // �������
	};

	void Reset();

	bool IsDecodeCompleted() const
	{
		return _state == kDecodeState_Completed;
	}

	// ����
	// ���ؽ����˵���Ч�������ݵĳ���
	size_t Decode(const std::string & data, std::string & err_msg)
	{
		return Decode(data.data(), data.length(), err_msg);
	}

	// ����
	// ���ؽ����˵���Ч�������ݵĳ���
	size_t Decode(const char * data, size_t len, std::string & err_msg);

protected:

	HttpDecoder(int32_t http_type);

	virtual ~HttpDecoder() {}

	std::shared_ptr<HttpRequest> GetHttpRequest() const
	{
		return _http_request;
	}

	std::shared_ptr<HttpResponse> GetHttpResponse() const
	{
		return _http_response;
	}

private:
	size_t DecodeFirstLine(const char * data, size_t len, std::string & err_msg);

	size_t DecodeHttpHeader(const char * data, size_t len, std::string & err_msg);

	size_t DecodeContent(const char * data, size_t len, std::string & err_msg);

	const std::string & GetHeader(const std::string & k);

private:
	int32_t _http_type;
	std::shared_ptr<HttpRequest> _http_request;
	std::shared_ptr<HttpResponse> _http_response;
	int32_t _state;
	int32_t _remain_content_len;      // -1.chunked��ȷ������; -2.����֪�����ӹرղŶ���; >0.����
	std::vector<std::string> _data_list;
};

class HttpRequestDecoder : public HttpDecoder
{
public:

	HttpRequestDecoder() : HttpDecoder(kHttpType_Request) {}

	~HttpRequestDecoder() {}

	std::shared_ptr<HttpRequest> GetResult() const
	{
		return GetHttpRequest();
	}
};

class HttpResponseDecoder : public HttpDecoder
{
public:

	HttpResponseDecoder() : HttpDecoder(kHttpType_Response) {}

	~HttpResponseDecoder() {}

	std::shared_ptr<HttpResponse> GetResult() const
	{
		return GetHttpResponse();
	}
};

}

#endif
