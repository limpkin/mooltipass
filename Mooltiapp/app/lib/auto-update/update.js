'use strict'

const os = require('os')
const platform = os.platform() + '_' + os.arch()

const { autoUpdater } = require('electron')

module.exports = function update (options) {
  if (!options.url) {
    console.info('Automatic updates disabled')
    return
  }

  var updaterFeedUrl = options.url + platform + '/' + options.version
  if (os.platform() === 'win32') {
    updaterFeedUrl += '/RELEASES'
  }

  console.info('Running version %s on platform %s', options.version, platform)

  try {
    // Don't try to update on development
    if (!process.execPath.match(/[\\\/]electron-prebuilt/)) {
      console.info('Checking for updates at %s', updaterFeedUrl)
      autoUpdater.setFeedURL(updaterFeedUrl)
      autoUpdater.checkForUpdates()
    }
  } catch (e) {
    console.error(e.message)
    throw e
  }

  autoUpdater.on('error', (e) => {
    console.error(e.message)
  })

  autoUpdater.on('checking-for-update', () => {
    console.info('Checking for update...')
  })

  autoUpdater.on('update-available', () => {
    console.info('Found available update!')
  })

  autoUpdater.on('update-not-available', () => {
    console.info('There are no updates available.')
  })

  autoUpdater.on('update-downloaded', () => {
    console.info('Update package downloaded')
    require('electron').ipcMain.emit('update-downloaded', autoUpdater)
  })
}
