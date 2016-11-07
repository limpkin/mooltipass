#!/bin/bash

# Install requirements. Currently only Ubuntu is supported. Look at the readme for other distributions.
sudo apt-get update
sudo apt-get install -y --force-yes python python-pip python-crypto python-dev libgmp3-dev build-essential > /dev/null

# Update tools manually via wget. git pull would be an alternative if cloned via git.
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/firmwareBundlePackAndSign.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/generic_hid_device.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/mooltipass_defines.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/mooltipass_hid_device.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/mooltipass_init_proc.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/mooltipass_security_check.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/mooltipass_tool.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/publickey.bin
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/requirements.txt
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/updatefile.img
pip install -r requirements.txt
mkdir -p export