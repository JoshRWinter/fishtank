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
"</style>"
;

static const std::string body_pre =
"<div id=\"topdiv\">\n"
"<p id=\"titleheader\" class=\"titlebar\">FISHTANK</p>\n"
"<span id=\"navspan\" class=\"titlebar\">\n"
"<a class=\"navlink\" href=\"/\">Status</a>"
"<a class=\"navlink\" href=\"/help\">Help</a>\n"
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

#endif // WEB_VIEW_STATIC_RESOURCES_H
