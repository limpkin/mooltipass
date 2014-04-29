SimpleHIDWrite is a tool to handle any HID device.
It is intended to aid in the development of HID devices or exploring unknown HID devices.
Knowledge of HID and USB is required to understand the program.

In the upper list box it presents the currently available HID devices. Select one to work with it.
A selected device is permanently read and all received reports are shown in the log.
The lower list box shows a log of the events happening.

"Info" shows a dialog with details about the selected device.
"Save As..." saves the log to a text file.
"Playback..." allows to play back a saved log. Only device selections and actions which write to the device are executed.
If you doubleclick an entry in the log list box it is executed if possible.
"Clear" simply clears the log list box.

The buttons of the lower row are only enabled if a device is selected.
"Write" enabled if the device declares an output report and can be written.
"Get Report" and "Set Report" are disabled for Windows 98 because the HID API does not support these calls for Windows 98.
"Get Feature" and "Set Feature" are enabled if the device declares a feature report.

The report to be written is entered in hex digits in the edit boxes.
Empty edit boxes are automatically filled with "00".