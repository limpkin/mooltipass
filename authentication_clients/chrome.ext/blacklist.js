// Saves options to chrome.storage
function save_options() {
  var mpTextBox = document.getElementById('mpBlacklist').value;
  var mpBlacklist=mpTextBox.split('\n');
  mpBlacklist.sort();
  chrome.storage.sync.set({
    mpBlacklist: mpBlacklist
  }, function() {
    // Update status to let user know options were saved.
    var status = document.getElementById('status');
    if (chrome.runtime.lastError){
	   status.textContent='ERROR: ' + chrome.runtime.lastError.message;
    } else {
	status.textContent = 'Mooltipass Blacklist Updated.';
    }
    document.getElementById('mpBlacklist').value = mpBlacklist.join('\n');
    setTimeout(function() {
      status.textContent = '';
    }, 750);
  });
}

// Restores select box and checkbox state using the preferences
// stored in chrome.storage.
function restore_options() {
  chrome.storage.sync.get({
    mpBlacklist: []
  }, function(items) {
		document.getElementById('mpBlacklist').value = items.mpBlacklist.join('\n');
  });
}
document.addEventListener('DOMContentLoaded', restore_options);
document.getElementById('save').addEventListener('click', save_options);