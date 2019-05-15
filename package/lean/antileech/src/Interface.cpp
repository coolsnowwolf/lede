/*
 * =====================================================================================
 *
 *       Filename:  Interface.cpp
 *
 *    Description:  A part of aMule DLP
 *
 *        Created:  2011-02-24 19:49
 *
 *         Author:  Bill Lee , bill.lee.y@gmail.com
 *	
 *	Copyright (c) 2011, Bill Lee
 *	License: GNU General Public License
 *
 * =====================================================================================
 */

/* #####   HEADER FILE INCLUDES   ################################################### */
#include "antiLeech.h"

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ############################ */
extern "C" IantiLeech* createAntiLeechInstant(){
	return new CantiLeech;
}
extern "C" int destoryAntiLeechInstant(IantiLeech* ptr){
	delete ptr;
	return 0;
}
