# Mooltiapp

Building

For Mac:

npm run build:osx


* Note: 

Permission error on MAC:

rm -Rf ~/.electron/
sudo npm install -g electron-prebuilt --unsafe-perm=true --allow-root

I also ran with problems building websocket, my solution was: inside app/node_modules/websocket/, edited package.json install script to "install": "node-gyp rebuild".