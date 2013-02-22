/**
 * implementation of hook_page_tabs_custom_pages_info
 */

function page_tabs_page_tabs_custom_pages_info() {
  return array(
    'unique_name' => 'custom test page',
    'title' => 'Custom test page'
  );
}

/**
 * implementation of hook_page_tabs_custom_pages_view
 */
function page_tabs_page_tabs_custom_pages_view($unique_name) {
  if($unique_name == 'custom test page') {
    return array(
      'content' => 'test content',
      'link_class' => ' aclass1 aclass2',
      'item_class' => 'li class'
    );
  }
}