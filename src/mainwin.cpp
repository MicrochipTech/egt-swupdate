/*
 * Copyright (C) 2024 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <sstream>
#include <ctime>
#include <filesystem>
#include "mainwin.h"

using namespace std;
using namespace egt;
using namespace egt::experimental;

static std::string getTime(void) {
        time_t sysTime = time(0);
        return ctime(&sysTime);
}

MainWindow::MainWindow(std::string const cfg) {
        provisioned = false;
        ubootCtx = NULL;
        ustate = 0;

        initUbootEnvAccess();
        checkIfUpdated();
        readConfigFile(cfg);

        std::string boardVer, serNum, hwVer, swVer, appVer, appDataVer, certFile;

        getAttrFromCfg("identify", "board", "value", boardVer);
        getAttrFromCfg("identify", "serial", "value", serNum);
        getAttrFromCfg("identify", "HW Version", "value", hwVer);
        getAttrFromCfg("identify", "SW Version", "value", swVer);
        getAttrFromCfg("suricatta", "sslcert", certFile);

        auto hsizer = make_shared<BoxSizer>(Orientation::horizontal);
        auto vsizer = make_shared<BoxSizer>(egt::Orientation::vertical);
        auto attrSizer = make_shared<VerticalBoxSizer>();
        auto verSizer = make_shared<VerticalBoxSizer>();

        add(expand(vsizer));

        auto header = make_shared<Frame>(Size(0, 60));

        header->fill_flags(Theme::FillFlag::blend);
        header->height(50);

        vsizer->add(expand_horizontal(header));
        vsizer->add(expand(hsizer));
        hsizer->add(expand(attrSizer));
        hsizer->add(expand(verSizer));

        timeClock = make_shared<Label>(getTime(), AlignFlag::center);
        timeClock->color(Palette::ColorId::bg, Palette::transparent);
        timeClock->align(AlignFlag::center_horizontal | AlignFlag::center_vertical);
        timeClock->margin(10);
        timeClock->font(egt::Font(24));
        header->add(timeClock);

        auto appName = make_shared<ImageLabel>(egt::Image("icon:cancel.png"), "A Super Cool App ");
        appName->color(Palette::ColorId::bg, Palette::transparent);
        appName->align(AlignFlag::left | AlignFlag::center_vertical);
        appName->image_align(AlignFlag::right);
        appName->margin(10);
        appName->font(egt::Font(24));
        header->add(appName);

        if (std::filesystem::exists(certFile) == true) {
                provisioned = true;
                appName->image(egt::Image("icon:ok.png"));
        }

        cpuMon = CPUMonitorUsage();
        cpuMonLabel = make_shared<Label>("CPU:---", Rect(0, 0, 100, 50));
        cpuMonLabel->color(Palette::ColorId::bg, Palette::transparent);
        cpuMonLabel->align(AlignFlag::right | AlignFlag::top);
        cpuMonLabel->margin(10);
        cpuMonLabel->font(egt::Font(24));
        header->add(cpuMonLabel);

        cpuTimer = PeriodicTimer(std::chrono::seconds(1));

        cpuTimer.on_timeout([this, appName, certFile]() {
                cpuMon.update();
                ostringstream ss;
                ss << "CPU: " << static_cast<int>(cpuMon.usage()) << "%";
                cpuMonLabel->text(ss.str());

                timeClock->text(getTime());

                if (provisioned == false) {
                        if (std::filesystem::exists(certFile) == true) {
                                provisioned = true;
                                appName->image(egt::Image("icon:ok.png"));
                        }
                }
        });

        auto board = make_shared<Label>("Board:", AlignFlag::left);
        board->color(Palette::ColorId::bg, Palette::transparent);
        board->align(AlignFlag::left | AlignFlag::top);
        board->margin(10);
        board->font(egt::Font(24));

        auto ser = make_shared<Label>("Serial Number:", AlignFlag::left);
        ser->color(Palette::ColorId::bg, Palette::transparent);
        ser->align(AlignFlag::left | AlignFlag::top);
        ser->margin(10);
        ser->font(egt::Font(24));

        auto hw = make_shared<Label>("HW Version:", AlignFlag::left);
        hw->color(Palette::ColorId::bg, Palette::transparent);
        hw->align(AlignFlag::left | AlignFlag::top);
        hw->margin(10);
        hw->font(egt::Font(24));

        auto sw = make_shared<Label>("SW Version:", AlignFlag::left);
        sw->color(Palette::ColorId::bg, Palette::transparent);
        sw->align(AlignFlag::left | AlignFlag::top);
        sw->margin(10);
        sw->font(egt::Font(24));

        auto app = make_shared<Label>("App Version:", AlignFlag::left);
        app->color(Palette::ColorId::bg, Palette::transparent);
        app->align(AlignFlag::left | AlignFlag::top);
        app->margin(10);
        app->font(egt::Font(24));

        attrSizer->add(board);
        attrSizer->add(ser);
        attrSizer->add(hw);
        attrSizer->add(sw);
        attrSizer->add(app);

        auto boardName = make_shared<Label>(boardVer, AlignFlag::left);
        boardName->color(Palette::ColorId::bg, Palette::transparent);
        boardName->align(AlignFlag::left | AlignFlag::top);
        boardName->margin(10);
        boardName->font(egt::Font(24));

        auto serialNum = make_shared<Label>(serNum, AlignFlag::left);
        serialNum->color(Palette::ColorId::bg, Palette::transparent);
        serialNum->align(AlignFlag::left | AlignFlag::top);
        serialNum->margin(10);
        serialNum->font(egt::Font(24));

        auto hwVersion = make_shared<Label>(hwVer, AlignFlag::left);
        hwVersion->color(Palette::ColorId::bg, Palette::transparent);
        hwVersion->align(AlignFlag::left | AlignFlag::top);
        hwVersion->margin(10);
        hwVersion->font(egt::Font(24));

        auto swVersion = make_shared<Label>(swVer, AlignFlag::left);
        swVersion->color(Palette::ColorId::bg, Palette::transparent);
        swVersion->align(AlignFlag::left | AlignFlag::top);
        swVersion->margin(10);
        swVersion->font(egt::Font(24));

        auto appVersion = make_shared<Label>(EGT_SWUPDATE_VERSION, AlignFlag::left);
        appVersion->color(Palette::ColorId::bg, Palette::transparent);
        appVersion->align(AlignFlag::left | AlignFlag::top);
        appVersion->margin(10);
        appVersion->font(egt::Font(24));

        verSizer->add(boardName);
        verSizer->add(serialNum);
        verSizer->add(hwVersion);
        verSizer->add(swVersion);
        verSizer->add(appVersion);

        cpuTimer.start();
}

MainWindow::~MainWindow() {

}

bool MainWindow::readConfigFile(std::string cfgFile) {
        try {
                swupdateCfg.readFile(cfgFile.c_str());
                return true;
        } catch(const libconfig::FileIOException &fioex) {
                std::cerr << "I/O error while reading config file." << std::endl;
                return false;
        } catch(const libconfig::ParseException &pex) {
                std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
                return false;
        }
}

bool MainWindow::getAttrFromCfg(std::string node, std::string attr, std::string& val) {
        try {
                libconfig::Setting &root = swupdateCfg.getRoot();

                const libconfig::Setting &n = root.lookup(node);

                if (n.exists(attr)) {
                        n.lookupValue(attr, val);
                        return true;
                }

                return false;

        } catch(const libconfig::SettingNotFoundException &nfex) {
                return false;
        }
}

bool MainWindow::getAttrFromCfg(std::string node, std::string subnode, std::string key, std::string& val) {
        try {
                libconfig::Setting &root = swupdateCfg.getRoot();
                const libconfig::Setting &n = root.lookup(node);

                for (libconfig::Setting const& sn : n) {
                        std::string snName;
                        sn.lookupValue("name", snName);
                        if (snName == subnode) {
                                sn.lookupValue(key, val);
                                return true;
                        }
                }
        } catch(const libconfig::SettingNotFoundException &nfex) {
                return false;
        }

        return false;
}

size_t MainWindow::initUbootEnvAccess(void) {
        int ret = 0;
        const char *ns;
        const char *value;

        ret = libuboot_read_config_ext(&ubootCtx, "/etc/fw_env.config");

        if (ret) {
		cout << "Cannot initialize u-boot env" << endl;
                return -1;
	}

	ns = libuboot_namespace_from_dt();

        if (ns) {
                ubootCtx = libuboot_get_namespace(ubootCtx, ns);
        }

	if (!ubootCtx) {
		cout << "Namespace not found" << endl;
		return -1;
	}

	if ((ret = libuboot_open(ubootCtx)) < 0) {
		cout << "Cannot read environment" << endl;
                return -1;
	}

        value = libuboot_get_env(ubootCtx, "ustate");

        ustate = stoi(value);

        libuboot_close(ubootCtx);

        return 0;
}

size_t MainWindow::setUbootEnvVar(ubootEnvVars_t var, std::string val) {
        int ret;
        const char *value;

        cout << "Setting " << ubootEnvVars.at(var) << " to " << val << endl;
        value = libuboot_get_env(ubootCtx, ubootEnvVars.at(var).c_str());

        if (value == NULL || val.compare(value) != 0) {
                writeUbootEnv = true;

                ret = libuboot_set_env(ubootCtx, ubootEnvVars.at(var).c_str(), val.c_str());
                if (ret) {
                        cout << "libuboot_set_env failed: " << ret << endl;
                        return -1;
                }
        } else {
                cout << "Not setting " << ubootEnvVars.at(var) << ", value is the same." << endl;
                writeUbootEnv = false;
        }

        return 0;
}

size_t MainWindow::writeUbootVarToEnv(ubootEnvVars_t var, std::string val) {
        int ret;

        if ((ret = libuboot_open(ubootCtx)) < 0) {
		cout << "Cannot read environment" << endl;
                return -1;
	}

        if (setUbootEnvVar(var, val) != 0) {
                return -1;
        };

        if (writeUbootEnv == true) {
                cout << "Writing u-boot env to memory." << endl;
                writeUbootEnv = false;

                ret = libuboot_env_store(ubootCtx);

                if (ret) {
                        cout << "Error storing the env" << endl;
                        return -1;
                }
        }

        libuboot_close(ubootCtx);

        return 0;
}

void MainWindow::checkIfUpdated(void) {
        if ((ustate == STATE_INSTALLED) || (ustate == STATE_TESTING)) {
                cout << "Software Updated successfully!" << endl;
                writeUbootVarToEnv(ENV_USTATE, ustateVal.at(STATE_OK));
        }
}
