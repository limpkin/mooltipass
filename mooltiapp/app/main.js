'use strict'

const electron = require('electron')
const app = electron.app
const Tray = electron.Tray;
const dialog = require('electron').dialog
const ipc = require('electron').ipcMain
const path = require('path')
const pjson = require('./package.json')
const _ = require('lodash')
const windowStateKeeper = require('electron-window-state');

const Menu = require('electron').Menu; 

// Use system log facility, should work on Windows too
require('./lib/log')(pjson.productName || 'SkelEktron');


// Manage unhandled exceptions as early as possible
process.on('uncaughtException', (e) => {
  console.error(`Caught unhandled exception: ${e}`)
  dialog.showErrorBox('Caught unhandled exception', e.message || 'Unknown error message')
  app.quit()
})


// Load build target configuration file
try {
  var config = require('./config.json')
  _.merge(pjson.config, config)
} catch (e) {
  console.warn('No config file loaded, using defaults')
}

const isDev = (require('electron-is-dev') || pjson.config.debug)
global.appSettings = pjson.config

if (isDev) {
  console.info('Running in development')
} else {
  console.info('Running in production')
}

console.debug(JSON.stringify(pjson.config))

// Adds debug features like hotkeys for triggering dev tools and reload
// (disabled in production, unless the menu item is displayed)
require('electron-debug')({
  enabled: pjson.config.debug || isDev || false
})

// Prevent window being garbage collected
let mainWindow

app.setName(pjson.productName || 'Mooltipass')

function initialize () {
  var shouldQuit = makeSingleInstance()
  if (shouldQuit) return app.quit()

  function onClosed () {
    // Dereference used windows
    // for multiple windows store them in an array
    mainWindow = null;
  }

  function createMainWindow () {
    // Load the previous window state with fallback to defaults
    let mainWindowState = windowStateKeeper({
      defaultWidth: 820,
      defaultHeight: 620,
      resizable: false
    })

    const win = new electron.BrowserWindow({
      'width': mainWindowState.width,
      'height': mainWindowState.height,
      'x': mainWindowState.x,
      'y': mainWindowState.y,
      'title': app.getName(),
      'icon': path.join(__dirname, '/chrome_app/images/icons/AppIcon_128.png'),
      'show': false, // Hide your application until your page has loaded
      'webPreferences': {
        'nodeIntegration': pjson.config.nodeIntegration || true, // Disabling node integration allows to use libraries such as jQuery/React, etc
        'preload': path.resolve(path.join(__dirname, 'preload.js'))
      }
    });

    // works only on Windows and Linux
    win.setMenu(null);
    win.setResizable(false);
    win.setSize(820, 620);

    // Let us register listeners on the window, so we can update the state
    // automatically (the listeners will be removed when the window is closed)
    // and restore the maximized or full screen state
    mainWindowState.manage(win);

    // EXPERIMENTAL: Minimize to tray
    win.on('minimize',function(event){
      event.preventDefault();
      win.hide();
    });

    // EXPERIMENTAL: Minimize to tray
    win.on('close', function (event) {
      if( !app.isQuiting ) {
        event.preventDefault()
        win.hide();
        app.dock.hide();
      }
      return false;
    });

    win.loadURL(`file://${__dirname}/${pjson.config.url}`, {})

    win.on('closed', onClosed)

    // Then, when everything is loaded, show the window and focus it so it pops up for the user
    // Yon can also use: win.webContents.on('did-finish-load')
    win.on('ready-to-show', () => {
      win.show()
      win.focus()
    })

    win.on('unresponsive', function () {
      // In the real world you should display a box and do something
      console.warn('The windows is not responding')
    })

    win.webContents.on('did-fail-load', (error, errorCode, errorDescription) => {
      var errorMessage

      if (errorCode === -105) {
        errorMessage = errorDescription || '[Connection Error] The host name could not be resolved, check your network connection'
        console.log(errorMessage)
      } else {
        errorMessage = errorDescription || 'Unknown error'
      }

      error.sender.loadURL(`file://${__dirname}/error.html`)
      win.webContents.on('did-finish-load', () => {
        win.webContents.send('app-error', errorMessage)
      })
    })

    win.webContents.on('crashed', () => {
      // In the real world you should display a box and do something
      console.error('The browser window has just crashed')
    })

    return win
  }

  app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
      app.quit()
    }
  })

  app.on('activate', () => {
    setTimeout( function() {
      if (!mainWindow) {
        mainWindow = createMainWindow()
      }
    }.bind(this),500);
  })

  let tray = null;
  app.on('ready', () => {
    mainWindow = createMainWindow();

    tray = new Tray( path.join(__dirname, '../chrome_app/images/icons/icon_cross_16.png') );

    var contextMenu = Menu.buildFromTemplate([
        { label: 'Show App', click:  function(){
            mainWindow.show();
            app.dock.show();
        } },
        { label: 'Quit', click:  function(){
            app.isQuiting = true;
            app.quit();
        } }
    ]);

    tray.setToolTip('Open or Quit MooltiApp');
    tray.setContextMenu(contextMenu);

    // Manage automatic updates
    try {
      require('./lib/auto-update/update')({
        url: (pjson.config.update) ? pjson.config.update.url || false : false,
        version: app.getVersion()
      })
      ipc.on('update-downloaded', (autoUpdater) => {
        // Elegant solution: display unobtrusive notification messages
        mainWindow.webContents.send('update-downloaded')
        ipc.on('update-and-restart', () => {
          autoUpdater.quitAndInstall()
        })

        // Basic solution: display a message box to the user
        // var updateNow = dialog.showMessageBox(mainWindow, {
        //   type: 'question',
        //   buttons: ['Yes', 'No'],
        //   defaultId: 0,
        //   cancelId: 1,
        //   title: 'Update available',
        //   message: 'There is an update available, do you want to restart and install it now?'
        // })
        //
        // if (updateNow === 0) {
        //   autoUpdater.quitAndInstall()
        // }
      })
    } catch (e) {
      console.error(e.message)
      dialog.showErrorBox('Update Error', e.message)
    }
  })

  app.on('will-quit', () => { })
}

// Make this app a single instance app.
//
// The main window will be restored and focused instead of a second window
// opened when a person attempts to launch a second instance.
//
// Returns true if the current version of the app should quit instead of
// launching.
function makeSingleInstance () {
  return app.makeSingleInstance(() => {
    if (mainWindow) {
      if (mainWindow.isMinimized()) mainWindow.restore()
      mainWindow.focus()
    }
  })
}

// Manage Squirrel startup event (Windows)
require('./lib/auto-update/startup')(initialize)

