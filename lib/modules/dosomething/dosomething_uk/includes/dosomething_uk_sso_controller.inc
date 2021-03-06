<?php

/**
 * A http client.
 */
class DosomethingUkSsoController {

  // ---------------------------------------------------------------------
  // Class constants

  // OAuth settings.
  const SIGNATURE_METHOD = 'sha1';
  const PARSE_METHOD     = HttpClientBaseFormatter::FORMAT_JSON;

  // Token lifetime.
  const REQUEST_TOKEN_LIFETIME     = 1800;    // 30 Minutes
  const ACCESS_TOKEN_LIFETIME      = 1800;    // 30 Minutes
  const USER_ACCESS_TOKEN_LIFETIME = 2592000; // 30 Days

  // Clearance code.
  const AUTHORIZED_NONE            = 0;
  const AUTHORIZED_REQUEST_TOKEN   = 1;
  const AUTHORIZED_REMOTE_REDIRECT = 2;
  const AUTHORIZED_USER_RETURNED   = 3;
  const AUTHORIZED_ACCESS_TOKEN    = 4;
  const AUTHORIZED_USER_ACCESS     = 5;

  // Endpoints.
  const ENDPOINT_USER_SHOW_CURRENT = '/users/current';
  const ENDPOINT_USER_CREATE       = '/signup';

  // Other.
  const DEBUG                      = 1;
  const LOGGER_NAME                = 'DosomethingUkSsoController';
  const PRODIGAL_USER_SESSION_NAME = 'DosomethingUkSsoOauthToken';

  // ---------------------------------------------------------------------
  // Instance variables

  /**
   * [$url description]
   * @var string
   */
  private $url;

  /**
   * [$key description]
   * @var string
   */
  private $key;

  /**
   * [$secret description]
   * @var string
   */
  private $secret;

  /**
   * [$callback description]
   * @var string
   */
  private $callback;

  /**
   * [$cipher description]
   * @var OAuthSignatureMethod
   */
  private $cipher;

  /**
   * [$parser description]
   * @var HttpClientFormatter
   */
  private $parser;

  /**
   * [$keymaker description]
   * @var DrupalOAuthClient
   */
  private $keymaker;

  /**
   * [$consumer description]
   * @var DrupalOAuthConsumer
   */
  private $consumer;

  /**
   * [$request_token description]
   * @var DrupalOAuthToken
   */
  private $request_token;

  /**
   * [$access_token description]
   * @var DrupalOAuthToken
   */
  private $access_token;

  /**
   * [$uid description]
   * @var int
   */
  private $uid;

  /**
   * [$clearance description]
   * @var int
   */
  private $clearance;

  /**
   * [$http_client description]
   * @var HttpClient
   */
  private $http_client;

  /**
   * [$last_result description]
   * @var string
   */
  private $last_result;

  /**
   * [$error_messages description]
   * @var string
   */
  private $error_messages;

  // ---------------------------------------------------------------------
  // Constructor and factories

  /**
   * Constructs a DosomethingUkSsoController object.
   */
  public function __construct()
  {
    // Drupal base URL.
    // Not to be confused with $this->url, remote API base URL.
    global $base_url;

    // OAuth credentials.
    $this->url    = variable_get('dosomething_uk_oauth_url');
    $this->key    = variable_get('dosomething_uk_oauth_key');
    $this->secret = variable_get('dosomething_uk_oauth_secret');
    if (!$this->url || !$this->key || !$this->secret) {
      throw new OAuthException("Insufficient OAuth configuration.");
    }

    // Optional callback.
    $this->callback = variable_get('dosomething_uk_oauth_callback', $base_url);

    // HTTP client settings.
    $this->cipher = DrupalOAuthClient::signatureMethod(self::SIGNATURE_METHOD);
    $this->parser = new HttpClientBaseFormatter(self::PARSE_METHOD);

    // Defaults.
    $this->keymaker       = FALSE;
    $this->consumer       = FALSE;
    $this->request_token  = FALSE;
    $this->access_token   = FALSE;
    $this->uid            = FALSE;
    $this->clearance      = self::AUTHORIZED_NONE;
    $this->http_client    = FALSE;
    $this->last_result    = FALSE;
    $this->error_messages = FALSE;
  }

  /**
   *  Static class builder.
   *
   * @return static
   */
  public static function init() {
    $instance = new self();
    $instance->authorize();
    return $instance;
  }

