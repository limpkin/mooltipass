/* Contains all methods which are accessed by the html app interface */
var mooltipass = mooltipass || {};
mooltipass.app = mooltipass.app || {};

mooltipass.app.get_password = function(app, user, callback) {
  var password = user + "123";
  var time = 2;
  console.warn("mooltipass.app.get_password not implemented, calling callback after " + time + "s with password '" + password + "'.");

  setTimeout(function(){
    callback(password);  
  }, time * 1000);

};