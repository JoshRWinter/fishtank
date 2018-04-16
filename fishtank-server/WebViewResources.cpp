#include "WebView.h"
#include "WebViewResourcesHelper.h"

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
	const std::string style = "\"padding: 10px 20px;border: 1px solid black;\"";

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
			const std::string play_time = format(time(NULL) - client.play_time);

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
			std::string escaped = cm.message;
			WebView::html_entities(escaped);
			content += std::string("<font color=") + (cm.from == "server" ? "\"red\"" : "\"blue\"") + ">" + cm.from + "</font>: " + escaped + "<br>\n";
		}
		content += "</div>";
	}
	else{
		content += "No Clients are connected.";
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
