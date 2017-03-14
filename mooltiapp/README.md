# Mooltiapp

Starting / Building the App - Windows
-------------------------------------
- delete the chrome_app file inside the app folder
- copy the chrome app folder from the mooltipass root repository to the app folder
- (from a shell with admin rights) npm install -g windows-build-tools
- (from a shell with admin rights) npm install -g electron
- (from a shell with admin rights) npm install -g node-gyp
- (standard shell, inside the mooltiapp folder) npm install

To run the app:

- npm start

To build the app (NOT WORKING AT THE MOMENT):

- inside app/node_modules/websocket/, edited package.json install script to "install": "node-gyp rebuild".
- npm run build:win


Starting / Building the App - MAC
---------------------------------

- npm install -g electron
- npm install
- inside app/node_modules/websocket/, edited package.json install script to "install": "node-gyp rebuild".
- "npm run build:osx" or "npm start"

Notes
-----
Permission error on MAC:

rm -Rf ~/.electron/
sudo npm install -g electron-prebuilt --unsafe-perm=true --allow-root

I also ran with problems building websocket, my solution was: inside app/node_modules/websocket/, edited package.json install script to "install": "node-gyp rebuild".
