#ifndef WEBVIEW_H
#define WEBVIEW_H

#include "fishtank-server.h"

#define DEFAULT_NAME "Fishtank WebView"

#define HTTP_STATUS_OK 200
#define HTTP_STATUS_NOT_FOUND 404
#define HTTP_STATUS_SEE_OTHER 303
#define HTTP_STATUS_INTERNAL_ERROR 500

class WebView{
public:
	WebView(unsigned short, Match&);
	WebView(const WebView&) = delete;

	void operator=(const WebView&) = delete;
	operator bool() const;

	void serve();
private:
	void process(net::tcp&);
	void index(net::tcp&);
	void kick(net::tcp&);
	void not_found(net::tcp&);

	static void respond(net::tcp&, const std::string&, int = HTTP_STATUS_OK);
	static void redirect(net::tcp&, const std::string&, const std::string&);
	static std::string get_http_request(net::tcp&);
	static std::string get_target_resource(const std::string&);
	static std::string construct_response_header(int, long long);
	static std::string construct_redirect_header(int, long long, const std::string&);
	static std::string get_status_code(int);
	static void check_http_request(const std::string&);
	static std::string html_wrap(const std::string&, const std::string&);

	net::tcp_server tcp;
	Match &match;
};

#endif // WEBVIEW_H
