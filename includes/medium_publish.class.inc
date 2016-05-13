<?php
/**
 * @file
 * Medium.medium.inc.
 */

/**
 * Handles communication with Medium.
 */
class MediumDrupal {
  private $apiUrl = 'https://api.medium.com/v1/';

  protected $clientId;
  protected $clientSecret;
  protected $state;
  protected $redirectUrl;
  protected $scopes;

  protected $accessToken = NULL;

  /**
   * Construct.
   */
  public function __construct($credentials) {
    $this->setUpCredentials($credentials);
  }

  /**
   * Get the url to authenticate the user to medium.
   *
   * @return string
   *   Formatted connection url.
   */
  public function getAuthenticationUrl() {
    $params = array(
      'client_id' => $this->clientId,
      'scope' => $this->scopes,
      'state' => $this->state,
      'response_type' => 'code',
      'redirect_uri' => $this->redirectUrl,
    );
    return 'https://medium.com/m/oauth/authorize?' . http_build_query($params);
  }

  /**
   * Get an access token (authenticate) from api.
   *
   * @param string $authorization_code
   *   The authorization code from oAuth flow.
   *
   * @return array
   *   Array of elements from Medium.
   */
  public function authenticate($authorization_code) {
    return $this->requestAccessToken($authorization_code);
  }

  /**
   * Ask medium for an access token.
   *
   * @param string $authorization_code
   *   The authorization code from oAuth flow.
   *
   * @return array
   *   Array of elements from Medium.
   *
   * @TODO Handle errors.
   */
  public function requestAccessToken($authorization_code) {
    $data = array(
      'code' => $authorization_code,
      'client_id' => $this->clientId,
      'client_secret' => $this->clientSecret,
      'grant_type' => 'authorization_code',
      'redirect_uri' => $this->redirectUrl,
    );
    $response_data = $this->request('POST', 'tokens', $data);

    $this->accessToken = $response_data->access_token;

    return array(
      'access_token' => $response_data->access_token,
      'refresh_token' => $response_data->refresh_token,
      'scope' => $response_data->scope,
      'expires_at' => ($response_data->expires_at / 1000),
    );
  }

  /**
   * Request a new access token using the refresh token.
   *
   * @param string $refresh_token
   *   The refresh token stored from first auth.
   *
   * @return array
   *   Array of elements from Medium.
   */
  public function exchangeRefreshToken($refresh_token) {
    $data = array(
      'refresh_token' => $refresh_token,
      'client_id' => $this->clientId,
      'client_secret' => $this->clientSecret,
      'grant_type' => 'refresh_token',
    );

    $response_data = $this->request('POST', 'tokens', $data);
    $this->accessToken = $response_data->access_token;

    return array(
      'access_token' => $response_data->access_token,
      'refresh_token' => $response_data->refresh_token,
      'scope' => $response_data->scope,
      'expires_at' => ($response_data->expires_at / 1000),
    );
  }

  /**
   * Ask for user information.
   */
  public function getAuthenticatedUser() {
    return $this->request('GET', 'me');
  }

  /**
   * Get the specified user publications.
   *
   * @param string $user_id
   *   The user id we want to get publications for.
   */
  public function getPublications($user_id) {
    return $this->request('GET', 'users/' . $user_id . '/publications');
  }

  /**
   * Publish a post.
   */
  public function createPost($author_id, array $data) {
    $response_data = $this->request('POST', 'users/' . $author_id . '/posts', $data);
  }

  /**
   * Publish a post to publication.
   */
  public function createPostforPublication($publication_id, array $data) {
    $response_data = $this->request('POST', 'publications/' . $publication_id . '/posts', $data);
  }

  /**
   * Setup initial api credentials.
   *
   * @param mixed $credentials
   *   The self issued token or array of credentials.
   */
  private function setUpCredentials($credentials) {
    if (is_array($credentials)) {
      $this->clientId = $credentials['client_id'];
      $this->clientSecret = $credentials['client_secret'];

      if (isset($credentials['redirect_url'])) {
        $this->redirectUrl = $credentials['redirect_url'];
      }

      if (isset($credentials['state'])) {
        $this->state = $credentials['state'];
      }

      if (isset($credentials['scopes'])) {
        $this->scopes = $credentials['scopes'];
      }

      if (isset($credentials['access_token'])) {
        $this->accessToken = $credentials['access_token'];
      }
    }
    else {
      $this->accessToken = $credentials;
    }
  }

  /**
   * Perform requests towards Medium.com.
   */
  private function request($method, $endpoint, $data = array()) {
    $headers = array(
      'Content-Type' => 'application/x-www-form-urlencoded',
      'Accept' => 'application/json',
      'Accept-Charset' => 'utf-8',
    );

    if (!is_null($this->accessToken)) {
      $headers['Authorization'] = 'Bearer ' . $this->accessToken;
    }

    $response = drupal_http_request($this->apiUrl . $endpoint, array(
      'headers' => $headers,
      'method' => $method,
      'data' => drupal_http_build_query($data),
    ));

    $json = json_decode($response->data);
    if ($response->code >= 400) {
      if (isset($json->errors[0])) {
        throw new MediumDrupalException($response->status_message, $response->code, $json->errors[0]->message, $json->errors[0]->code);
      }
      else {
        throw new MediumDrupalException($response->status_message, $response->code);
      }
    }

    return $json;
  }

}
