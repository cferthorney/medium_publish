<?php

class MediumDrupalException extends Exception {

  protected $apiMessage;
  protected $apiCode;

  public function __construct($message, $code, $apiMessage = NULL, $apiCode = NULL) {
    $this->apiMessage = $apiMessage;
    $this->apiCode = $apiCode;
    parent::__construct($message, $code);
  }

  public function getApiMessage() {
    return $this->apiMessage;
  }

  public function getApiCode() {
    return $this->apiCode;
  }

}