  /**
   * Initialize class using saved user's access token.
   *
   * @param int $uid
   *   Integer specifying the user ID to load access for.
   *
   * @return static
   */
  public static function initForLocalUser($uid) {
    $instance = new self();
    $instance->uid = $uid;
    $instance->authorize($uid);
    return $instance;
  }

public static function signup(DosomethingUkSsoUser $sso_user) {
    $instance = new self();
    $instance->setupConsumer();
    $instance->setupRemoteClient();
    // @todo: Find a place to keep the user.
    $user = $instance->createRemoteUser($sso_user);
    // @todo: Assign perishable token.
    return $instance;
  }

  // ---------------------------------------------------------------------
  // Public: remote API calls

  public function remoteUserAdmitted()
  {
    return !!$this->http_client;
  }

  public function getRemoteUser()
  {
    $user_data = $this->getRemote(self::ENDPOINT_USER_SHOW_CURRENT);
    $sso_user = DosomethingUkSsoUser::fromArray($user_data);
    return $sso_user;
  }

  public function createRemoteUser(DosomethingUkSsoUser $sso_user)
  {
    $sso_user = $this->createRemote(
      self::ENDPOINT_USER_CREATE, $sso_user->toFormFields()
    );
    return $sso_user;
  }

  /**
   * Performs a GET request to the remote API.
   *
   * @param  string $path
   *   Endpoint path on the remote server.
   *
   * @return mixed
   *   Remote response parsed to PHP compound type
   */
  public function getRemote($path)
  {
    if (!$this->remoteUserAdmitted()) {
      return FALSE;
    }

    $this->error_messages = FALSE;
    $this->last_result    = FALSE;

    try {
      $this->last_result = $this->http_client->get($this->apiPath($path));
      // @todo: parse result like it's done in createRemote().
    }
    catch (Exception $e) {
      $this->logExeption($e);
    }
    return $this->last_result;
  }

  /**
   * Performs a POST request to the remote API.
   *
   * @param  string $path
   *   Endpoint path on the remote server.
   *
   * @return mixed
   *   Remote response parsed to PHP compound type
   */
  public function createRemote($path, $data)
  {
    if (!$this->remoteUserAdmitted()) {
      return FALSE;
    }

    $this->error_messages = FALSE;
    $this->last_result    = FALSE;

    try {
      $this->last_result = $this->http_client->post($this->apiPath($path), NULL, $data);
    }
    catch (HttpClientException $e) {
      $this->processResponseErrorMessages($e);
      if (!$this->error_messages) {
        // Abnormal response: unexpected HTTP code or no body.
        $this->logExeption($e);
      }
    }
    catch (Exception $e) {
      $this->logExeption($e);
    }

    return $this->last_result;
  }

  // ---------------------------------------------------------------------
  // Public: getters

  /**
   * Returns current authorization stage.
   *
   * Corresponds to one of AUTHORIZED_* class constants.
   *
   * @return int
   *   Current clearance code
   */
  public function getClearance()
  {
    return $this->clearance;
  }

  public function getLastResult()
  {
    return $this->last_result;
  }

  public function getErrorMessages()
  {
    return $this->error_messages;
  }

  public function getAuthorizationUrl($relative = FALSE, $skip_callback = FALSE)
  {
    // Available for users authorized for redirect and returned users.
    if ($this->clearance < self::AUTHORIZED_REMOTE_REDIRECT ||
        $this->clearance > self::AUTHORIZED_USER_RETURNED) {
      // Todo: watchdog.
      return FALSE;
    }

    $authorize_url = $this->keymaker->getAuthorizationUrl();

    if (!$skip_callback) {
      $authorize_url .= strpos($authorize_url, '?') === FALSE ? '?' : '&';
      $authorize_url .= http_build_query(array('callback' => $this->callback));
    }

    if (!$relative) {
      return $authorize_url;
    }

    $parsed = parse_url($authorize_url);
    return $parsed['path'] . '?' . $parsed['query'];
  }

  // ---------------------------------------------------------------------
  // Public: auth flow

  public function authorize()
  {
    $this->setupConsumer();
    $this->checkClearance();

    if ($this->clearance < self::AUTHORIZED_ACCESS_TOKEN) {
      $this->setupKeymaker();
    }
    else {
      $this->admitRemoteUser();
    }
    return $this;
  }

