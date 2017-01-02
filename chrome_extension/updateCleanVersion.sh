#!/bin/bash

# Remove previous files

rm -Rf CleanVersion/*

# Inject Scripts

cp mooltipass-content.js CleanVersion/mooltipass-content.js
cp mooltipass-content.css CleanVersion/mooltipass-content.css
cp manifest.json CleanVersion/
cp mooltipass-pre-jquery.js CleanVersion/

cp -Rf vendor CleanVersion/
cp -Rf popups CleanVersion/
cp -Rf css CleanVersion/
cp -Rf options CleanVersion/
cp -Rf background CleanVersion/
cp -Rf images CleanVersion/
