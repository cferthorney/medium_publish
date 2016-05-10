<?php
/**
 * @file
 * Medium.admin.inc.
 */

/**
 * Menu callback for config overview.
 */
function medium_publish_config_overview() {
  ctools_include('export');

  $configs = ctools_export_crud_load_all('medium_publish_config');

  $client_id = variable_get('medium_publish_client_id', FALSE);
  $client_secret = variable_get('medium_publish_client_secret', FALSE);
  if (!($client_id && $client_secret)) {
    drupal_set_message(t('Medium integration must be configured for this to work. Please go to <a href="@link">settings</a> and configure it.', array(
      '@link' => url('admin/config/content/medium/settings'),
    )), 'warning');
  }

  $rows = array();
  foreach ($configs as $config) {
    $row = array();

    // Style disabled configurations differently.
    if (!empty($config->disabled)) {
      $row['class'][] = 'disabled';
    }

    $row['data']['details'] = drupal_ucfirst($config->instance);

    $operations = array();
    if (medium_publish_config_access('disable', $config)) {
      $operations['edit'] = array(
        'title' => ($config->export_type & EXPORT_IN_DATABASE) ? t('Edit') : t('Override'),
        'href' => 'admin/config/content/medium/config/' . $config->instance,
      );
    }
    if (medium_publish_config_access('enable', $config)) {
      $operations['enable'] = array(
        'title' => t('Enable'),
        'href' => 'admin/config/content/medium/config/' . $config->instance . '/enable',
        'query' => array(
          'token' => drupal_get_token('enable-' . $config->instance),
        ) + drupal_get_destination(),
      );
    }
    if (medium_publish_config_access('disable', $config)) {
      $operations['disable'] = array(
        'title' => t('Disable'),
        'href' => 'admin/config/content/medium/config/' . $config->instance . '/disable',
        'query' => array(
          'token' => drupal_get_token('disable-' . $config->instance),
        ) + drupal_get_destination(),
      );
    }

    if (medium_publish_config_access('delete', $config)) {
      $operations['delete'] = array(
        'title' => t('Delete'),
        'href' => 'admin/config/content/medium/config/' . $config->instance . '/delete',
      );
    }
    $row['data']['operations'] = array(
      'data' => array(
        '#theme' => 'links',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline')),
      ),
    );

    $rows[$config->instance] = $row;
  }

  $build['config_table'] = array(
    '#theme' => 'table',
    '#header' => array(
      'type' => t('Type'),
      'operations' => t('Operations'),
    ),
    '#rows' => $rows,
    '#empty' => t('No content type configs yet'),
    '#suffix' => '<p>' . t('Only content types with a enabled config defined will have the option to publish to Medium.') . '</p>',
  );

  return $build;
}

/**
 * Settings form.
 */
function medium_publish_admin_settings_form() {
  global $base_url;

  $form = array();

  $form['medium_callback'] = array(
    '#type' => 'item',
    '#title' => t('Redirect URL'),
    '#markup' => $base_url . '/api/medium',
    '#description' => t('Provide this when registering your application at Medium.com'),
  );

  $form['medium_publish_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Client ID'),
    '#default_value' => $client_id = variable_get('medium_publish_client_id', ''),
    '#description' => t('Register your application to get a client id: !link', array(
      '!link' => l(t('Medium.com'), 'https://medium.com/me/applications'),
    )),
  );
  $form['medium_publish_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Client Secret'),
    '#default_value' => $client_secret = variable_get('medium_publish_client_secret', ''),
  );

  return system_settings_form($form);
}

/**
 * Form for adding new config.
 */
function medium_publish_config_add_form($form, &$form_state) {
  $form['instance'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#description' => t('Select content type to override global settings.'),
    '#options' => _medium_publish_config_instance_get_available_options(),
    '#required' => TRUE,
  );
  $form['config'] = array(
    '#type' => 'value',
    '#value' => array(),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Add and configure'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => isset($_GET['destination']) ? $_GET['destination'] : 'admin/config/content/medium',
  );

  return $form;
}

/**
 * Submit handler for adding configs.
 */
function medium_publish_config_add_form_submit($form, &$form_state) {
  form_state_values_clean($form_state);
  $config = (object) $form_state['values'];
  medium_publish_config_save($config);
  $form_state['redirect'] = 'admin/config/content/medium/config/' . $config->instance;
}

/**
 * Edit form.
 */
