/*
 * Copyright (C) 2024 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <cxxopts.hpp>
#include "mainwin.h"

using namespace std;
using namespace egt;

int main(int argc, char** argv) {
	cxxopts::Options options(argv[0], "EGT SWUpdate Training Application");

	options.add_options()
	("h,help", "Show help")
	("f,config", "Path to swupdate config file, defaults to /etc/swupdate.cfg if no arguments specified", cxxopts::value<std::string>()->default_value("/etc/swupdate.cfg"))
	("v,version", "Show version");

	auto args = options.parse(argc, argv);
	if (args.count("help")) {
		cout << options.help() << endl;
		return 0;
	} else if (args.count("version")) {
		cout << "egt-swupdate " << EGT_SWUPDATE_VERSION << endl;
		return 0;
	}

	Application app(argc, argv);

	MainWindow window(args["config"].as<std::string>());

	window.show();

	return app.run();
}


