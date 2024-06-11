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

using namespace std;
using namespace egt;

static const std::string EGT_SWUPDATE_VERSION = std::to_string(EGT_SWUPDATE_VERSION_MAJOR) + "." + std::to_string(EGT_SWUPDATE_VERSION_MINOR) + "." + std::to_string(EGT_SWUPDATE_VERSION_PATCH);

class MainWindow : public TopWindow {
public:
	MainWindow();
	virtual ~MainWindow();

private:

};

#endif /* __MAINWIN_H__ */
