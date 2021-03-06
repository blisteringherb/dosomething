<?php

/**
 * Represents UK Local user.
 */
class DosomethingUkRegisterController implements ExternalAuthRegisterController {

  // ---------------------------------------------------------------------
  // Instance variables

  /**
   * The remote user object.
   *
   * @var DosomethingUkSsoUser
   */
  private $remote_account;

  /**
   * Signup http response.
   */
  private $signup_result;

  /**
   * The SSO controller.
   *
   * @var DosomethingUkSsoController
   */
  private $sso;

  // ---------------------------------------------------------------------
  // Public: interface implementation

  public function setup(Array $form, Array &$form_state) {
    $values = &$form_state['values'];

    // -- Required user data. --
    $user_data = array(
      'first_name' => $values['field_first_name'][LANGUAGE_NONE][0]['value'],
      'password'   => $values['pass'],
      'email'      => $values['mail'],
      'dob'        => $values['field_birthdate'][LANGUAGE_NONE][0]['value'],
      'last_name'  => $values['field_last_name'][LANGUAGE_NONE][0]['value'],
      'postcode'   => $values['field_address'][LANGUAGE_NONE][0]['postal_code'],
    );
    $this->remote_account = DosomethingUkSsoUser::fromArray($user_data);

    // -- Optional user data. --
    // Determines whether the user agreed to receive e-mail newsletter.
    if (!empty($values['field_opt_in_email'][LANGUAGE_NONE][0]['value'])) {
      $this->remote_account->setContactable(TRUE);
    }

    // Use unpreprocessed cell number.
    // Actual value is unset during validation of the number format,
    // @see dosomething_user_register_validate().
    // However, it's fine to use the raw value since it will be validated on SSO
    // anyways. Appropriate error message will be rendered on validation fail.
    //
    // WARNING! If you are copying this block, please be aware that
    // $form_state['input'] is not the same as $form_state['values'] and may
    // contain unsafe data values.
    //
    // @todo: use normal value when the international phone validator is ready.
    if (!empty($form_state['input']['field_mobile'][LANGUAGE_NONE][0]['value'])) {
      $phone = $form_state['input']['field_mobile'][LANGUAGE_NONE][0]['value'];
      $this->remote_account->setPhoneNumber($phone);

      // Save the phone number as is to the local DB only
      // when it passed the remote validation.
      form_set_value(
        $form['field_mobile'],
        array(LANGUAGE_NONE => array(0 => array('value' => $phone))),
        $form_state
      );
    }

    // If the postal code is validated remotely, save also the country.
    if ($this->remote_account->getPostcode()) {
      form_set_value(
        $form['field_address'],
        array(LANGUAGE_NONE => array(array(
          'postal_code' => $this->remote_account->getPostcode(),
          'country' => $this->remote_account->getCountry(),
        ))),
        $form_state
      );
    }

    return $this;
  }

  public function signup() {
    $this->sso = DosomethingUkSsoController::signup($this->remote_account);
    $this->signup_result = $this->sso->getLastResult();
    return (bool) $this->signup_result;
  }

  public function processSignupErrors(Array $form) {
    if ($error_messages = $this->sso->getErrorMessages()) {
      dosomething_uk_remote_errors_to_local_fields($error_messages, $form);
    }

    // SSO returned unexpected errors with no much to local fields.
    if (!form_get_errors()) {
      form_set_error('form', t('Unknown error during response parsing.'));
    }
  }

  // ---------------------------------------------------------------------

}
