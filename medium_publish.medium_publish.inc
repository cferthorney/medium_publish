<?php
/**
 * @file
 * Contains our own hooks that we call.
 */

 /**
  * Implements hook_medium_publish_config_default().
  */
function medium_publish_medium_publish_config_default() {
  if (!variable_get('medium_publish_load_defaults', TRUE)) {
    return;
  }

  // Skip example if content type is missing.
  if (!node_type_load('article')) {
    return;
  }

  $configs = array();

  $config = new stdClass();
  $config->instance = 'article';
  $config->api_version = 1;
  $config->disabled = TRUE;
  $config->config = array(
    'title' => '[node:title]',
    'content' => '[node:body]',
    'format' => 'html',
    'status' => 'public',
    'license' => 'all-rights-reserved',
    'canonical' => '[node:url:absolute]',
    'tags' => '[node:field_tags]',
  );
  $configs[$config->instance] = $config;

  return $configs;
}
