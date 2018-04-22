#include <stdexcept>
#include <chrono>
#include <iostream>

#include <time.h>
#include <ctype.h>

#include "WebView.h"

std::unordered_map<std::string, WebView::ROUTEFN> WebView::routes;

WebView::WebView(unsigned short port, Match &m, const std::string &a)
	: tcp(port)
	, match(m)
	, allowed(a)
{}

WebView::operator bool()const{
	return tcp;
}

void WebView::serve(){
	const int sock = tcp.accept();
	if(sock == -1)
		return;

	net::tcp connection(sock);

	try{
		const auto start = std::chrono::high_resolution_clock::now();
		router(connection);
		const auto end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<float, std::ratio<1, 1000>> diff = end - start;
		if(diff.count() >= 1.5f)
			std::cout << "WebView: took " << diff.count() << " milliseconds to serve" << std::endl;
	}
	catch(const std::runtime_error &e)
	{
		std::cout << "@@@@@@@@@@@@@@ WebView Exception: " << e.what() << std::endl;
	}
	catch(const ShutdownException &e)
	{
		respond(connection, e.what(), HTTP_STATUS_OK);
		throw;
	}
}

// add a route to the routes map
bool WebView::add_route(const std::string &name, ROUTEFN fn){
	WebView::routes.insert(std::pair<std::string, ROUTEFN>(name, fn));

	return true;
}

// decode the "%20" related stuff from url
void WebView::url_decode(std::string &text){
	// replace + with space
	for(char &c : text)
		if(c == '+')
			c = ' ';

	// percent decode
	for(unsigned i = 0; i < text.length(); ++i){
		const char c = text.at(i);

		if(c == '%'){
			// get the 2 hex chars
			const std::string hex = text.substr(i + 1, 2);
			unsigned int decoded = '?';
			sscanf(hex.c_str(), "%x", &decoded);

			// remove and replace
			text.erase(i, 3);
			text.insert(text.begin() + i, (char)decoded);
		}
	}
}

// needs to be run after WebView::url_decode(...)
void WebView::html_entities(std::string &text){
	for(unsigned i = 0; i < text.length(); ++i){
		const char c = text.at(i);

		if(c == '<'){
			text.erase(text.begin() + i);
			text.insert(i, "&lt;");
		}
		else if(c == '>'){
			text.erase(text.begin() + i);
			text.insert(i, "&gt;");
		}
	}
}

// dispatch get request to proper controller
void WebView::router(net::tcp &sock){
	// handle people who aren't localhost
	const std::string &name = sock.get_name();
	if(!authorized(name)){
		WebView::dispatch(WebView::get_controller("http_forbidden"), "", match, sock);
		return;
	}

	const std::string request = WebView::get_http_request(sock);
	const std::string resource = WebView::get_target_resource(request);
	const std::string target = WebView::get_page(resource);
	const std::string args = WebView::get_args(resource);
	ROUTEFN controller = WebView::get_controller(target);

	WebView::dispatch(controller, args, match, sock);
}

// determine if the ip address is authorized to access
bool WebView::authorized(const std::string &ipaddr)const{
	if(allowed.find(ipaddr) != std::string::npos || (ipaddr.find("::ffff:") == 0 && allowed.find(ipaddr.substr(7)) != std::string::npos))
		return true;

	return ipaddr == "::1" || ipaddr == "::ffff:127.0.0.1" || ipaddr == "127.0.0.1";
}

// dispatch
void WebView::dispatch(ROUTEFN controller, const std::string &args, Match &m, net::tcp &sock){
	const WebViewResult result = controller(args, m);

	if(result.code == HTTP_STATUS_SEE_OTHER)
		WebView::redirect(sock, result.html, result.redirect_to);
	else
		WebView::respond(sock, result.html, result.code);
}

WebView::ROUTEFN WebView::get_controller(const std::string &name){
	auto cit = WebView::routes.find(name);

	if(cit == WebView::routes.end()){
		cit = WebView::routes.find("http_not_found");

		if(cit == WebView::routes.end()){
			cit = WebView::routes.find("http_internal_server_error");

			if(cit == WebView::routes.end())
				throw std::runtime_error("no 500 controller");
		}
	}

	return cit->second;
}

// send http response
void WebView::respond(net::tcp &sock, const std::string &response, int code){
	const int length = response.length();
	const std::string sum = WebView::construct_response_header(code, length) + response;

	sock.send_block(sum.c_str(), sum.length());
}

// send http response with location header (for redirecting the browser)
void WebView::redirect(net::tcp &sock, const std::string &response, const std::string &location){
	const int length = response.length();
	const std::string sum = WebView::construct_redirect_header(HTTP_STATUS_SEE_OTHER, length, location) + response;

	sock.send_block(sum.c_str(), sum.length());
}

