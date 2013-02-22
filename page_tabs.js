Drupal.behaviors.page_tabs = {
  attach: function(context, settings) {
    var actual_language_links = [];

    var active_tab = getParameterByName('active_tab');
    tab_active_index = 0;

    var hash = window.location.hash;
    if(hash.indexOf('active_tab:') != -1) {
      active_tab = hash.substr(12);
    }

    if(active_tab.length > 0) {
      var active_li = jQuery('.page_tabs li:has(a[href="#' + active_tab + '"])', context);
      fix_block_language("#" + active_tab);
      tab_active_index = active_li.parents('ul').find('li').index(active_li);
      if(tab_active_index < 0) {
        tab_active_index = 0;
      }
    }
    jQuery(".page_tabs", context).tabs({ selected: tab_active_index });


    // when the tab is selected update the url with the hash
    jQuery(".page_tabs", context).bind("tabsselect", function(event, ui) {
      window.location.hash = 'active_tab:' + ui.tab.hash.substr(1);
      fix_block_language(ui.tab.hash);
    });

    function fix_block_language(new_hash) {
      jQuery('#block-locale-language .language-switcher-locale-url a').each(function(index){
        if(!new_hash.length) return;
        var href = jQuery(this).attr('href');
        var inner_hash = new_hash;
        if(actual_language_links[index]){
          href = actual_language_links[index];
        }
        else {
          actual_language_links[index] = href;
        }

        inner_hash = inner_hash.substr(1);
        var pos = href.indexOf('?');
        if(pos != -1) {
          href = href.substr(0, pos) + '&active_tab=' + inner_hash;
        }
        else {
          href = href + '?active_tab=' + inner_hash;
        }
        jQuery(this).attr('href', href);
      });
    }

    function getParameterByName(name) {
      name = name.replace(/[\[]/, "\\\[").replace(/[\]]/, "\\\]");
      var regexS = "[\\?&]" + name + "=([^&#]*)";
      var regex = new RegExp(regexS);
      var results = regex.exec(window.location.search);
      if(results == null)
        return "";
      else
        return decodeURIComponent(results[1].replace(/\+/g, " "));
    }
  }
}

