<?php

class ApiCache {

  /**
   * Get data from cache based on id from endpoint and URL parameters.
   *
   * @param  string  $endpoint  Type of resource based on endpoint.
   * @param  array   $parameters  The URL parameters passed that define the request.
   * @return mixed
   */
  public function get($endpoint, $parameters) {
    $id = $this->generate_id($endpoint, $parameters);

    $cache = cache_get($id, 'cache_dosomething_api');

    if ($cache && $cache->expire < REQUEST_TIME) {
      cache_clear_all($id, 'cache_dosomething_api');

      return FALSE;
    }

    return $cache;
  }


  /**
   * Set data in cache with id based on endpoint and URL parameters.
   *
   * @param  string  $endpoint
   * @param  array   $parameters
   * @param  mixed   $data
   * @return bool
   */
  public function set($endpoint, $parameters, $data) {
    $id = $this->generate_id($endpoint, $parameters);

    cache_set($id, $data, 'cache_dosomething_api', REQUEST_TIME + (60 * 60 * 1));

    return TRUE;
  }


  /**
   * Generate an id from resource endpoint type and URL parameters.
   *
   * @param  string  $endpoint
   * @param  array   $parameters
   * @return string
   */
  private function generate_id($endpoint, $parameters) {
    return $endpoint . '_api_request' . $this->stringify($parameters);
  }


  /**
   * Create a concatenated string from URL parameters.
   *
   * @param  array  $parameters
   * @return string
   * @todo: may want to allow passing nested array instead of one-dimensional array.
   */
  private function stringify($parameters) {
    $string = '';
    $items = array_filter($parameters);

    foreach ($items as $key => $value) {
      $string .= '|' . $key . '=' . $value;
    }

    return $string;
  }
}

