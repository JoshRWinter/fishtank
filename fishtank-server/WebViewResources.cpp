#include "WebView.h"
#include "WebViewResources.h"

// http 500
ROUTE_ADD(http_internal_server_error, args, match){
	const std::string content = "<h2>500 Internal Server Error</h2>";

	return {wrap(content, "500 Internal Server Error"), HTTP_STATUS_INTERNAL_ERROR};
}

// http 404
ROUTE_ADD(http_not_found, args, match){
	const std::string content = "<h2>404 Not Found</h2>";

	return {wrap(content, "404 Not Found"), HTTP_STATUS_NOT_FOUND};
}

// http 403
ROUTE_ADD(http_forbidden, args, match){
	const std::string content = "<h2>403 Forbidden</h2><p>nice try</p>";

	return {wrap(content, "403 Forbidden"), HTTP_STATUS_FORBIDDEN};
}

// homepage
ROUTE_ADD(index, args, match){
	const std::vector<ShortClient> summary = match.client_summary();
	const std::string style = "\"padding: 10px;border: 1px solid black;\"";

	// fill in the client table
	std::string content;
	if(summary.size() > 0){
		content += "<div style=\"float: left;padding-right: 30px;\"><h2>Clients</h2><table style=\"border: 1px solid black;border-collapse: collapse;\">"

		"<tr id=\"table_normal\"><th>Id</th>"
		"<th id=\"table_normal\">Name</th>"
		"<th id=\"table_normal\">Play Time</th>"
		"<th id=\"table_normal\">Match Victories</th>"
		"<th id=\"table_normal\">Kills</th>"
		"<th id=\"table_normal\">Deaths</th>"
		"<th id=\"table_normal\">Rounds Played</th>"
		"<th id=\"table_normal\">Action</th></tr>\n";

		for(const ShortClient &client : summary){
			const std::string id_string = std::to_string(client.id);
			const std::string play_time = format(time(NULL) - client.play_time);

			content +=
			"<tr>"
			"<td id=\"table_normal\">" + id_string + "</td>"
			"<td id=\"table_normal\">" + client.name + "</td>"
			"<td id=\"table_normal\">" + play_time + "</td>"
			"<td id=\"table_normal\">" + std::to_string(client.match_victories) + "</td>"
			"<td id=\"table_normal\">" + std::to_string(client.victories) + "</td>"
			"<td id=\"table_normal\">" + std::to_string(client.deaths) + "</td>"
			"<td id=\"table_normal\">" + std::to_string(client.rounds_played) + "</td>"
			"<td id=\"table_normal\"><a href=\"/kick/" + id_string + "\"><button class=\"button\">Kick</button></a></td>"
			"</tr>\n";
		}
		content += "</table></div><div style=\"float: left;max-width: 350px;\">";

		// fill in the chats table
		const std::vector<ChatMessage> chats = match.chat_log();
		content += "\n<h2>Chat Log</h2>";
		content += "<form action=\"/say\" method=\"get\"><input type=\"text\" id=\"saymessage\" name=\"m\" autocomplete=\"off\" style=\"margin-right: 10px;\" autofocus><input type=\"submit\" value=\"Say\"></form><br>";
		for(const ChatMessage &cm : chats){
			std::string escaped = cm.message;
			WebView::html_entities(escaped);
			content += std::string("<font color=") + (cm.from == "server" ? "\"red\"" : "\"blue\"") + ">" + cm.from + "</font>: " + escaped + "<br>\n";
		}
		content += "</div>";
	}
	else{
		content += "<p>No Clients are connected.</p>";
	}

	return {wrap(content, "Server Status", refresher_js)};
}

ROUTE_ADD(say, args, match){
	std::string message;
	if(args.length() < strlen("m?="))
		message = "";
	else
		message = args.substr(3);

	// get rid of the "%20"'s and related
	WebView::url_decode(message);

	if(message.length() > 0)
		match.send_chat(message);
	const std::string content = "<h2>Moved</h2>";

	return {wrap(content, "moved"), HTTP_STATUS_SEE_OTHER, "/"};
}

ROUTE_ADD(kick, args, match){
	int kick_id = -1;
	if(sscanf(args.c_str(), "%d", &kick_id) != 1)
		return {wrap("<h2>ID \"" + args + "\" does not exist.</h2>", "Invalid ID"), HTTP_STATUS_NOT_FOUND};
	else
		match.kick(kick_id);

	return {wrap("<h2>Moved</h2>", "See Other"), HTTP_STATUS_SEE_OTHER, "/"};
}

ROUTE_ADD(help, args, match){
	const std::string content =
	"<br>"
	"<img src=\"https://i.imgur.com/PyX3s3M.png\" alt=\"fishtank logo\">\n"
	"<h2>About</h2><p>"
	"Fishtank is an Android game capable of real-time multiplayer over the internet.<br>"
	"</p><p>"
	"A Client-Server model is used to coordinate all connected players. The Fishtank Game Server "
	"is intended to run on your computer, and you can connect to your computer's IP address to play."
	"</p><h2>Port Forwarding</h2><p>"
	"If you want to play with people who are outside of your local area network, then you need to configure "
	"your router to forward connections to your computer. This is called <b>Port Forwarding</b>."
	"</p><ol>"
	"<li>Open your router's web interface</li>"
	"<li>Find the settings for Port Forwarding</li>"
	"<li>Forward the following ports: 28856 (TCP), and 28857 (UDP)</li>"
	"</ol>"
	"<h2>Source Code</h2><p>"
	"Source code can be found on <a href=\"https://github.com/joshrwinter/fishtank\">github</a>."
	"</p>"
	;

	return wrap(content, "about");
}

ROUTE_ADD(configuration, args, match){
	if(args.length() > 0){
		if(args == "reset")
			match.reset_stats();
		else
			return http_not_found(args, match);

		return {wrap("<h2>See Other</h2>", "See Other"), HTTP_STATUS_SEE_OTHER, "/configuration"};
	}

	const std::string content =
	"<h2>Additional Configuration</h2>\n"
	"<table><tr><th id=\"table_noborder\">Action</th><th id=\"table_noborder\">Description</th></tr>\n"
	"<tr><td id=\"table_noborder\"><a href=\"/configuration/reset\"><button class=\"button\">Reset Stats</button></a></td><td id=\"table_noborder\">Resets ranked status for all connected players</td></tr>\n"
	"</table>\n"
	;

	return wrap(content, "Configuration");
}
