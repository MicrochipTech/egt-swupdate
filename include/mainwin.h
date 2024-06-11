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

#define HASH_CHUNK_SIZE 4096

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

class RebootWindow : public egt::Popup {
public:
	explicit RebootWindow() : egt::Popup(egt::Application::instance().screen()->size() / 2) {
		colorSwitch = 0;
		this->color(Palette::ColorId::bg, Palette::red);
		rebootCnt = 30;

		rebootWarn = Label("Update Available! Rebooting in " + std::to_string(rebootCnt));
		add(egt::center(rebootWarn));

		cancel = Button("Cancel");
		cancel.align(egt::AlignFlag::right | egt::AlignFlag::bottom);

		add(cancel);

		cancel.on_click([this](egt::Event&) {
			cout << "Cancelling reboot..." << endl;
			rebootTimer.stop();
			this->hide();
		});

		rebootTimer = PeriodicTimer(std::chrono::seconds(1));

		rebootTimer.on_timeout([this]() {
			if (colorSwitch) {
				colorSwitch ^= 1;
				this->color(Palette::ColorId::bg, Palette::red);
			} else {
				colorSwitch ^= 1;
				this->color(Palette::ColorId::bg, Palette::yellow);
			}

			if (rebootCnt-- == 0) {
				system("/usr/sbin/reboot");
			} else {
				rebootWarn.text("Update Available! Rebooting in " + std::to_string(rebootCnt));
			}
		});
	}

	void startRebootTimer(size_t countdown) {
		rebootCnt = countdown;
		rebootWarn.text("Rebooting in " + std::to_string(rebootCnt));
		rebootTimer.start();
	}

protected:
	Label rebootWarn;
	Button cancel;
	size_t rebootCnt;
	PeriodicTimer rebootTimer;
	size_t colorSwitch;

private:

};

class MainWindow : public TopWindow {
public:
	MainWindow(std::string const cfg);
	virtual ~MainWindow();

private:
	std::string getTime(void);
	std::string getTime(ssize_t future);

	bool readConfigFile(std::string cfgFile);
	bool getAttrFromCfg(std::string node, std::string attr, std::string& val);
	bool getAttrFromCfg(std::string node, std::string subnode, std::string key, std::string& val);
	void getServerAttrs(void);

	size_t initUbootEnvAccess(void);
	size_t setUbootEnvVar(ubootEnvVars_t var, std::string val);
	size_t writeUbootVarToEnv(ubootEnvVars_t var, std::string val);
	size_t setUpdateAvailableInUbootEnv(void);
	void checkIfUpdated(void);
	bool hashAppData(std::string file, std::string& digest);
	bool pollHawkbitServer(void);
	bool sendMsgToHawkbitServer(void);

	PeriodicTimer cpuTimer;
	CPUMonitorUsage cpuMon;
	std::shared_ptr<Label> timeClock;
	std::shared_ptr<Label> cpuMonLabel;
	std::shared_ptr<Label> pollTime;

	libconfig::Config swupdateCfg;

	bool provisioned;

	struct uboot_ctx *ubootCtx;
	ssize_t ustate;
	bool writeUbootEnv = false;

	PeriodicTimer updatePollTimer;
	ssize_t serverPollTime;
	ssize_t actionId;
	bool updateAvailable;
	bool updateInstalled;

	std::string uri;
	std::string sslkey;
	std::string sslcert;

	std::string appDataFile;
	std::string appDataMd;

	RebootWindow rebootWin;
};

#endif /* __MAINWIN_H__ */
