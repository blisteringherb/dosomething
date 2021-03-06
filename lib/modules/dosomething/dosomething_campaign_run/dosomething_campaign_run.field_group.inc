<?php
/**
 * @file
 * dosomething_campaign_run.field_group.inc
 */

/**
 * Implements hook_field_group_info().
 */
function dosomething_campaign_run_field_group_info() {
  $export = array();

  $field_group = new stdClass();
  $field_group->disabled = FALSE; /* Edit this to true to make a default field_group disabled initially */
  $field_group->api_version = 1;
  $field_group->identifier = 'group_basic_info|node|campaign_run|form';
  $field_group->group_name = 'group_basic_info';
  $field_group->entity_type = 'node';
  $field_group->bundle = 'campaign_run';
  $field_group->mode = 'form';
  $field_group->parent_name = '';
  $field_group->data = array(
    'label' => 'Basic Info',
    'weight' => '0',
    'children' => array(
      0 => 'field_campaigns',
      1 => 'title',
    ),
    'format_type' => 'fieldset',
    'format_settings' => array(
      'formatter' => 'collapsible',
      'instance_settings' => array(
        'description' => '',
        'classes' => 'group-basic-info field-group-fieldset',
        'required_fields' => 1,
      ),
    ),
  );
  $export['group_basic_info|node|campaign_run|form'] = $field_group;

  $field_group = new stdClass();
  $field_group->disabled = FALSE; /* Edit this to true to make a default field_group disabled initially */
  $field_group->api_version = 1;
  $field_group->identifier = 'group_the_buzz|node|campaign_run|form';
  $field_group->group_name = 'group_the_buzz';
  $field_group->entity_type = 'node';
  $field_group->bundle = 'campaign_run';
  $field_group->mode = 'form';
  $field_group->parent_name = '';
  $field_group->data = array(
    'label' => 'The Buzz',
    'weight' => '2',
    'children' => array(
      0 => 'field_additional_text',
      1 => 'field_additional_text_title',
      2 => 'field_gallery_collection',
    ),
    'format_type' => 'fieldset',
    'format_settings' => array(
      'label' => 'The Buzz',
      'instance_settings' => array(
        'required_fields' => 0,
        'classes' => 'group-what-people-are-saying field-group-fieldset',
        'description' => '',
      ),
      'formatter' => 'collapsed',
    ),
  );
  $export['group_the_buzz|node|campaign_run|form'] = $field_group;

  $field_group = new stdClass();
  $field_group->disabled = FALSE; /* Edit this to true to make a default field_group disabled initially */
  $field_group->api_version = 1;
  $field_group->identifier = 'group_totals|node|campaign_run|form';
  $field_group->group_name = 'group_totals';
  $field_group->entity_type = 'node';
  $field_group->bundle = 'campaign_run';
  $field_group->mode = 'form';
  $field_group->parent_name = 'group_what_you_did';
  $field_group->data = array(
    'label' => 'Totals',
    'weight' => '3',
    'children' => array(
      0 => 'field_total_participants',
      1 => 'field_total_quantity',
      2 => 'field_total_quantity_label',
    ),
    'format_type' => 'fieldset',
    'format_settings' => array(
      'label' => 'Totals',
      'instance_settings' => array(
        'required_fields' => 0,
        'classes' => 'group-totals field-group-fieldset',
        'description' => 'If you don\'t want any totals displayed leave all fields blank. If you want the user to know that we are still counting the output number fill out the Total Quantity Label field and leave the Total Quantity field blank. Otherwise any fields filled out will display.',
      ),
      'formatter' => 'collapsed',
    ),
  );
  $export['group_totals|node|campaign_run|form'] = $field_group;

  $field_group = new stdClass();
  $field_group->disabled = FALSE; /* Edit this to true to make a default field_group disabled initially */
  $field_group->api_version = 1;
  $field_group->identifier = 'group_what_you_did|node|campaign_run|form';
  $field_group->group_name = 'group_what_you_did';
  $field_group->entity_type = 'node';
  $field_group->bundle = 'campaign_run';
  $field_group->mode = 'form';
  $field_group->parent_name = '';
  $field_group->data = array(
    'label' => 'What You Did',
    'weight' => '1',
    'children' => array(
      0 => 'field_intro',
      1 => 'group_totals',
    ),
    'format_type' => 'fieldset',
    'format_settings' => array(
      'label' => 'What You Did',
      'instance_settings' => array(
        'required_fields' => 0,
        'classes' => 'group-what-you-did field-group-fieldset',
        'description' => '',
      ),
      'formatter' => 'collapsed',
    ),
  );
  $export['group_what_you_did|node|campaign_run|form'] = $field_group;

  $field_group = new stdClass();
  $field_group->disabled = FALSE; /* Edit this to true to make a default field_group disabled initially */
  $field_group->api_version = 1;
  $field_group->identifier = 'group_winners|node|campaign_run|form';
  $field_group->group_name = 'group_winners';
  $field_group->entity_type = 'node';
  $field_group->bundle = 'campaign_run';
  $field_group->mode = 'form';
  $field_group->parent_name = '';
  $field_group->data = array(
    'label' => 'Winners',
    'weight' => '3',
    'children' => array(
      0 => 'field_winners',
    ),
    'format_type' => 'fieldset',
    'format_settings' => array(
      'label' => 'Winners',
      'instance_settings' => array(
        'required_fields' => 0,
        'classes' => 'group-winners field-group-fieldset',
        'description' => '',
      ),
      'formatter' => 'collapsed',
    ),
  );
  $export['group_winners|node|campaign_run|form'] = $field_group;

  return $export;
}
