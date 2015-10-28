/**
 * Capitalize first letter without changing the others
 * Author: Steve Harrison, http://stackoverflow.com/a/1026087/568270
 * @param string
 * @returns {string}
 */
function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
}

function contains(array, obj) {
    for (var i = 0; i < array.length; i++) {
        if (array[i] === obj) {
            return true;
        }
    }
    return false;
}

function mergeObjects(sourceObject, destinationObject) {
    for (var key in sourceObject) {
        destinationObject[key] = sourceObject[key];
    }
}

applyCallback = function(callbackFunction, callbackParameters, ownParameters) {
    if(callbackFunction) {
        var args = ownParameters || [];
        args = args.concat(callbackParameters || []);
        callbackFunction.apply(this, args);
    }
};

var _debug = false;
function logging() {
    if(_debug !== true) {
        return;
    }

    var values = [];
    for(var i = 1; i < arguments.length; i++) {
        values.push(arguments[i]);
    }

    if(arguments[0] == 'log') {
        console.log(values);
    }
    else if(arguments[0] == 'warn') {
        console.warn(values);
    }
    else if(arguments[0] == 'error') {
        console.error(values);
    }
}
