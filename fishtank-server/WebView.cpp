#include <stdexcept>

#include <time.h>
#include <ctype.h>

#include "WebView.h"

WebView::WebView(unsigned short port, Match &m)
	: tcp(port)
	, match(m)
{}

WebView::operator bool()const{
	return tcp;
}

void WebView::serve(){
	const int sock = tcp.accept();
	if(sock == -1)
		return;

	net::tcp connection(sock);
	const std::string &name = connection.get_name();
	if(name != "::ffff:127.0.0.1" && name != "::1" && name != "127.0.0.1")
		return;

	try{
		process(connection);
	}
	catch(const std::exception &e)
	{
		std::cout << "@@@@@@@@@@@@@@ WebView Exception: " << e.what() << std::endl;
	}
}

void WebView::process(net::tcp &sock){
	const std::string request = WebView::get_http_request(sock);
	const std::string resource = WebView::get_target_resource(request);

	if(resource == "/"){
		index(sock);
	}
	else if(resource.find("/kick/") == 0 && resource.size() > 6){
		int id = 0;
		if(sscanf(resource.c_str() + 6, "%d", &id) == 1){
			match.kick(id);
			kick(sock);
		}
		else
			not_found(sock);
	}
	else{
		not_found(sock);
	}
}

void WebView::index(net::tcp &sock){
	const std::vector<ShortClient> summary = match.client_summary();
	const std::string style = "\"padding: 10px 20px;border: 1px solid black;\"";

	std::string content;
	if(summary.size() > 0){
		content = "<table style=\"border: 1px solid black;border-collapse: collapse;\">"
		"<tr style=" + style + "><th>Id</th>"
		"<th style=" + style + ">Name</th>"
		"<th style=" + style + ">Play Time</th>"
		"<th style=" + style + ">Match Victories</th>"
		"<th style=" + style + ">Kills</th>"
		"<th style=" + style + ">Deaths</th>"
		"<th style=" + style + ">Rounds Played</th>"
		"<th style=" + style + ">Action</th></tr>";

		for(const ShortClient &client : summary){
			const std::string id_string = std::to_string(client.id);
			const std::string play_time = WebView::format(time(NULL) - client.play_time);

			content +=
			"<tr>"
			"<td style=" + style + ">" + id_string + "</td>"
			"<td style=" + style + ">" + client.name + "</td>"
			"<td style=" + style + ">" + play_time + "</td>"
			"<td style=" + style + ">" + std::to_string(client.match_victories) + "</td>"
			"<td style=" + style + ">" + std::to_string(client.victories) + "</td>"
			"<td style=" + style + ">" + std::to_string(client.deaths) + "</td>"
			"<td style=" + style + ">" + std::to_string(client.rounds_played) + "</td>"
			"<td style=" + style + "><a href=\"/kick/" + id_string + "\"><button class=\"button\">Kick</button></a></td>"
			"</tr>";
		}

		content += "</table>";
	}
	else{
		content = "No Clients are connected.";
	}

	WebView::respond(sock, WebView::html_wrap("Client Index", content));
}

void WebView::kick(net::tcp &sock){
	const std::string content = WebView::html_wrap("moved", "see other");

	WebView::redirect(sock, content, "/");
}

void WebView::not_found(net::tcp &sock){
	const std::string content = WebView::html_wrap("404 - Page Not Found", "The requested resource was not understood.");

	WebView::respond(sock, content, HTTP_STATUS_NOT_FOUND);
}

void WebView::respond(net::tcp &sock, const std::string &response, int code){
	const int length = response.length();
	const std::string sum = WebView::construct_response_header(code, length) + response;

	sock.send_block(sum.c_str(), sum.length());
}

void WebView::redirect(net::tcp &sock, const std::string &response, const std::string &location){
	const int length = response.length();
	const std::string sum = WebView::construct_redirect_header(HTTP_STATUS_SEE_OTHER, length, location) + response;

	sock.send_block(sum.c_str(), sum.length());
}

std::string WebView::get_http_request(net::tcp &sock){
	bool end = false;
	const int get_size = 128; // try to recv how many characters at a time
	std::string req; // request

	while(!end){
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

	return target;
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
	default:
		status = "500 Internal Server Error";
		break;
	}

	return status;
}

std::string WebView::html_wrap(const std::string &page_title, const std::string &body){
	return
	"<!Doctype html>"
	"<html>"
	"<head><title>" + page_title + "</title></head>"
	"<body>"
	+ body +
	"</body>"
	"</html>"
	;
}

std::string WebView::format(int played){
	int minutes = played / 60;
	int seconds = played % 60;

	char fmt[21];
	snprintf(fmt, sizeof(fmt), "%02d:%02d", minutes, seconds);

	return fmt;
}
