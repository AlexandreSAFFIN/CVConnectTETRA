/*
 * AppRecources.hpp
 *
 *  Created on: Apr 6, 2021
 *      Author: gourmi
 */

#ifndef INC_APPRESOURCES_HPP_
#define INC_APPRESOURCES_HPP_

#define BOLD_TEXT "\x1B('style':'bold')"

#define AUTH 1
#define CREATE 2

#define DISK_PATH "/CVCONNECT"
#define FIC_PARAM "/CVCONNECT/PARAM"
#define FIC_LICENSE "/CVCONNECT/LICENSE"
#define FIC_UPDATE "/CVCONNECT/UPDATE"

#define HOST_PROD "api-cfpay.cib-ingenierie.fr"
#define HOST_DEV "api-cfpay-dev.cib-ingenierie.fr"


//! Events detected
#define EVENT_NONE 0
#define EVENT_END 1

//! Card detection methods
#define GET_DATA 0x0001
#define PAY 0x0010

// Detection return value
#define DETECTED 1
#define CANCEL 2
#define CONTINUE 3

/**#######################################################################
 * ################# Please don't change the order #######################
 * #######################################################################*/

#include "cib/graphics/Graphics.hpp"
#include "cib/json/Json.hpp"
#include "cib/application/Application.hpp"
#include "cib/tools/Tools.hpp"
#include <cib/disk/Disk.hpp>
#include "cib/communication/Communication.hpp"
#include "cib/settings/Settings.hpp"
#include "cib/devices/Devices.hpp"
#include "ingenico/tools/Singleton.hpp"

using namespace ingenico::tools;

using namespace cib::application;
// using namespace cib::transaction;
using namespace cib::communication;
using namespace cib::communication::http;
using namespace cib::communication::ssl;
using namespace cib::disk;
using namespace cib::json;
using namespace cib::graphics;
using namespace cib::devices;
using namespace cib::settings::params;

/**#######################################################################
 * ################# Please don't change the order #######################
 * #######################################################################*/


#endif /* INC_APPRESOURCES_HPP_ */
