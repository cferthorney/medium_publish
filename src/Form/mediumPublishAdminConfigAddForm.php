<?php

namespace \Drupal\medium_publish\Form;

class mediumPublishAdminConfigAddForm extends Form {

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
}