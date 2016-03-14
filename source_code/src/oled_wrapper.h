/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*! \file   oled_wrapper.h
 *  \brief  OLED library wrapper
 *  Copyright [2016] [Mathieu Stephan]
 */
#include "oledmini.h"
#include "defines.h"
#include "oledmp.h"

#ifndef OLED_WRAPPER_H_
#define OLED_WRAPPER_H_

#if defined(HARDWARE_OLIVIER_V1)
    #define oledInitIOs()                   stockOledInitIOs()
    #define oledInvertedDisplay()           stockOledInvertedDisplay()
    #define oledNormalDisplay()             stockOledNormalDisplay()
    #define oledBegin(x)                    stockOledBegin(x)
    #define oledBitmapDrawFlash(a,b,c,d)    stockOledBitmapDrawFlash(a,b,c,d)
    #define oledPutstrXY(a,b,c,d)           stockOledPutstrXY(a,b,c,d)
    #define oledClear()                     stockOledClear()
    #define oledDisplayOtherBuffer()        stockOledDisplayOtherBuffer()
    #define oledFillXY(a,b,c,d,e)           stockOledFillXY(a,b,c,d,e)
    #define oledIsOn()                      stockOledIsOn()
    #define oledOn()                        stockOledOn()
    #define oledOff()                       stockOledOff()
    #define oledWriteInactiveBuffer()       stockOledWriteInactiveBuffer()
    #define oledPutch(x)                    stockOledPutch(x)
    #define oledSetXY(x,y)                  stockOledSetXY(x,y)
    #define oledPutstr(x)                   stockOledPutstr(x)
    #define oledSetFont(x)                  stockOledSetFont(x)
#elif defined(MINI_VERSION)
    #define oledInitIOs()                   miniOledInitIOs()
    #define oledInvertedDisplay()           miniOledInvertedDisplay()
    #define oledNormalDisplay()             miniOledNormalDisplay()
    #define oledBegin(x)                    miniOledBegin(x)
    #define oledBitmapDrawFlash(a,b,c,d)    miniOledBitmapDrawFlash(a,b,c,d)
    #define oledPutstrXY(a,b,c,d)           miniOledPutstrXY(a,b,c,d)
    #define oledClear()                     miniOledClearFrameBuffer()
    #define oledDisplayOtherBuffer()        miniOledDisplayOtherBuffer()
    #define oledFillXY(a,b,c,d,e)           miniOledDrawRectangle(a,b,c,d,e)
    #define oledIsOn()                      miniOledIsScreenOn()
    #define oledOn()                        miniOledOn()
    #define oledOff()                       miniOledOff()
    #define oledWriteInactiveBuffer()       miniOledWriteInactiveBuffer()
    #define oledPutch(x)                    miniOledPutch(x)
    #define oledSetXY(x,y)                  miniOledSetXY(x,y)
    #define oledPutstr(x)                   miniOledPutstr(x)
    #define oledSetFont(x)                  miniOledSetFont(x)
#endif

#endif /* OLED_WRAPPER_H_ */