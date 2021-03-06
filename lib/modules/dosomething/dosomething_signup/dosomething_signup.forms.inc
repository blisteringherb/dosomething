<?php

/**
 * @file
 * Code for dosomething_signup forms.
 */

/**
 * Implements hook_forms().
 *
 * Allows dosomething_signup_form to render multiple times on auth pitch page.
 */
function dosomething_signup_forms($form_id, $args) {
  // If this is one of multiple signup forms rendered on the page:
  if (strpos($form_id, 'dosomething_signup_form_') === 0) {
    // Set its callback and callback arguments:
    $forms[$form_id] = array(
      // Set the callback as dosomething_signup_form.
      'callback' => 'dosomething_signup_form',
      // Pass the relevant $args to it.
      'callback arguments' => $args,
    );
    return $forms;
  }
}

/**
 * Form constructor for user signup form.
 *
 * Displayed for an authenticated user to signup for a node.
 *
 * @param int $nid
 *   The node nid to signup for.
 * @param string $label
 *   The label to display on the form submit button.
 */
function dosomething_signup_form($form, &$form_state, $nid, $label = "Sign Up") {
  $form['nid'] = array(
    '#type' => 'hidden',
    '#value' => $nid,
    '#access' => FALSE,
  );
  // Check query string for signup source value.
  if ($source = dosomething_signup_get_query_source()) {
    $form['source'] = array(
      '#type' => 'hidden',
      '#value' => $source,
      '#access' => FALSE,
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $label,
    '#attributes' => array(
      'class' => array(
        'btn',
      ),
    ),
  );
  return $form;
}

/**
 * Form submission handler for dosomething_signup_form().
 *
 * @see dosomething_signup_form()
 */
function dosomething_signup_form_submit($form, &$form_state) {
  // Shouldn't be able to submit form as anonymous user, but check.
  if (!user_is_logged_in()) {
    drupal_set_message("Please sign in first.");
    return;
  }
  $nid = $form_state['values']['nid'];
  $source = NULL;
  if (isset($form_state['values']['source'])) {
    $source = $form_state['values']['source'];
  }
  // Create signup for logged in user.
  dosomething_signup_user_signup($nid, NULL, $source);
}

/**
 * Form constructor for user presignup form.
 *
 * @param int $nid
 *   The node nid to presignup for.
 * @param string $label
 *   The label to display on the form submit button.
 */
function dosomething_signup_presignup_form($form, &$form_state, $nid, $label) {
  $form['nid'] = array(
    '#type' => 'hidden',
    '#value' => $nid,
    '#access' => FALSE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $label,
    '#attributes' => array(
      'class' => array(
        'btn',
        'medium',
      ),
    ),
  );
  return $form;
}

/**
 * Form submission handler for dosomething_signup_presignup_form().
 *
 * @see dosomething_signup_presignup_form()
 */
function dosomething_signup_presignup_form_submit($form, &$form_state) {
  $nid = $form_state['values']['nid'];
  // Handle user presignup.
  dosomething_signup_user_presignup($nid);
}

/**
 * Form constructor for a SMS Game signup form.
 *
 * @param int $nid
 *   Loaded $node that the form is being rendered on.
 * @param int $num_friends
 *   Number of inputs to provide for friends' numbers.
 */
function dosomething_signup_sms_game_form($form, &$form_state, $node, $num_friends = 3) {
  $nid = $node->nid;
  // Add relevant configuration form elements.
  dosomething_signup_sms_game_form_config($form, $nid);

  $form['nid'] = array(
    '#type' => 'hidden',
    '#default_value' => $nid,
    '#access' => FALSE,
  );
  $form['title'] = array(
    '#type' => 'hidden',
    '#default_value' => $node->title,
    '#access' => FALSE,
  );
  $form['num_betas'] = array(
    '#type' => 'hidden',
    '#default_value' => $num_friends,
    '#access' => FALSE,
  );
  // Load "all participants" copy.
  $all_participants_copy = variable_get('dosomething_campaign_sms_game_all_participants_copy');
  $form['all_participants_copy'] = array(
    '#prefix' => '<div class="message-callout -above"><div class="__copy"><p>',
    '#markup' => $all_participants_copy,
    '#suffix' => '</p></div></div>',
  );
  $form['alpha_first_name'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => 'Your First Name',
    '#default_value' => dosomething_user_get_field('field_first_name'),
    '#attributes' => array(
      'data-validate' => 'fname',
      'data-validate-required' => '',
      'placeholder' => t('First Name'),
    ),
  );
  // Add fieldsets to wrappers to various mobile form elements.
  $form['group1'] = array(
    '#type' => 'fieldset',
    '#attributes' => array(
      'class' => array('-columned first-group'),
    ),
  );
  $form['group2'] = array(
    '#type' => 'fieldset',
    '#attributes' => array(
      'class' => array('-columned second-group'),
    ),
  );
  $form['group1']['alpha_mobile'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => 'Your Cell Number',
    '#default_value' => dosomething_user_get_field('field_mobile'),
    '#attributes' => array(
      'data-validate' => 'phone',
      'data-validate-required' => '',
      'placeholder' => t('(555) 555-5555'),
      'autocomplete' => 'off',
    ),
  );
  for ($i = 0; $i < $num_friends; $i++) {
    // Default to second fieldset.
    $group = 'group2';
    if ($i == 0) {
      // Unless its the first beta.
      $group = 'group1';
    }
    $form[$group]['beta_mobile_' .$i] = array(
      '#type' => 'textfield',
      '#required' => TRUE,
      '#title' => 'Friend\'s Cell Number',
      '#attributes' => array(
        'data-validate' => 'phone',
        'data-validate-required' => '',
        'placeholder' => t('(555) 555-5555'),
        'autocomplete' => 'off',
      ),
    );
  }
  // Check if varaible is set for the Friends Form submit button label.
  $label = variable_get('dosomething_campaign_sms_game_signup_friends_form_button_copy', t('Share'));
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => $label,
    ),
  );
  return $form;
}

