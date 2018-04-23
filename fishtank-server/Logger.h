#ifndef FISHTANK_LOG_H
#define FISHTANK_LOG_H

#include <vector>
#include <string>

void fishtank_log(const char*, ...);

const std::vector<std::string> &get_log_buffer();

#endif // FISHTANK_LOG_H
