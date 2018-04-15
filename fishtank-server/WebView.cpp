#include <stdexcept>
#include <chrono>
#include <iostream>

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

	try{
		const auto start = std::chrono::high_resolution_clock::now();
		process(connection);
		const auto end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<float, std::ratio<1, 1000>> diff = end - start;
		if(diff.count() >= 1.0f)
			std::cout << "WebView: took " << diff.count() << " milliseconds to serve" << std::endl;
	}
	catch(const std::exception &e)
	{
		std::cout << "@@@@@@@@@@@@@@ WebView Exception: " << e.what() << std::endl;
	}
}

void WebView::process(net::tcp &sock){
	// handle people who aren't localhost
	const std::string &name = sock.get_name();
	if(name != "::ffff:127.0.0.1" && name != "::1" && name != "127.0.0.1"){
		forbidden(sock);
		return;
	}

	const std::string request = WebView::get_http_request(sock);
	const std::string resource = WebView::get_target_resource(request);

	if(resource == "/"){
		index(sock);
	}
	else if(resource.find("/say") == 0){
		const auto pos = resource.find("?m=");
		if(pos != std::string::npos){
			std::string msg = resource.substr(pos + 3);
			WebView::escape_and_encode(msg);
			say(sock, msg);
		}
	}
	else if(resource.find("/kick") == 0 && resource.size() > 6){
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

	const std::string head =
	"<script type=\"text/javascript\">\n"
	"function timed_refresh(){\n"
		"\tsetTimeout(do_reload, 5000);\n"
	"}\n"
	"function do_reload(){\n"
		"\tvar elm = document.getElementById('saymessage');\n"
		"\tvar say = elm == undefined ? \"\" : elm.value;\n"
		"\tif(say.trim().length === 0){\n"
			"\t\tlocation.reload(true);\n"
		"\t}\n"
		"\telse{\n"
			"\t\tsetTimeout(do_reload, 5000);\n"
		"\t}\n"
	"}\n"
	"window.onload=timed_refresh;\n"
	"</script>";

	// fill in the client table
	std::string content = "<a href=\"https://github.com/joshrwinter/fishtank\">source code</a><br>";
	if(summary.size() > 0){
		content += "<div style=\"float: left;\"><h2>Clients</h2><table style=\"border: 1px solid black;border-collapse: collapse;\">"
		"<tr style=" + style + "><th>Id</th>"
		"<th style=" + style + ">Name</th>"
		"<th style=" + style + ">Play Time</th>"
		"<th style=" + style + ">Match Victories</th>"
		"<th style=" + style + ">Kills</th>"
		"<th style=" + style + ">Deaths</th>"
		"<th style=" + style + ">Rounds Played</th>"
		"<th style=" + style + ">Action</th></tr>\n";

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
			"</tr>\n";
		}
		content += "</table></div><div style=\"float: left;padding-left: 30px;\">";

		// fill in the chats table
		const std::vector<ChatMessage> chats = match.chat_log();
		content += "\n<h2>Chat Log</h2>";
		content += "<form action=\"/say\" method=\"get\"><input type=\"text\" id=\"saymessage\" name=\"m\" autocomplete=\"off\" style=\"margin-right: 10px;\" autofocus><input type=\"submit\" value=\"Say\"></form><br>";
		for(const ChatMessage &cm : chats){
			content += std::string("<font color=") + (cm.from == "server" ? "\"red\"" : "\"blue\"") + ">" + cm.from + "</font>: " + cm.message + "<br>\n";
		}
		content += "</div>";
	}
	else{
		content += "No Clients are connected.";
	}

	WebView::respond(sock, WebView::html_wrap("Client Index", content, head));
}

void WebView::say(net::tcp &sock, const std::string &msg){
	match.send_chat(msg);
	const std::string content = WebView::html_wrap("moved", "see other");

	WebView::redirect(sock, content, "/");
}

void WebView::kick(net::tcp &sock){
	const std::string content = WebView::html_wrap("moved", "see other");

	WebView::redirect(sock, content, "/");
}

void WebView::forbidden(net::tcp &sock){
	const std::string content = WebView::html_wrap("Nope", "<h1>Forbidden</h1><br><p>nice try</p>");

	WebView::respond(sock, content, HTTP_STATUS_FORBIDDEN);
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
	case HTTP_STATUS_FORBIDDEN:
		status = "403 Forbidden";
		break;
	default:
		status = "500 Internal Server Error";
		break;
	}

	return status;
}

std::string WebView::html_wrap(const std::string &page_title, const std::string &body, const std::string &head){
	return
	"<!Doctype html>"
	"<html>"
	"<head><title>" + page_title + "</title>\n" + head + "\n</head>"
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

void WebView::escape_and_encode(std::string &text){
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
			if(decoded == '<')
				text.insert(i, "&lt;");
			else if(decoded == '>')
				text.insert(i, "&gt;");
			else
				text.insert(text.begin() + i, (char)decoded);
		}
	}
}