/**
 * Add hidden form elements for SMS Game $nid, or set message if not configured.
 *
 * @param array $form
 *   A Form API form to add the elements to.
 * @param int $nid
 *   The node $nid to check configuration values for.
 */
function dosomething_signup_sms_game_form_config(&$form, $nid) {
  $vars = dosomething_helpers_get_variables($nid);

  $sms_game_type = NULL;

  // Single-player variables:
  $opt_in_path = $vars['mobilecommons_opt_in_path'];
  $friends_opt_in_path = $vars['mobilecommons_friends_opt_in_path'];

  // Multi-player variables:
  $mp_story_id = $vars['sms_game_mp_story_id'];
  $mp_story_type = $vars['sms_game_mp_story_type'];

  // Check for single-player variables:
  if ($opt_in_path && $friends_opt_in_path) {
    $sms_game_type = 'single_player';
    $form['opt_in_path'] = array(
      '#type' => 'hidden',
      '#default_value' => $opt_in_path,
      '#access' => FALSE,
    );
    $form['friends_opt_in_path'] = array(
      '#type' => 'hidden',
      '#default_value' => $friends_opt_in_path,
      '#access' => FALSE,
    );
  }
  // Else check for multi-player variables:
  elseif ($mp_story_id && $mp_story_type) {
    $sms_game_type = 'multi_player';
    $form['story_id'] = array(
      '#type' => 'hidden',
      '#default_value' => $mp_story_id,
      '#access' => FALSE,
    );
    $form['story_type'] = array(
      '#type' => 'hidden',
      '#default_value' => $mp_story_type,
      '#access' => FALSE,
    );
  }
  else {
    // Else set a warning message to inform staff to not expect any texts.
    drupal_set_message(t("This SMS Game is not properly configured."), "error");
  }

  if ($sms_game_type) {
    $form['sms_game_type'] = array(
      '#type' => 'hidden',
      '#default_value' => $sms_game_type,
      '#access' => FALSE,
    );
  }
}

