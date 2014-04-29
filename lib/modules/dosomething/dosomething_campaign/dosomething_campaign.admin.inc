<?php
/**
 * @file
 * Code for dosomething_campaign admin functionality.
 */

/**
 * Page callback for campaign admin status page.
 */
function dosomething_campaign_admin_status_page() {
  $vars['header'] = array('Nid', 'Edit', 'Status', 'Char count');
  // Display campaigns with title values over recommended limit.
  foreach (dosomething_campaign_admin_status_title_query() as $record) {
    $vars['rows'][] = dosomething_campaign_admin_status_get_row_array($record);
  }
  $output .= '<h2>Titles over 20 characters</h2>';
  $output .= theme('table', $vars);
  // Initalize rows as empty array.
  $vars['rows'] = array();
  // Display campaigns with field_call_to_action values over recommended limit.
  foreach (dosomething_campaign_admin_status_call_to_action_query() as $record) {
    $vars['rows'][] = dosomething_campaign_admin_status_get_row_array($record);
  }
  $output .= '<h2>Call to action over 65 characters</h2>';
  $output .= theme('table', $vars);
  return $output;
}

/**
 * Returns array for admin status table rows for given query result $record.
 *
 * @param object $record
 *   A query result record as returned by query->execute().
 *
 * @return array
 *   Array with expected values for a campaign admin status table.
 */
function dosomething_campaign_admin_status_get_row_array($record) {
  return array(
    $record->nid, 
    l($record->title, 'node/' . $record->nid . '/edit', array(
      // Link back to admin status page for editor to verify char count is ok.
      'query' => array('destination' => 'admin/content/campaign-status'))
    ),
    $record->status ? 'Published' : 'Unpublished',
    $record->length,
  );
}

/**
 * Returns campaign nodes with titles having char_count greater than given
 * $char_count.
 *
 * @param int $char_count
 *   The number of characters to check for.
 *
 * @return array
 *   Result array of the query.
 */
function dosomething_campaign_admin_status_title_query($char_count = 20) {
  $query = db_select('node', 'n');
  $query->condition('type', 'campaign');
  $query->fields('n', array('nid', 'title', 'status'));
  $query->addExpression('CHAR_LENGTH(n.title)', 'length');
  $query->where('CHAR_LENGTH(n.title) > ' . $char_count);
  return $query->execute();
}

/**
 * Returns campaign nodes with call_to_action having char_count greater than given
 * $char_count.
 *
 * @param int $char_count
 *   The number of characters to check for.
 *
 * @return array
 *   Result array of the query.
 */
function dosomething_campaign_admin_status_call_to_action_query($char_count = 65) {
  $query = db_select('node', 'n');
  $query->condition('type', 'campaign');
  $query->join('field_data_field_call_to_action', 'cta', 'n.nid = cta.entity_id');
  $query->fields('n', array('nid', 'title', 'status'));
  $query->addExpression('CHAR_LENGTH(cta.field_call_to_action_value)', 'length');
  $query->where('CHAR_LENGTH(cta.field_call_to_action_value) > ' . $char_count);
  return $query->execute();
}