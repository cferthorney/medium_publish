<?php

namespace \Drupal\medium_publish\Form;

class mediumPublishAdminSettingsForm extends Form {


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
}