/**
 * Form validation handler for dosomething_signup_sms_game_form().
 *
 * Provides server-side checks for valid and duplicate numbers, cleans input format.
 */
function dosomething_signup_sms_game_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  // Text to display for invalid number format.
  $invalid_format_msg = t("Please provide a valid telephone number.");
  // Initialize array to store all inputted numbers, to check for duplicates.
  $numbers = array();

  // Check if alpha is a valid number:
  $alpha_value = dosomething_user_clean_mobile_number($values['alpha_mobile']);
  if (!$alpha_value) {
    form_set_error('alpha_mobile', $invalid_format_msg);
  }
  else {
    // Add to numbers inputted.
    $numbers[] = $alpha_value;
    // Update form_state value with the clean format.
    $form_state['values']['alpha_mobile'] = $alpha_value;
  }

  // Loop through beta form values.
  for ($i = 0; $i < $values['num_betas']; $i++) {
    $beta = 'beta_mobile_' . $i;
    // If a value exists:
    if (!empty($values[$beta])) {

      // Check if valid number.
      $beta_value = dosomething_user_clean_mobile_number($values[$beta]);
      if (!$beta_value) {
        form_set_error($beta, $invalid_format_msg);
      }
      // Check if the number is a duplicate value.
      elseif (in_array($beta_value, $numbers)) {
        form_set_error($beta, t("Hey, you entered this number already!"));
      }
      // Else store as an inputted number to check against later.
      else {
        $numbers[] = $beta_value;
        // Update form_state value with the clean format.
        $form_state['values'][$beta] = $beta_value;
      }

    }
  }
}

/**
 * Form submission handler for dosomething_signup_sms_game_form().
 *
 * Redirects user to the reportback confirmation page for given node.
 */
function dosomething_signup_sms_game_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $nid = $values['nid'];

  if (user_is_logged_in()) {
    // Create a signup.
    dosomething_signup_create($nid);
    // @todo: Save mobile and first name to user if not set.
    // @see https://github.com/DoSomething/dosomething/issues/2274.
  }

  // Check if an account exists for the given alpha mobile.
  // An anonymous user is able to submit the form, but they may still have an account.
  if ($account = dosomething_user_get_user_by_mobile($values['alpha_mobile'])) {
    // Create a signup for the account.
    // If $account happens to be the logged in user, it's okay.
    // dosomething_signup_create will check if a signup exists before creating.
    dosomething_signup_create($nid, $account->uid);
  }

  // Send request to relevant SMS Game endpoints.
  dosomething_signup_sms_game_signup_request($values['sms_game_type'], $values);

  // Redirect to the reportback confirmation page.
  $redirect = dosomething_campaign_get_confirmation_path($values['nid']);
  $form_state['redirect'] = $redirect;
}

/**
 * Form constructor for a node unsignup form.
 *
 * @param int $nid
 *   The node nid to remove signup from.
 *
 * @ingroup forms
 */
function dosomething_signup_node_unsignup_form($form, &$form_state, $node) {
  $form['nid'] = array(
    '#type' => 'hidden',
    '#default_value' => $node->nid,
  );
  $form['warning'] = array(
    '#markup' => t("Are you sure you want to remove this signup?  This cannot be undone."),
  );
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    ),
  );
  return $form;
}

/**
 * Form submit handler for dosomething_signup_node_unsignup_form().
 */
function dosomething_signup_node_unsignup_form_submit($form, &$form_state) {
  $nid = $form_state['values']['nid'];
  if (dosomething_signup_delete_signup($nid)) {
    drupal_set_message("Signup removed.");
    $form_state['redirect'] = 'node/' . $nid;
    return;
  }
  drupal_set_message("There was an error with your request.");
}

