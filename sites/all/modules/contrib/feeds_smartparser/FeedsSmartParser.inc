<?php

/**
 * @file
 * Contains FeedsSmartParser and related classes.
 */

/**
 * A smart parser for html using http://code.fivefilters.org/php-readability/
 */
class FeedsSmartParser extends FeedsParser {
  /**
   * Implements FeedsParser::parse().
   */
  public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {

    feeds_include_library('common_syndication_parser.inc', 'common_syndication_parser');
    $feed = common_syndication_parser_parse($fetcher_result->getRaw());
    $result = new FeedsParserResult();
    $result->title = $feed['title'];
    $result->description = $feed['description'];
    $result->link = $feed['link'];
    require_once 'Readability.php';
    if (is_array($feed['items'])) {
      foreach ($feed['items'] as $item) {
        if (isset($item['description']) && isset($item['url'])) {
          $url = $item['url'];
          $html = file_get_contents($url);
          //I use following code to convert encode, which is very usefull in Chinese.
          $encode = mb_detect_encoding($html, array("ASCII","GB2312","GBK","UTF-8","BIG5"));
          if($encode == 'UTF-8'){
            //if($tmp){
          }else{
            $html = iconv($encode, "UTF-8//IGNORE", $html);
          }

          if (function_exists('tidy_parse_string')) {
            $tidy = tidy_parse_string($html, array(), 'UTF8');
            $tidy->cleanRepair();
            $html = $tidy->value;
          }
          $readability = new Readability($html);
          $read = $readability->init();
          if($read){
            $content = $readability->getContent()->innerHTML;
            if (function_exists('tidy_parse_string')) {
              $tidy = tidy_parse_string($content, array('indent'=>true, 'show-body-only' => true), 'UTF8');
              $tidy->cleanRepair();
              $content = $tidy->value;
            }
            $item['description'] = $content;
          }
        }

        if (isset($item['geolocations'])) {
          foreach ($item['geolocations'] as $k => $v) {
            $item['geolocations'][$k] = new FeedsGeoTermElement($v);
          }
        }
        $result->items[] = $item;
      }
    }
    return $result;
  }

  /**
   * Implements FeedsParser::getMappingSources().
   */
  public function getMappingSources() {
    return array(
      'title' => array(
        'name' => t('Title'),
        'description' => t('Title of the html.'),
      ),
      'link' => array(
        'name' => t('Link'),
        'description' => t('Mirror of the html.'),
      ),
      'description' => array(
        'name' => t('Description'),
        'description' => t('Description of the feed item.'),
      ),
      'author_name' => array(
        'name' => t('Author name'),
        'description' => t('Name of the feed item\'s author.'),
      ),
      'timestamp' => array(
        'name' => t('Published date'),
        'description' => t('Published date as UNIX time GMT of the feed item.'),
      ),
      'url' => array(
        'name' => t('Item URL (link)'),
        'description' => t('URL of the feed item.'),
      ),
      'guid' => array(
        'name' => t('Item GUID'),
        'description' => t('Global Unique Identifier of the feed item.'),
      ),
      'tags' => array(
        'name' => t('Categories'),
        'description' => t('An array of categories that have been assigned to the feed item.'),
      ),
      'geolocations' => array(
        'name' => t('Geo Locations'),
        'description' => t('An array of geographic locations with a name and a position.'),
      ),
    ) + parent::getMappingSources();
  }
}
