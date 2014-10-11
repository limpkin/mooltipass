/* CDDL HEADER START
 * 
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/* Copyright (c) 2014 Bjorn Wielens. All rights reserved. */

/*!      \file blacklist.js
 *        \brief        Mooltipass Chrome Authentication plugin options page
 *        Created: 11/09/2014
 *        Author: Bjorn Wielens
 *
 *        Allows user to maintain a blacklist of sites that do not interact with the Mooltipass.
 * 
 */


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

// Restores contents of text area so user can edit.
function restore_options() {
  chrome.storage.sync.get({
    mpBlacklist: []
  }, function(items) {
		document.getElementById('mpBlacklist').value = items.mpBlacklist.join('\n');
  });
}
document.addEventListener('DOMContentLoaded', restore_options);
document.getElementById('save').addEventListener('click', save_options);