#ifndef WEB_VIEW_STATIC_RESOURCES_H
#define WEB_VIEW_STATIC_RESOURCES_H

#include <time.h>

static const std::string refresher_js =
"<script type=\"text/javascript\">\n"
"function timed_refresh(){\n"
"setTimeout(do_reload, 5000);\n"
"}\n"
"function do_reload(){\n"
"var elm = document.getElementById('saymessage');\n"
"var say = elm == undefined ? \"\" : elm.value;\n"
"if(say.trim().length === 0){\n"
"location.reload(true);\n"
"}\n"
"else{\n"
"setTimeout(do_reload, 5000);\n"
"}\n"
"}\n"
"window.onload=timed_refresh;\n"
"</script>\n"
;

static const std::string master_css =
"<style>\n"
"body { margin-left: 12%; margin-right: 12%; }\n"
"#topdiv { background-color: rgb(42, 157, 212); }\n"
"#titleheader { padding-left: 5px; display: inline; }\n"
"#navspan { margin: 0; padding: 0; float: right; }\n"
".titlebar { font-size: 20px; }\n"
"a.navlink, a:visited.navlink { height: 100%; padding-left: 16px; padding-right: 16px; text-decoration: none; background-color: rgb(42, 157, 212); color: black; }\n"
"a:hover.navlink { background-color: rgb(62, 177, 232); }\n"
"#table_noborder { padding-bottom: 10px; padding-right: 100px; }\n"
"#table_normal { padding: 10px; border: 1px solid black; }\n"
"#red { color: rgb(200, 0, 0); }\n"
"#red_bold { color: rgb(255, 0, 0); }\n"
"#green { color: rgb(0, 200, 0); }\n"
"#green_bold { color: rgb(0, 255, 0); }\n"
"#cyan { color: rgb(0, 200, 200); }\n"
"#cyan_bold { color: rgb(0, 255, 255); }\n"
"#gray { color: rgb(128, 128, 128); }\n"
"pre { font-size: 14px; }\n"
"</style>"
;

static const std::string body_pre =
"<div id=\"topdiv\">\n"
"<p id=\"titleheader\" class=\"titlebar\">FISHTANK</p>\n"
"<span id=\"navspan\" class=\"titlebar\">\n"
"<a class=\"navlink\" href=\"/\">Status</a>"
"<a class=\"navlink\" href=\"/configuration\">Configuration</a>\n"
"<a class=\"navlink\" href=\"/logs\">Logs</a>\n"
"<a class=\"navlink\" href=\"/help\">?</a>\n"
"</span>\n"
"</div>\n"
;

static std::string wrap(const std::string &body, const std::string &title, const std::string &head = ""){
	return
		"<!Doctype html>\n"
		"<html>\n"
		"<head>\n"
		"<title>" + title + "</title>\n"
		+ head + "\n"
		+ master_css +"\n"
		"</head>\n"
		"<body>\n"
		+ body_pre + "\n"
		+ body + "\n"
		"</body>\n"
		"</html>\n"
	;
}

static std::string format(int span){
	const int minutes = span / 60;
	const int seconds = span % 60;

	char fmt[25];
	snprintf(fmt, sizeof(fmt), "%02d:%02d", minutes, seconds);

	return fmt;
}

typedef std::string::size_type position;

struct sequence_position{
	sequence_position(position s, int c, const char *sp) : start(s), count(c), span(sp) {}
	sequence_position() : start(std::string::npos), count(0), span("") {}

	position start;
	int count;
	const char *span;
};

static const char *code_to_span(const std::string &code){
	if(code == "\033[31m")
		return "<span id=\"red\">";
	else if(code == "\033[31;1m")
		return "<span id=\"red_bold\">";

	else if(code == "\033[32m")
		return "<span id=\"green\">";
	else if(code == "\033[32;1m")
		return "<span id=\"green_bold\">";

	else if(code == "\033[36m")
		return "<span id=\"cyan\">";
	else if(code == "\033[36;1m")
		return "<span id=\"cyan_bold\">";

	else if(code == "\033[0m")
		return "</span>";

	return "<span>";
}

static sequence_position find_next_sequence(const std::string &line, int start){
	const position pos = line.find("\033[", start);
	if(pos == std::string::npos)
		return {};

	// find the 'm'
	position after_index = pos;
	for(position i = pos + 2; i < line.length(); ++i){
		if(line[i] == 'm'){
			after_index = i + 1;
			break;
		}
	}
	const int length = after_index - pos;

	const std::string sequence = line.substr(pos, length);
	return {pos, length, code_to_span(sequence)};
}

static std::string colorize(const std::string &line){
	bool open_span = false;
	std::string colorized;

	sequence_position pos = find_next_sequence(line, 0);

	for(position index = 0; index < line.length(); ++index){
		if(index == pos.start){
			if(!strcmp(pos.span, "</span>"))
				open_span = false;
			else if(open_span)
				colorized.append("</span>");
			else
				open_span = true;

			colorized.append(pos.span);
			index += pos.count - 1;
			pos = find_next_sequence(line, index + 1);
		}
		else{
			colorized.push_back(line[index]);
		}
	}

	return colorized;
}

#endif // WEB_VIEW_STATIC_RESOURCES_H
