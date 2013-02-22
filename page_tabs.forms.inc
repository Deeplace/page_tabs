<?php

function page_tabs_weight_form($form, $form_state) {
  $languages = language_list();

  $query = db_select('page_tabs_weight', 'tw');
  $query->leftJoin('node', 'n', 'tw.vid = n.vid');
  $query->leftJoin('page_specials_tabs', 'st', 'tw.spec_id = st.id');
  $query->fields('tw')
    ->fields('n', array('title', 'language'));
  $query->addField('st', 'title', 'spec_title');
  $query->orderBy('tw.weight', 'ASC');

  $result = $query->execute();

  $form['#tree'] = TRUE;

  foreach($result as $obj) {
    if($obj->type == 'special') {
      $obj->title = $obj->spec_title;
    }
    $form[$obj->id]['#node_data'] = $obj;
    $form[$obj->id]['name'] = array('#markup' => $obj->title);
    $form[$obj->id]['language'] = array('#markup' => (!empty($languages[$obj->language]) ? $languages[$obj->language]->name : t('Language neutral')));
    $form[$obj->id]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight for tab @title', array('@title' => $obj->title)),
      '#title_display' => 'invisible',
      '#delta' => 10,
      '#default_value' => $obj->weight,
    );
    if($obj->type !== 'special') {
      $form[$obj->id]['operations'] = array('#markup' => l(t('Change'), 'node/' . $obj->nid . '/edit'));
    }
    else {
      $form[$obj->id]['operations'] = array('#markup' => l(t('Change'), 'admin/config/system/page-special-tabs'));
    }
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}

function page_tabs_weight_form_submit($form, &$form_state) {
  foreach ($form_state['values'] as $k => $v) {
    if(is_numeric($k)) {
      if($form[$k]['#node_data']->weight != $form_state['values'][$k]['weight']) {
        db_update('page_tabs_weight')
          ->fields(array('weight' => $form_state['values'][$k]['weight']))
          ->condition('id', $form[$k]['#node_data']->id)
          ->execute();
      }
    }
  }
  drupal_set_message(t('The configuration options have been saved.'));
}

function theme_page_tabs_weight_form($variables) {
  $form = $variables['form'];

  $rows = array();

  foreach (element_children($form) as $key) {
    if (isset($form[$key]['name'])) {
      $tab = &$form[$key];

      $row = array();
      $row[] = drupal_render($tab['name']);
      if (isset($tab['weight'])) {
        $tab['weight']['#attributes']['class'] = array('tabs-weight');
        $row[] = drupal_render($tab['weight']);
      }
      $row[] = drupal_render($tab['language']);
      $row[] = drupal_render($tab['operations']);
      $rows[] = array('data' => $row, 'class' => array('draggable'));
    }
  }

  $header = array(t('Name'));
  if (isset($form['actions'])) {
    $header[] = t('Weight');
    drupal_add_tabledrag('tabs', 'order', 'sibling', 'tabs-weight');
  }

  $header = array(
    array('data' => t('Title')),
    array('data' => t('Weight')),
    array('data' => t('Language')),
    array('data' => t('Operations')),
  );

  return theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('No tabs available.'), 'attributes' => array('id' => 'tabs'))) . drupal_render_children($form);
}

function page_tabs_special_form($form, $form_state) {
  $custom_pages = module_invoke_all('page_tabs_custom_pages_info');
  $parsed_custom_pages = array();
  if(!empty($custom_pages) && is_array($custom_pages['unique_name'])) {
    foreach($custom_pages['unique_name'] as $k => $v) {
      $parsed_custom_pages[$v] = array(
        'unique_name' => $v,
        'title' => $custom_pages['title'][$k]
      );
    }
  }

  $result = db_select('page_specials_tabs', 'pst')
    ->fields('pst')
    ->execute();
  $tabs_exist_data = array();
  foreach($result as $obj) {
    $tabs_exist_data[$obj->unique_name] = $obj;
  }

  $form['custom_page_tabs'] = array(
    '#tree' => TRUE
  );


  foreach($parsed_custom_pages as $k => $v) {
    $form['custom_page_tabs'][$v['unique_name']]['wrapper-' . $v['unique_name']] = array(
      '#type' => 'fieldset',
      '#title' => $v['unique_name'] . ' ' . t('tab settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    $form['custom_page_tabs'][$v['unique_name']]['wrapper-' . $v['unique_name']]['tab_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Tab title'),
      '#default_value' => (!empty($tabs_exist_data[$v['unique_name']]) ? $tabs_exist_data[$v['unique_name']]->title : $v['title']),
      '#required' => TRUE,
    );

//TODO FIXME
    $vocabulary = taxonomy_vocabulary_machine_name_load('page_tabs');
    $tree = taxonomy_get_tree($vocabulary->vid, 0, 1);
    $options[''] = t('--------');
    foreach($tree as $term) {
      $options[$term->tid] = $term->name;
    }
    $form['custom_page_tabs'][$v['unique_name']]['wrapper-' . $v['unique_name']]['category'] = array(
      '#type' => 'select',
      '#title' => t('Tab category'),
      '#options' => $options,
      '#default_value' => (!empty($tabs_exist_data[$v['unique_name']]) ? $tabs_exist_data[$v['unique_name']]->category : ''),
      '#required' => TRUE,
    );

    $form['custom_page_tabs'][$v['unique_name']]['wrapper-' . $v['unique_name']]['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable @name tab?', array('@name' => $v['unique_name'])),
      '#optsion' => array(
        '1' => t('Yes'),
        '0' => t('No')
      ),
      '#default_value' => (!empty($tabs_exist_data[$v['unique_name']]) ? $tabs_exist_data[$v['unique_name']]->enabled : '')
    );

    $form['custom_page_tabs'][$v['unique_name']]['wrapper-' . $v['unique_name']]['unique_name'] = array(
      '#type' => 'value',
      '#value' => $v['unique_name']
    );
  }


  $form['#tabs_exist_data'] = $tabs_exist_data;

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function page_tabs_special_form_submit($form, $form_state) {
  $tabs_exist_data = $form['#tabs_exist_data'];
  foreach ($form_state['values']['custom_page_tabs'] as $k => $v) {
    if(!empty($tabs_exist_data[$v['wrapper-' . $k]['unique_name']])) {
      db_update('page_specials_tabs')
        ->fields(
          array(
            'title' => $v['wrapper-' . $k]['tab_title'],
            'enabled' => $v['wrapper-' . $k]['enabled'],
            'category' => $v['wrapper-' . $k]['category'],
          )
        )
        ->condition('unique_name', $k)
        ->execute();
    }
    else {
      $id = db_insert('page_specials_tabs')
        ->fields(
          array(
            'title' => $v['wrapper-' . $k]['tab_title'],
            'enabled' => $v['wrapper-' . $k]['enabled'],
            'unique_name' => $v['wrapper-' . $k]['unique_name'],
            'category' => $v['wrapper-' . $k]['category'],
          )
        )
        ->execute();

      db_insert('page_tabs_weight')
        ->fields(
          array(
            'spec_id' => $id,
            'type' => 'special'
          )
        )
      ->execute();
    }
  }
  drupal_set_message(t('The configuration options have been saved.'));
}