#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <string>
#include <thread>

#include "sag/rec/MatchRecorder.h"
#include "tools/CmdLineThread.h"

auto main() -> int {
	const std::map<std::string, sag::rec::Signal> cliRecorderSignals = {
		{"record", sag::rec::Signal::Record},
		{"r", sag::rec::Signal::Record},
		{"halt", sag::rec::Signal::Halt},
		{"h", sag::rec::Signal::Halt},
		{"quit", sag::rec::Signal::Quit},
		{"q", sag::rec::Signal::Quit},
		{"exit", sag::rec::Signal::Quit},
		{"status", sag::rec::Signal::Status},
		{"s", sag::rec::Signal::Status},
		{"info", sag::rec::Signal::Status},
	};

	auto logger = spdlog::default_logger();
	logger->set_level(spdlog::level::info);
	logger->info("recorder start");
	tools::CmdLineThread cmd_line_thread;

	using TTTRec = sag::rec::TicTacToeRecorder;

	TTTRec::graph::container graph;
	TTTRec::graph::rules rules;
	TTTRec::storage storage;
	std::vector<TTTRec::player> players = {{}, {}};
	sag::rec::MatchRecorder<TTTRec> recorder(players, graph, rules, storage);
	tools::MutexQueue<sag::rec::Signal> recorder_signal_queue;
	std::jthread recorder_thread(recorder, std::ref(recorder_signal_queue));

	while (true) {
		std::string command = cmd_line_thread.queue().wait_and_dequeue();
		std::ranges::transform(command, command.begin(), [](unsigned char letter) { return std::tolower(letter); });
		if (cliRecorderSignals.contains(command)) {
			sag::rec::Signal signal = cliRecorderSignals.at(command);
			recorder_signal_queue.emplace(signal);
			if (signal == sag::rec::Signal::Quit)
				break;
		} else {
			logger->warn("unknown command: {0}", command);
		}
	}

	logger->info("recorder end");
	return 0;
}
