#ifndef WEB_VIEW_STATIC_RESOURCES_H
#define WEB_VIEW_STATIC_RESOURCES_H

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
"</script>\n";

static std::string wrap(const std::string &body, const std::string &title, const std::string &head = ""){
	return
		"<!Doctype html>\n"
		"<html>\n"
		"<head>\n"
		"<title>" + title + "</title>\n"
		+ head + "\n"
		"</head>\n"
		"<body>\n"
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
