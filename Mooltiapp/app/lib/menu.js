'use strict'

const isDev = (require('electron-is-dev') || global.appSettings.debug)
const { app, BrowserWindow } = require('electron')
const ipc = require('electron').ipcMain

function sendAction (action) {
  const win = BrowserWindow.getFocusedWindow()
  if (process.platform === 'darwin') {
    win.restore()
  }
  win.webContents.send(action)
}

const editSubmenu = [
  {
    role: 'undo'
  },
  {
    role: 'redo'
  },
  {
    type: 'separator'
  },
  {
    role: 'cut'
  },
  {
    role: 'copy'
  },
  {
    role: 'paste'
  },
  {
    role: 'selectall'
  }
]

// Need to be edited with Dev Tools items
var viewSubmenu = [
  {
    label: 'Back',
    accelerator: 'CmdOrCtrl+B',
    click: function (item, focusedWindow) {
      if (focusedWindow) {
        focusedWindow.webContents.goBack()
      }
    }
  },
  {
    label: 'Reload',
    accelerator: 'CmdOrCtrl+R',
    click: function (item, focusedWindow) {
      if (focusedWindow) {
        focusedWindow.reload()
      }
    }
  },
  {
    type: 'separator'
  },
  {
    role: 'togglefullscreen'
  },
  {
    label: 'Increase Text Size',
    id: 'zoom-in',
    accelerator: 'CmdOrCtrl+Plus',
    enabled: false,
    click () {
      sendAction('zoom-in')
    }
  },
  {
    label: 'Decrease Text Size',
    id: 'zoom-out',
    accelerator: 'CmdOrCtrl+-',
    enabled: false,
    click () {
      sendAction('zoom-out')
    }
  },
  {
    label: 'Reset Text Size',
    id: 'zoom-actual',
    accelerator: 'CmdOrCtrl+0',
    enabled: false,
    click () {
      sendAction('zoom-actual')
    }
  }
]

const helpSubmenu = [
  {
    label: 'Info',
    click: () => {
      ipc.emit('open-info-window')
    }
  }
]

const darwinTemplate = [
  {
    label: app.getName(),
    submenu: [
      {
        role: 'about'
      },
      {
        type: 'separator'
      },
      {
        role: 'services',
        submenu: []
      },
      {
        type: 'separator'
      },
      {
        role: 'hide'
      },
      {
        role: 'hideothers'
      },
      {
        role: 'unhide'
      },
      {
        type: 'separator'
      },
      {
        role: 'quit'
      }
    ]
  },
  {
    label: 'Edit',
    submenu: editSubmenu
  },
  {
    label: 'View',
    id: 'view',
    submenu: viewSubmenu
  },
  {
    label: 'Window',
    role: 'window',
    submenu: [
      {
        role: 'minimize'
      },
      {
        role: 'close'
      },
      {
        type: 'separator'
      },
      {
        role: 'front'
      }
    ]
  },
  {
    label: 'Help',
    role: 'help',
    submenu: helpSubmenu
  }
]

const otherTemplate = [
  {
    label: 'File',
    submenu: [
      {
        role: 'quit'
      }
    ]
  },
  {
    label: 'Edit',
    submenu: editSubmenu
  },
  {
    label: 'View',
    id: 'view',
    submenu: viewSubmenu
  },
  {
    label: 'Help',
    role: 'help',
    submenu: helpSubmenu
  }
]

// Show Dev Tools menu if running in development
if (isDev) {
  viewSubmenu.push({
    type: 'separator'
  })
  viewSubmenu.push(
    {
      label: 'Toggle Developer Tools',
      accelerator: process.platform === 'darwin' ? 'Alt+Command+I' : 'Ctrl+Shift+I',
      click: function (item, focusedWindow) {
        if (focusedWindow) {
          focusedWindow.webContents.toggleDevTools()
        }
      }
    }
  )
}

module.exports = process.platform === 'darwin' ? darwinTemplate : otherTemplate
