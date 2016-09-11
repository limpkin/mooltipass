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

function _wrapWithArrayIfNeeded(input) {
    var output = input || [];

    if(output.constructor == Array) {
        // Is an array
        return output;
    }

    // Everything else than an array --> wrap with array
    return [output];
}

function applyCallback(_callbackFunction, _callbackParameters, _ownParameters) {
    if(_callbackFunction) {
        var args = _wrapWithArrayIfNeeded(_ownParameters);
        args = args.concat( _wrapWithArrayIfNeeded(_callbackParameters) );
        _callbackFunction.apply(this, args);
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
