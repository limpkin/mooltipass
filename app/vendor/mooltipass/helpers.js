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