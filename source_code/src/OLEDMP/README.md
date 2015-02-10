#OLED Library for Mooltipass

The OLED library implements support for displaying text and graphics for the Mooltipass's 256x64 OLED display.

It provides functions for the following features:
- Initialise the display.
- Clear the display.
- Turning the display on and off.
- Writing text to the display.
- Drawing bitmaps on the display.
- Scroll the display.

## Display Buffers

The library uses the OLED's buffer memory to implement two 256x64 display buffers.  Either buffer can be selected to be displayed, and either can be selected as the target for drawing text and bitmaps.

The oledWriteActiveBuffer() function selects the currently displayed (active) buffer as the target for text and bitmap operations, and the oledWriteInactiveBuffer() function selects the currently hidden (inactive) buffer instead.  By building a display on the inactive buffer, the new image can be quickly shown on the display by swapping the active buffer with the inactive buffer using the oledFlipDisplayedBuffer() function.

The inactive buffer can also be scrolled onto the display at a controlled rate with the oledFlipBuffers(mode, delay) function. The mode can be OLED_SCROLL_UP or OLED_SCROLL_DOWN, scrolling the new image from bottom up onto the display line by line or from top down onto display.  The delay parameter controls the speed of the scroll by setting the number of milliseconds of delay before scrolling the next line onto the display.
