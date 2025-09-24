/*----------------------------------------------------------------------------
 INGENICO Technical Software Department
 ------------------------------------------------------------------------------
 Copyright (c) 2015, Ingenico.
 28-32 boulevard de Grenelle 75015 Paris, France.
 All rights reserved.
 This source program is the property of INGENICO Company and may not be copied
 in any form or by any means, whether in part or in whole, except under license
 expressly granted by INGENICO company
 All copies of this program, whether in part or in whole, and
 whether modified or not, must display this and all other
 embedded copyright and ownership notices in full. */

#include <CvConnect.hpp>
#include "AppResources.hpp"
// Namespace used for GOAL classes
using namespace ingenico::graphics;

// Namespace used for Standard Template Library (STL)
using namespace std;

int main(void)
{
	// Create goal sample application
	string myClassName = "local.template.";
	myClassName += _ING_APPLI_FAMILY;
	CvConnect app(myClassName + "CvConnect");
	SGL::ref().setCharset(GL_ENCODING_ISO_8859_1);
	app.open();
	while (1)
	{
		// Dispatch all service events
		app.dispatch();
	}
	return 0;
}
