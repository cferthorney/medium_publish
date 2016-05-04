<?php
/**
 * @file
 * medium.medium.inc
 */

class MediumDrupal {
  private $api_url = 'https://api.medium.com/v1/';

  protected $client_id;
  protected $client_secret;
  protected $state;
  protected $redirect_url;
  protected $scopes;

  protected $access_token = NULL;

  function __construct($credentials) {
    $this->setUpCredentials($credentials);
  }

  /**
   * Get the url to authenticate the user to medium.
   *
   * @return string
   */
  public function getAuthenticationUrl() {
    $params = array(
      'client_id' => $this->client_id,
      'scope' => $this->scopes,
      'state' => $this->state,
      'response_type' => 'code',
      'redirect_uri' => $this->redirect_url,
    );
    return 'https://medium.com/m/oauth/authorize?' . http_build_query($params);
  }

  /**
   * Get an access token (authenticate) from the user
   * to make requests to medium's api using the authorization code.
   *
   * @param string $authorizationCode
   *
   * @return void
   *
   * @TODO Use drupal_http_request.
   */
  public function authenticate($authorizationCode) {
    // 1. Request access token
    // 2. Access-token from Client class.
    $test = $this->requestAccessToken($authorizationCode);
    return $test;
  }

  /**
   * Ask medium for an access token
   * using the provided authorization code.
   *
   * @param string $authorizationCode
   * @param string $clientId
   * @param string $clientSecret
   * @param string $redirectUrl
   *
   * @return string
   *
   * @TODO Handle errors.
   */
  public function requestAccessToken($authorizationCode) {
    $data = array(
      'code' => $authorizationCode,
      'client_id' => $this->client_id,
      'client_secret' => $this->client_secret,
      'grant_type' => 'authorization_code',
      'redirect_uri' => $this->redirect_url,
    );
    $response_data = $this->request('POST', 'tokens', $data);

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
   * @param string $refreshToken
   *
   * @return string
   *
   * @TODO Use drupal_http_request.
   */
  public function exchangeRefreshToken($refreshToken) {
    $data = array(
      'refresh_token' => $refreshToken,
      'client_id' => $this->client_id,
      'client_secret' => $this->client_secret,
      'grant_type' => 'refresh_token',
    );

    $response_data = $this->request('POST', 'tokens', $data);

    return $response_data->access_token;
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
   *
   * @return void
   */
  private function setUpCredentials($credentials) {
    if (is_array($credentials)) {
      // using full credentials
      $this->client_id = $credentials['client_id'];
      $this->client_secret = $credentials['client_secret'];
      $this->redirect_url = $credentials['redirect_url'];
      $this->state = $credentials['state'];
      $this->scopes = $credentials['scopes'];
    }
    else {
      // using self issued access token
      $this->accessToken = $credentials;
    }
  }

  /**
   * Perform requests towards Medium.com.
   */
  private function request($method, $endpoint, $data) {
    $headers = array(
      'Content-Type' => 'application/x-www-form-urlencoded',
      'Accept' => 'application/json',
      'Accept-Charset' => 'utf-8',
    );

    if (!is_null($this->access_token)) {
      $headers['Bearer'] = $this->access_token;
    }

    $response = drupal_http_request($this->api_url . $endpoint, array(
      'headers' => $headers,
      'method' => $method,
      'data' => drupal_http_build_query($data),
    ));
    dpm($response);

    return json_decode($response->data);
  }
}
