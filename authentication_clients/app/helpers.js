/**
 * Capitalize first letter without changing the others
 * Author: Steve Harrison, http://stackoverflow.com/a/1026087/568270
 * @param string
 * @returns {string}
 */
function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
}