// get http request from client
std::string WebView::get_http_request(net::tcp &sock){
	bool end = false;
	const int get_size = 128; // try to recv how many characters at a time
	std::string req; // request

	const int start_time = time(NULL);
	while(!end){
		if(time(NULL) - start_time > 1)
			throw std::runtime_error("no http request");

		// receive a bit of the HTTP request
		char block[get_size + 1];
		const int received = sock.recv_nonblock(block, get_size);

		// check for socket error
		if(sock.error())
			throw std::runtime_error("closed");

		block[received] = 0;
		req += block;

		// end of http request is denoted by CRLFCRLF
		end = req.find("\r\n\r\n") != std::string::npos;
	}

	// check request for validity
	check_http_request(req);

	return req;
}

// check http request for validity
void WebView::check_http_request(const std::string &request){
	const int len = request.length();

	// make sure there's enough room at least for "GET "
	if(len < 4)
		throw std::runtime_error("malformed");

	// servant only supports GET request
	if(!(request[0] == 'G' && request[1] == 'E' && request[2] == 'T' && request[3] == ' '))
		throw std::runtime_error("unsupported request");

	if(len < 14) // shortest possible request: GET / HTTP/1.1
		throw std::runtime_error("malformed");

	// check http version
	const unsigned pos = request.find("\r\n");
	if(pos == std::string::npos || pos<3)
		throw std::runtime_error("malformed");

	if(request[pos - 3] != '1') // 3 back from the newline is the major http version
		throw std::runtime_error("bad version");
}

// pick out the requested resource from <header> and put it in <target>
std::string WebView::get_target_resource(const std::string &header){
	std::string target;

	// find the first space in the header
	unsigned beginning = header.find(" ");
	if(beginning == std::string::npos)
		return target;
	++beginning; // get past the space

	// consistency check
	if(beginning >= header.size() - 1)
		return target;

	// find the second space in the header
	const unsigned end = header.find(" ", beginning);
	if(end == std::string::npos)
		return target;

	// get the bit in the middle
	target = header.substr(beginning, end - beginning);

	// to lower
	for(char &c : target)
		c = tolower(c);

	// strip the leading slash (if any)
	while(target.length() > 0 && target[0] == '/')
		target.erase(target.begin());

	return target;
}

// get the page from the request
// e.g. "http://localhost:20501/index" will return "index"
// "http://localhost:20501/say?m=hello+there" will return "say"
std::string WebView::get_page(const std::string &request){
	if(request == "")
		return "index";

	const auto question_pos = request.find("?");
	if(question_pos != std::string::npos)
		return request.substr(0, question_pos);

	const auto slash_pos = request.find("/");
	if(slash_pos != std::string::npos)
		return request.substr(0, slash_pos);

	return request;
}

// get the arguments bit from the requested resource
// e.g. "http://localhost:50201/index/say" will extract "say"
// "http://localhost:20501/index?arg=hello+there&arg2=more+words" will extract "?arg=hello+there&arg2=more+words"
std::string WebView::get_args(const std::string &request){
	const auto question_pos = request.find("?");
	if(question_pos != std::string::npos)
		return request.substr(question_pos);

	const auto slash_pos = request.find("/");

	return slash_pos == std::string::npos ? "" : request.substr(slash_pos + 1);
}

std::string WebView::construct_response_header(int code, long long content_length){
	char length_string[25];
	sprintf(length_string, "%lld" ,content_length);

	const std::string status = get_status_code(code);
	std::string header;

	header = "HTTP/1.1 " + status + "\r\n" +
	"Cache-Control: no-cache, no-store, must-revalidate" + "\r\n" +
	"Content-Length: " + length_string + "\r\n" +
	"Content-Type: text/html\r\n" +
	"Server: " + DEFAULT_NAME + "\r\n\r\n";

	return header;
}

std::string WebView::construct_redirect_header(int code, long long content_length, const std::string &location){
	std::string regular = WebView::construct_response_header(code, content_length);

	const auto pos = regular.find("Server:");
	if(pos == std::string::npos)
		throw std::runtime_error("could not find server header");

	regular.insert(pos, "Location: " + location + "\r\n");

	return regular;
}

// fill in the status code (e.g. "200 OK")
std::string WebView::get_status_code(int code){
	std::string status;

	switch(code){
	case HTTP_STATUS_OK:
		status = "200 OK";
		break;
	case HTTP_STATUS_NOT_FOUND:
		status = "404 Not Found";
		break;
	case HTTP_STATUS_SEE_OTHER:
		status = "302 See Other";
		break;
	case HTTP_STATUS_FORBIDDEN:
		status = "403 Forbidden";
		break;
	default:
		status = "500 Internal Server Error";
		break;
	}

	return status;
}
