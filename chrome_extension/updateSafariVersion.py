import json, plistlib;

with open("manifest.json") as dataFile:    
    data = json.load(dataFile)

version = data["version"];

infoFile = "Mooltipass.safariextension/Info.plist";
plist = plistlib.readPlist(infoFile);
plist["CFBundleShortVersionString"] = ".".join(version.split(".")[:2]);
plist["CFBundleVersion"] = version.split(".")[2];
plistlib.writePlist(plist, infoFile);
