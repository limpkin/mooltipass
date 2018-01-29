/*
 * Icon to indicate login field.
 * 
 * @param type (small | big) {String}
 * @param settings {Object}
 */

window.data = JSON.parse(decodeURIComponent(window.location.search.slice(1)))

$(function() {
	$('.icon')
		.addClass('icon__' + data.type)
		.css('background-image', 'url(' + data.settings['extension-base'] + 'images/icon_login_128.png)')
});