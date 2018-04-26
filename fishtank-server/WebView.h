#ifndef WEBVIEW_H
#define WEBVIEW_H

#undef distance
#include <unordered_map>

#include "fishtank-server.h"

#define DEFAULT_NAME "Fishtank WebView"

#define HTTP_STATUS_OK 200
#define HTTP_STATUS_NOT_FOUND 404
#define HTTP_STATUS_SEE_OTHER 303
#define HTTP_STATUS_FORBIDDEN 403
#define HTTP_STATUS_INTERNAL_ERROR 500

#define ROUTE_ADD(fn, q, m) \
static WebViewResult fn(const std::string&, Match&); \
static const bool private_route_variable_ ## fn = WebView::add_route(#fn, fn); \
static WebViewResult fn(const std::string &q, Match &m)

class ShutdownException : public std::exception{
public:
	ShutdownException(const std::string &c)
		: content(c) {}
	virtual ~ShutdownException() noexcept {}

	virtual const char *what() const noexcept{
		return content.c_str();
	}

private:
	const std::string content;
};

class RestartException : public std::exception{
public:
	RestartException(const std::string &c)
		: content(c) {}
	virtual ~RestartException() noexcept {}

	virtual const char *what() const noexcept {
		return content.c_str();
	}

private:
	const std::string content;
};

struct WebViewResult
{
	WebViewResult(const std::string &content, int rc = HTTP_STATUS_OK, const std::string &redir = "")
		: html(content)
		, redirect_to(redir)
		, code(rc)
	{}

	const std::string html;
	const std::string redirect_to;
	const int code;
};

class WebView{
public:
	typedef WebViewResult (*ROUTEFN)(const std::string&, Match&);

	WebView(unsigned short, Match&, const std::string&);
	WebView(const WebView&) = delete;

	void operator=(const WebView&) = delete;
	operator bool() const;

	void serve();

	static bool add_route(const std::string&, ROUTEFN);
	static void url_decode(std::string&);
	static void html_entities(std::string&);

private:
	void router(net::tcp&);
	bool authorized(const std::string&)const;

	static void dispatch(ROUTEFN, const std::string&, Match&, net::tcp &sock);
	static ROUTEFN get_controller(const std::string&);
	static void respond(net::tcp&, const std::string&, int);
	static void redirect(net::tcp&, const std::string&, const std::string&);
	static std::string get_http_request(net::tcp&);
	static std::string get_target_resource(const std::string&);
	static std::string get_page(const std::string&);
	static std::string get_args(const std::string&);
	static std::string construct_response_header(int, long long);
	static std::string construct_redirect_header(int, long long, const std::string&);
	static std::string get_status_code(int);
	static void check_http_request(const std::string&);

	net::tcp_server tcp;
	Match &match;
	const std::string &allowed;

	static std::unordered_map<std::string, ROUTEFN> routes;
};

#endif // WEBVIEW_H
