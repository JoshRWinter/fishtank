#ifndef FISHTANK_LOG_H
#define FISHTANK_LOG_H

#include <vector>
#include <tuple>
#include <string>

void fishtank_log(const char*, ...);

const std::vector<std::tuple<unsigned, std::string> > &get_log_buffer();

#endif // FISHTANK_LOG_H
