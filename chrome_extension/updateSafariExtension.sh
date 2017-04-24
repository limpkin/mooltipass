#!/bin/bash

# Inject Scripts

cp mooltipass-content.js Mooltipass.safariextension/mooltipass-content.js
cp mooltipass-content.css Mooltipass.safariextension/mooltipass-content.css
cp -Rf options Mooltipass.safariextension/ 
cp -Rf vendor Mooltipass.safariextension/
cp -Rf background Mooltipass.safariextension/
cp -Rf images Mooltipass.safariextension/
cp -Rf css Mooltipass.safariextension/
cp -Rf popups Mooltipass.safariextension/
cp -Rf fonts Mooltipass.safariextension/