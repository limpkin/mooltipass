# Mooltipass Python Tool

## Installation on Ubuntu 16.04
```
sudo apt-get install python python-pip python-crypto
pip install -r requirements.txt
sudo python mooltipass_tool.py
```

## Installation on Windows
### Downloads
- Python 2.7.x from https://www.python.org/downloads/
- libusb-win32 from https://sourceforge.net/projects/libusb-win32/files/libusb-win32-releases/1.2.6.0/libusb-win32-devel-filter-1.2.6.0.exe/download

### Python Installation
- Double click on the .msi installation package.
- Install for all users
- Keep default install directory
- In the customize python page, select the 'Add python.exe to Path' option (entire feature...)
- Click 'Next' and 'Finish'
- Reboot the computer

### Python Modules Install
- start a command prompt inside the directory
- type 'pip install -r requirements.txt'

### Libusb Install
- Connect the Mooltipass to your computer
- Double click on the .exe installation package
- Follow the steps and launch the filter installer wizard at the installation procedure
- Keep 'Install a device filter' selected, click on Next
- Select the 'vid:16d0 pid:09a0 rev:0100' device, click 'Install'
- Select the 'vid:16d0 pid:09a0 rev:0100 mi:00' device, click 'Install'
- Close the window

## Arch Linux

From official repositories:
```
python2 python2-crypto
```

From AUR:
```
python2-pyusb python2-intelhex python2-pyqrcode
```
