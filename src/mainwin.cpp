/*
 * Copyright (C) 2024 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <sstream>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <nlohmann/json.hpp>
#include "mainwin.h"
#include "http.h"

using namespace std;
using namespace egt;
using namespace egt::experimental;
using json = nlohmann::json;

static std::string getTime(void) {
        time_t sysTime = time(0);
        return ctime(&sysTime);
}

MainWindow::MainWindow(std::string const cfg) {
        provisioned = false;
        ubootCtx = NULL;
        ustate = 0;
        serverPollTime = 300;   // 5 min default
        updateAvailable = false;
        updateInstalled = false;

        appDataFile = std::string("/opt/data/app_data.img");
        hashAppData(appDataFile, appDataMd);

        initUbootEnvAccess();
        checkIfUpdated();
        readConfigFile(cfg);
        getServerAttrs();

        std::string boardVer, serNum, hwVer, swVer, appVer, certFile;

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

        auto hash = make_shared<Label>("App Data Hash:", AlignFlag::left);
        hash->color(Palette::ColorId::bg, Palette::transparent);
        hash->align(AlignFlag::left | AlignFlag::top);
        hash->margin(10);
        hash->font(egt::Font(24));

        auto poll = make_shared<Label>("Next Check-in:", AlignFlag::left);
        poll->color(Palette::ColorId::bg, Palette::transparent);
        poll->align(AlignFlag::left | AlignFlag::top);
        poll->margin(10);
        poll->font(egt::Font(24));

        attrSizer->add(board);
        attrSizer->add(ser);
        attrSizer->add(hw);
        attrSizer->add(sw);
        attrSizer->add(app);
        attrSizer->add(hash);
        attrSizer->add(poll);

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

        auto appHash = make_shared<Label>(appDataMd.substr(0, 22) + " ...", AlignFlag::left);
        appHash->color(Palette::ColorId::bg, Palette::transparent);
        appHash->align(AlignFlag::left | AlignFlag::top);
        appHash->margin(10);
        appHash->font(egt::Font(24));

        pollTime = make_shared<Label>(" ", AlignFlag::left);
        pollTime->color(Palette::ColorId::bg, Palette::transparent);
        pollTime->align(AlignFlag::left | AlignFlag::top);
        pollTime->margin(10);
        pollTime->font(egt::Font(24));

        verSizer->add(boardName);
        verSizer->add(serialNum);
        verSizer->add(hwVersion);
        verSizer->add(swVersion);
        verSizer->add(appVersion);
        verSizer->add(appHash);
        verSizer->add(pollTime);

        cpuTimer.start();

        pollHawkbitServer();

        updatePollTimer = PeriodicTimer(std::chrono::seconds(serverPollTime));

        updatePollTimer.on_timeout([this]() {
                // check for new poll time
                pollHawkbitServer();

                if (updateAvailable == true) {
                        if (setUpdateAvailableInUbootEnv() != 0) {
                                cout << "Error setting u-boot env, not rebooting" << endl;
                        } else {
                                rebootWin.startRebootTimer(10);
                                rebootWin.show_modal(true);
                        }
                }
        });

        updatePollTimer.start();
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

size_t MainWindow::setUpdateAvailableInUbootEnv(void) {
        int ret;

        if ((ret = libuboot_open(ubootCtx)) < 0) {
		cout << "Cannot read environment" << endl;
                return ret;
	}

        if (setUbootEnvVar(ENV_UPGRADE, "1") != 0) {
                return -1;
        }

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

bool MainWindow::hashAppData(std::string file, std::string& digest) {
        EVP_MD_CTX *mdCtx;
        const EVP_MD *mdAlg = EVP_get_digestbyname("SHA256");
        unsigned char buf[HASH_CHUNK_SIZE];
        unsigned char md[SHA256_DIGEST_LENGTH];
        FILE *appData;
        unsigned int cnt = 0;

        std::stringstream hash;

        appData = fopen(file.c_str(), "r");

        if (!appData) {
                cout << "Error opening app data file" << endl;
                return false;
        }

        mdCtx = EVP_MD_CTX_new();
        if (!EVP_DigestInit_ex(mdCtx, mdAlg, NULL)) {
                cout << "EVP_DigestInit failed" << endl;
                EVP_MD_CTX_free(mdCtx);
                return false;
        }

        do {
                cnt = fread(buf, 1, HASH_CHUNK_SIZE, appData);
                if (!EVP_DigestUpdate(mdCtx, buf, cnt)) {
                        cout << "EVP_DigestUpdate failed" << endl;
                        EVP_MD_CTX_free(mdCtx);
                        return false;
                }
        } while (cnt > 0);

        unsigned int outlen;
        if (!EVP_DigestFinal_ex(mdCtx, md, &cnt)) {
                cout << "EVP_DigestFinal failed" << endl;
                EVP_MD_CTX_free(mdCtx);
                return false;
        }

        EVP_MD_CTX_free(mdCtx);

        fclose(appData);

        hash << std::hex << std::uppercase << std::setfill('0');

        for (const auto &c: md) {
                hash << std::setw(2) << (int)c;
        }

        digest = hash.str();

        return true;
}

void MainWindow::getServerAttrs(void) {
        std::string tenant, id;
        getAttrFromCfg("suricatta", "url", uri);

        // if we have an id field in the config object, then a config file was passed to the program
        // construct URL from config file, otherwise, command line arguments were used and URL is fully formed
        if (getAttrFromCfg("suricatta", "id", id) == true) {
                getAttrFromCfg("suricatta", "tenant", tenant);
                uri.append("/" + tenant + "/controller/v1/" + id);
        }

        if (uri.contains("https")) {
                getAttrFromCfg("suricatta", "sslkey", sslkey);
                getAttrFromCfg("suricatta", "sslcert", sslcert);
        }
}

std::string MainWindow::getTime(void) {
        time_t sysTime = time(0);
        return ctime(&sysTime);
}

std::string MainWindow::getTime(ssize_t future) {
        time_t now = time(0);
        time_t futureTime = now + future;

        return ctime(&futureTime);
}

bool MainWindow::pollHawkbitServer(void) {
        HTTP updateServer;
        std::string res;

        res = updateServer.get(uri, sslkey, sslcert);

        auto updateServerJson = nlohmann::json::parse(res);

        // check for config and get polling time
        if (updateServerJson.contains("config")) {
                const auto& config  = updateServerJson.at("config");

                if (config.contains("polling")) {
                        const auto& polling = config.at("polling");
                        std::istringstream ss;
                        std::tm t;

                        ss.str(polling.at("sleep"));
                        ss >> std::get_time(&t, "%H:%M:%S");
                        serverPollTime = (t.tm_hour * 3600) + (t.tm_min * 60) + (t.tm_sec);
                        //cout << "Next poll time is " << getTime(serverPollTime) << endl;
                        pollTime->text(getTime(serverPollTime));
                } else {
                        cout << "Warning, did not get polling time from server, setting to default 5 minutes" << endl;
                        serverPollTime = 300;
                }

        } else {
                cout << "Error parsing JSON from server, no config node!" << endl;
                return false;
        }

        if (updateServerJson.contains("_links")) {
                if (updateServerJson["_links"].contains("deploymentBase")) {
                        const auto& deploymentBase = updateServerJson.at("_links").at("deploymentBase");

                        // get actionId
                        std::string s = deploymentBase.at("href");
                        unsigned start = s.find("deploymentBase/");
                        unsigned startSize = std::string("deploymentBase/").size();
                        unsigned startPos = start + startSize;
                        unsigned end = s.find("?");

                        std::string id = s.substr(startPos, end - startPos);

                        actionId = stoi(id);

                        // check if an update was recently installed or if one is available
                        if (updateInstalled == false) {
                                cout << "Update available with actionId: " << actionId << endl;
                                updateAvailable = true;
                        }
                }
        }

        return true;
}