  public function authorizeRequestToken()
  {
    self::logDebug('Authorizing request token.');

    // No request token needed: access token is already available.
    if ($this->clearance >= self::AUTHORIZED_ACCESS_TOKEN) {
      return $this;
    }

    $this->destroyProdigalUserSession();
    try {
      $request_token = $this->keymaker->getRequestToken();
    }
    catch (Exception $e) {
      self::logExeption($e);
      return $this;
    }

    if (!$request_token || empty($request_token->key)) {
      // Todo: watchdog.
      return $this;
    }
    $this->request_token = $request_token;
    $this->request_token->expires = REQUEST_TIME + self::REQUEST_TOKEN_LIFETIME;
    $this->request_token->write();

    $this->clearance = self::AUTHORIZED_REQUEST_TOKEN;
    return $this;
  }

  public function authorizeRemoteRedirect()
  {
    self::logDebug('Authorizing remote redirect.');

    // No request token:
    if ($this->clearance < self::AUTHORIZED_REQUEST_TOKEN) {

      // Try to authorize request token automatically:
      $this->authorizeRequestToken();

      if ($this->clearance == self::AUTHORIZED_REQUEST_TOKEN) {
        // If sucessfull, restart this function:
        return $this->authorizeRemoteRedirect();
      }

      // Otherwise, do nothing.
      return $this;
    }

    // Session already has been saved.
    if ($this->clearance > self::AUTHORIZED_REMOTE_REDIRECT) {
      return $this;
    }

    // Save client session.
    $this->saveProdigalUserSession();
    $this->clearance = self::AUTHORIZED_REMOTE_REDIRECT;
    return $this;
  }

  public function authorizeAccessToken()
  {
    self::logDebug('Authorizing access token.');

    // When redirected user is not recognized as returned,
    // try to reauthorize it automatically.
    if ($this->clearance == self::AUTHORIZED_REMOTE_REDIRECT) {
      $this->authorize();
    }

    // Procceed only when the user sucesfully got back from redirect:
    if ($this->clearance != self::AUTHORIZED_USER_RETURNED) {
      return $this;
    }

    try {
      $access_token = $this->keymaker->getAccessToken();
    }
    catch (Exception $e) {
      self::logExeption($e);
      return $this;
    }

    if (!$access_token || empty($access_token->key)) {
      // Todo: watchdog.
      return $this;
    }

    $this->access_token = $access_token;
    $this->access_token->expires = REQUEST_TIME + self::ACCESS_TOKEN_LIFETIME;
    $this->access_token->write();
    $this->request_token->delete();
    $this->destroyProdigalUserSession();
    $this->clearance = self::AUTHORIZED_ACCESS_TOKEN;
    $this->admitRemoteUser();
    return $this;
  }

  public function authorizeUserAccess($uid)
  {
    self::logDebug('Authorizing user access token.');
    if ($this->clearance != self::AUTHORIZED_ACCESS_TOKEN) {
      // Todo: watchdog.
      return $this;
    }

    $this->uid = $uid;
    $this->access_token->expires = REQUEST_TIME + self::USER_ACCESS_TOKEN_LIFETIME;
    $this->access_token->uid = $this->uid;
    $this->access_token->write();
    $this->clearance = self::AUTHORIZED_USER_ACCESS;
    return $this;
  }

  // ---------------------------------------------------------------------
  // Private: initialization

  private function checkClearance()
  {
    // Check if self::AUTHORIZED_USER_ACCESS. From saved user access token:
    if ($this->uid) {
      $access_token = $this->getUserAccessToken($this->uid);
      if (!$access_token) {
        $this->clearance = self::AUTHORIZED_NONE;
        return FALSE;
      }
      $this->access_token = $access_token;
      $this->clearance = self::AUTHORIZED_USER_ACCESS;
      return TRUE;
    }


    // Check if self::AUTHORIZED_USER_RETURNED. From saved session request key:
    $session = $this->getProdigalUserSession();
    if (!$session) {
      $this->clearance = self::AUTHORIZED_NONE;
      return TRUE;
    }

    $request_token = $this->getStoredRequestToken($session);
    if (!$request_token) {
      $this->destroyProdigalUserSession();
      $this->clearance = self::AUTHORIZED_NONE;
      return FALSE;
    }

    $this->request_token = $request_token;
    $this->clearance = self::AUTHORIZED_USER_RETURNED;
    return TRUE;
  }

  private function admitRemoteUser()
  {
    if ($this->clearance < self::AUTHORIZED_ACCESS_TOKEN) {
      // Todo: watchdog.
      return FALSE;
    }

    $this->setupRemoteClient();
  }

