#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "Logger.h"

static char say[512];
static std::vector<std::tuple<unsigned, std::string> > buffer;

void fishtank_log(const char *fmt, ...){
	va_list args;
	va_start(args, fmt);

	vsnprintf(say, sizeof(say), fmt, args);
	buffer.emplace_back(time(NULL), say); // save it
	puts(say); // say it
	fflush(stdout);

	va_end(args);
}

const std::vector<std::tuple<unsigned, std::string> > &get_log_buffer(){
	return buffer;
}
