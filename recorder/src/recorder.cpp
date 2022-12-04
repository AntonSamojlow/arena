// recorder.cpp : Defines the entry point for the application.
//

#include "recorder.h"
#include <spdlog/spdlog.h>

using namespace std;

int main()
{
	auto logger = spdlog::default_logger();
	logger->info("recorder start");
	logger->info("recorder end");
	return 0;
}