function medium_publish_config_edit_form($form, &$form_state, $config) {
  $form['pid'] = array(
    '#type' => 'value',
    '#value' => !empty($config->pid) ? $config->pid : NULL,
  );
  $form['instance'] = array(
    '#type' => 'value',
    '#value' => $config->instance,
  );

  $form['medium'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
  );

  $form['medium']['intro_text'] = array(
    '#markup' => '<p>' . t('Configure the medium export.') . '</p>',
    '#weight' => -10,
  );

  $form['medium']['token_tree'] = array(
    '#theme' => 'token_tree',
    '#token_types' => array('node'),
  );
  $form['medium']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => (isset($config->config['title']) ? $config->config['title'] : ''),
    '#required' => TRUE,
  );
  $form['medium']['content'] = array(
    '#type' => 'textfield',
    '#title' => t('Content'),
    '#default_value' => (isset($config->config['content']) ? $config->config['content'] : ''),
    '#required' => TRUE,
    '#description' => t('You can combine multiple fields by adding them in the order you want them to appear.'),
  );
  $form['medium']['format'] = array(
    '#type' => 'select',
    '#title' => t('Select format of posts'),
    '#default_value' => (isset($config->config['format']) ? $config->config['format'] : 'html'),
    '#options' => array(
      'html' => 'HTML (Recommended)',
      'markdown' => 'Markdown',
    ),
    '#required' => TRUE,
    '#description' => t('Only switch this to Markdown if you write your posts in Markdown.'),
  );
  $form['medium']['status'] = array(
    '#type' => 'select',
    '#title' => t('Select format of posts'),
    '#default_value' => (isset($config->config['status']) ? $config->config['status'] : 'public'),
    '#options' => array(
      'public' => 'Public',
      'draft' => 'Draft',
      'unlisted' => 'Unlisted',
    ),
  );
  $form['medium']['license'] = array(
    '#type' => 'select',
    '#title' => t('License'),
    '#default_value' => (isset($config->config['license']) ? $config->config['license'] : 'all-rights-reserved'),
    '#options' => array(
      'all-rights-reserved' => 'all-rights-reserved',
      'cc-40-by' => 'cc-40-by',
      'cc-40-by-sa' => 'cc-40-by-sa',
      'cc-40-by-nd' => 'cc-40-by-nd',
      'cc-40-by-nc' => 'cc-40-by-nc',
      'cc-40-by-nc-nd' => 'cc-40-by-nc-nd',
      'cc-40-by-nc-sa' => 'cc-40-by-nc-sa',
      'cc-40-zero' => 'cc-40-zero',
      'public-domain' => 'public-domain',
    ),
  );
  $form['medium']['canonical'] = array(
    '#type' => 'textfield',
    '#title' => t('Canonical URL'),
    '#default_value' => (isset($config->config['canonical']) ? $config->config['canonical'] : ''),
  );
  $form['medium']['tags'] = array(
    '#type' => 'textfield',
    '#title' => t('Tags'),
    '#default_value' => (isset($config->config['tags']) ? $config->config['tags'] : ''),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => isset($_GET['destination']) ? $_GET['destination'] : 'admin/config/content/medium',
  );

  $form['#submit'][] = 'medium_publish_config_edit_form_submit';

  return $form;
}

/**
 * Submit handler for editing configs.
 */
function medium_publish_config_edit_form_submit($form, &$form_state) {
  // Build the configuration object and save it.
  form_state_values_clean($form_state);
  $config = (object) $form_state['values'];
  $config->config = $config->medium;

  unset($config->medium);
  medium_publish_config_save($config);

  $label = drupal_ucfirst($config->instance);
  drupal_set_message(t('The tokens for @label have been saved.', array('@label' => $label)));

  $form_state['redirect'] = 'admin/config/content/medium';
}

/**
 * Build an FAPI #options array for the instance select field.
 */
function _medium_publish_config_instance_get_available_options() {
  $options = array();
  $content_types = node_type_get_types();

  foreach ($content_types as $content_type => $content_type_info) {
    if (medium_publish_config_load($content_type)) {
      continue;
    }

    $options[$content_type] = $content_type_info->name;
  }

  return $options;
}

/**
 * Enable disabled configs.
 */
function medium_publish_config_enable($config) {
  if (!isset($_GET['token']) || !drupal_valid_token($_GET['token'], 'enable-' . $config->instance)) {
    return MENU_ACCESS_DENIED;
  }

  ctools_export_crud_enable('medium_publish_config', $config);

  $label = drupal_ucfirst($config->instance);
  drupal_set_message(t('The medium export for @label have been enabled.', array('@label' => $label)));
  drupal_goto();
}

/**
 * Disable enabled configs.
 */
function medium_publish_config_disable($config) {
  if (!isset($_GET['token']) || !drupal_valid_token($_GET['token'], 'disable-' . $config->instance)) {
    return MENU_ACCESS_DENIED;
  }

  ctools_export_crud_disable('medium_publish_config', $config);

  $label = drupal_ucfirst($config->instance);
  drupal_set_message(t('The medium export for @label have been disabed.', array('@label' => $label)));
  drupal_goto();
}

/**
 * Form for deleting config.
 */
function medium_publish_config_delete_form($form, &$form_state, $config) {
  $form['pid'] = array('#type' => 'value', '#value' => $config->pid);
  $form['instance'] = array('#type' => 'value', '#value' => $config->instance);

  $label = drupal_ucfirst($config->instance);
  $delete = medium_publish_config_access('delete', $config);
  $title = $delete ? t('Are you sure you want to delete the config for @label?', array('@label' => $label)) : t('Are you sure you want to revert the config for @label?', array('@label' => $label));

  return confirm_form(
    $form,
    $title,
    'admin/config/content/medium',
    t('This action cannot be undone.')
  );
}

/**
 * Submit handler for deleting configs.
 */
function medium_publish_config_delete_form_submit($form, &$form_state) {
  $config = medium_publish_config_load($form_state['values']['instance']);
  medium_publish_config_delete($config);

  $label = drupal_ucfirst($config->instance);
  $delete = medium_publish_config_access('delete', $config);
  $title = $delete ? t('The config for @label have been deleted.', array('@label' => $label)) : t('The config for @label have been reverted.', array('@label' => $label));
  drupal_set_message($title);

  $form_state['redirect'] = 'admin/config/content/medium';
}
