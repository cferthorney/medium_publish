<?php
/**
 * @file
 * API documentation for Medium Publish.
 */

 /**
  * Provides a default configuration for Medium exports.
  *
  * This hook allows modules to provide their own setups for mapping content
  * type fields to Medium export with tokens.
  *
  * This hook should be placed in MODULENAME.medium_publish.inc and it will be
  * auto-loaded. MODULENAME.medium_publish.inc *must* be in the same directory
  * as the .module file which *must* also contain an implementation of
  * hook_ctools_plugin_api, preferably with the same code as found in
  * medium_publish_ctools_plugin_api().
  *
  * The $config->disabled boolean attribute indicates whether the
  * instance should be enabled (FALSE) or disabled (TRUE) by default.
  *
  * @return array
  *   An associative array containing the structures of Medium Publish instances
  *   keyed by the Medium Publish config name.
  *
  * @see medium_publish_medium_publish_config_default()
  * @see medium_ctools_plugin_api()
  */
function hook_medium_publish_config_default() {
  $configs = array();

  $config = new stdClass();
  // Instance name is Node bundle machine name:
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

/**
 * Allow modules to alter configuration before save.
 *
 * This hook is invoked for both insert and update.
 */
function hook_medium_publish_config_presave() {

}

/**
 * Allow modules to alter configuration after insert.
 */
function hook_medium_publish_config_insert() {

}

/**
 * Allow modules to alter configuration after update.
 */
function hook_medium_publish_config_update() {

}

/**
 * Allow modules to alter configuration after deletion.
 */
function hook_medium_publish_config_delete() {

}
