/*
 * Copyright (C) 2024 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MAINWIN_H__
#define __MAINWIN_H__

#include <egt/ui>
#include <egt/window.h>
#include "version.h"
#include "libconfig.h++"
#include "libuboot.h"

using namespace std;
using namespace egt;
using namespace egt::experimental;

static const std::string EGT_SWUPDATE_VERSION = std::to_string(EGT_SWUPDATE_VERSION_MAJOR) + "." + std::to_string(EGT_SWUPDATE_VERSION_MINOR) + "." + std::to_string(EGT_SWUPDATE_VERSION_PATCH);
inline static const std::vector<std::string> ubootEnvVars = {"upgrade_available", "bootcount", "ustate"};
inline static const std::vector<std::string> ustateVal = {"0", "1", "2", "3", "4", "5", "6", "7"};

typedef enum ubootEnvVars_t {
	ENV_UPGRADE = 0,
	ENV_BOOTCNT,
	ENV_USTATE,
	ENV_MAX,
} ubootEnvVars_t;

typedef enum ustate_t {
	STATE_OK = 0,
	STATE_INSTALLED = 1,
	STATE_TESTING = 2,
	STATE_FAILED = 3,
	STATE_NOT_AVAILABLE = 4,
	STATE_ERROR = 5,
	STATE_WAIT = 6,
	STATE_IN_PROGRESS = 7,
	STATE_LAST = STATE_IN_PROGRESS
} ustate_t;

class MainWindow : public TopWindow {
public:
	MainWindow(std::string const cfg);
	virtual ~MainWindow();

private:
	bool readConfigFile(std::string cfgFile);
	bool getAttrFromCfg(std::string node, std::string attr, std::string& val);
	bool getAttrFromCfg(std::string node, std::string subnode, std::string key, std::string& val);

	size_t initUbootEnvAccess(void);
	size_t setUbootEnvVar(ubootEnvVars_t var, std::string val);
	size_t writeUbootVarToEnv(ubootEnvVars_t var, std::string val);
	void checkIfUpdated(void);

	PeriodicTimer cpuTimer;
	CPUMonitorUsage cpuMon;
	std::shared_ptr<Label> timeClock;
	std::shared_ptr<Label> cpuMonLabel;

	libconfig::Config swupdateCfg;

	bool provisioned;

	struct uboot_ctx *ubootCtx;
	ssize_t ustate;
	bool writeUbootEnv = false;
};

#endif /* __MAINWIN_H__ */
