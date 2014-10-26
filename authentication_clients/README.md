Installation process
--------------------
Dedicated tutorial <a href="http://tinyurl.com/chromemp">here</a>.

udev rule
---------
KERNEL=="hidraw*", SUBSYSTEM=="hidraw", ATTRS{idVendor}=="16d0", ATTRS{idProduct}=="09a0", MODE="0664", GROUP="plugdev"

Then adding your user to plugdev with:
gpasswd -a user plugdev