$(function(){
  // Load contributor list
  $("#contributor-list").each(function () {
    $.getJSON("https://api.github.com/repos/limpkin/mooltipass/contributors", function (contributors) {
      $("#contributor-list").html("");
      for (_contributor in contributors) {
        var contributor = contributors[_contributor];

        var p = $('<a class="column small-4 contributor" target="_blank" id="contributor-' + _contributor.toString() + '">');
        $("#contributor-list").append(p);

        var xhr = new XMLHttpRequest();
        xhr.open('GET', contributor.avatar_url, true);
        xhr.contributor = contributor;
        xhr.contributor_id = _contributor.toString();
        xhr.responseType = 'blob';
        xhr.onload = function(e) {
          var img = document.createElement('img');
          img.src = window.URL.createObjectURL(this.response);
          var p = $('#contributor-' + this.contributor_id);
          p.attr("href", this.contributor.html_url);
          p.append(img);
          p.append(this.contributor.login);
        };
        xhr.send();        
      }
    });
  });
});