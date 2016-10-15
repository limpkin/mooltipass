# Mooltipass Python Tool

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
- start a command prompt inside this directory
- type 'pip install -r requirements.txt'

### Libusb Install
- Connect the Mooltipass to your computer
- Double click on the .exe installation package
- Follow the steps and launch the filter installer wizard at the installation procedure
- Keep 'Install a device filter' selected, click on Next
- Select the 'vid:16d0 pid:09a0 rev:0100' device, click 'Install'
- Select the 'vid:16d0 pid:09a0 rev:0100 mi:00' device, click 'Install'
- Close the window

## Installation on Linux

To access the device as normal user make sure you have set up the correct [udev rules](https://www.themooltipass.com/udev_rule.txt). If you're not willing to
and are feeling adventurous, you may run mooltipass_tool.py with sudo. **Using
sudo is not recommended as it is dangerous and can cause serious problems.**

### Ubuntu 16.04
```
sudo apt-get install python python-pip python-crypto python-dev libgmp3-dev build-essential
pip install -r requirements.txt
./mooltipass_tool.py (when udev rules are installed)
sudo ./mooltipass_tool.py (when no udev rules are installed)
```

### Arch Linux

From official repositories:
```
sudo pacman -S --needed python2 python2-crypto
```

From AUR:
```
python2-pyusb python2-intelhex python2-pyqrcode

# Not on aur (yet):
pip install seccure
```

### Other Linux Distributions

Install `python2` and `pip` for you linux distribution.
It is named `python2` or `python2` and `python2-pip` or `python-pip` in most cases.
Then install all dependencies via pip:

```
pip install -r requirements.txt
./mooltipass_tool.py
```
