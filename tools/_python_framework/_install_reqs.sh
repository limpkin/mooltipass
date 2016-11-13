#!/bin/bash
sudo apt-get update
sudo apt-get -y --force-yes install python python-pip python-crypto python-dev libgmp3-dev build-essential python-numpy python-matplotlib python-wheel < /dev/null
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
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/mooltipass_mass_prod_init_proc.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/png_labels.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/generate_prog_file.py
wget -N https://raw.githubusercontent.com/limpkin/mooltipass/master/tools/_python_framework/Helvetica.ttf
sudo pip install -r requirements.txt
sudo pip install seccure
sudo pip install https://github.com/pklaus/brother_ql/archive/bef9cea.zip
sudo pip install --upgrade --no-deps https://github.com/pklaus/brother_ql/archive/bef9cea.zip
mkdir -p export