  private function setupKeymaker()
  {
    switch ($this->clearance) {
      case self::AUTHORIZED_USER_RETURNED:
        $this->keymaker = new DrupalOAuthClient($this->consumer,
                                                $this->request_token);
        break;

      case self::AUTHORIZED_NONE:
        $this->keymaker = new DrupalOAuthClient($this->consumer);
        break;
    }
  }

  private function setupConsumer()
  {
    if ($this->consumer) {
      return FALSE;
    }

    $consumer = DrupalOAuthConsumer::load($this->key, FALSE);
    if (!$consumer) {
      $consumer = new DrupalOAuthConsumer($this->key, $this->secret);
      $consumer->configuration['provider_url']     = $this->url;
      $consumer->configuration['signature_method'] = self::SIGNATURE_METHOD;
      $consumer->write();
    }

    if (!$consumer) {
      return FALSE;
    }

    $this->consumer = $consumer;
    return TRUE;
  }

  private function setupRemoteClient()
  {
    $oauth = new HttpClientOAuth(
      $this->consumer, $this->access_token, $this->cipher, FALSE, TRUE
    );

    $this->http_client = new HttpClient();
    $this->http_client->setFormatter($this->parser);
    $this->http_client->setAuthentication($oauth);
  }

  // ---------------------------------------------------------------------
  // Private: getters/setters

  private function getStoredRequestToken($request_token_key)
  {
    return DrupalOAuthToken::loadByKey($request_token_key, $this->consumer,
      OAUTH_COMMON_TOKEN_TYPE_REQUEST
    );
  }

  private function getUserAccessToken($uid)
  {
    $select = db_select('oauth_common_token', 't')
      ->fields('t')
      ->condition('csid', $this->consumer->csid, '=')
      ->condition('uid',  $uid, '=')
      ->condition('type', OAUTH_COMMON_TOKEN_TYPE_ACCESS, '=')
      ->orderBy('tid', 'DESC')
      ->range(0, 1)
      ->execute();
    return DrupalOAuthToken::fromResult($select, $this->consumer);
  }

  private function getProdigalUserSession()
  {
    if (!empty($_SESSION[self::PRODIGAL_USER_SESSION_NAME])) {
      return $_SESSION[self::PRODIGAL_USER_SESSION_NAME];
    }
    return FALSE;
  }

  private function saveProdigalUserSession()
  {
    if ($this->clearance != self::AUTHORIZED_REQUEST_TOKEN) {
      return FALSE;
    }
    $_SESSION[self::PRODIGAL_USER_SESSION_NAME] = $this->request_token->key;
  }

  private function destroyProdigalUserSession()
  {
    $_SESSION[self::PRODIGAL_USER_SESSION_NAME] = null;
    unset($_SESSION[self::PRODIGAL_USER_SESSION_NAME]);
  }

  // ---------------------------------------------------------------------
  // Private: utilities

  private function processResponseErrorMessages(HttpClientException $http_error) {
    // Allow the following HTTP codes:
    // - 422 Unprocessable Entity
    $allowed_http_codes = array(422);
    if (!in_array($http_error->getCode(), $allowed_http_codes)) {
      return FALSE;
    }

    // Retrieve HTTP response body text.
    $response = $http_error->getResponse();
    if (!is_object($response) || empty($response->body)) {
      return FALSE;
    }

    self::logDebug('Response error messages: ' . $response->body);

    // Process response to PHP data types.
    try {
      $this->error_messages = $this->parser->unserialize($response->body);
    }
    catch (Exception $e) {
      // Deserialization failed.
      // @see HttpClientBaseFormatter::unserialize()
      throw new HttpClientException($e->getMessage(), 0, $response, $e);
    }

    return TRUE;
  }

  private function apiPath($path) {
    return $this->url . $path;
  }

  private static function log($message, $error)
  {
    $variables = array();
    if (!empty($error)) {
      $variables['%s'] = $error;
    }
    watchdog(self::LOGGER_NAME, $message, $variables, WATCHDOG_NOTICE);
  }


  private static function logDebug($message)
  {
    if (self::DEBUG) {
      watchdog(self::LOGGER_NAME, $message, array(), WATCHDOG_DEBUG);
    }
  }

  private static function logExeption(Exception $e)
  {
    watchdog_exception(self::LOGGER_NAME, $e);
  }

  // ---------------------------------------------------------------------

}
