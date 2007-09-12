<?php
// $Id: date_views.inc,v 1.2 2007/09/07 18:58:03 karens Exp $

/**
 * Views field filters
 */
function date_views_filters($field) {
  switch ($field['type']) {
    case(DATE_ISO):
      $handler = 'date_views_filter_handler';
      $ymd_handler = 'date_views_handler_filter_ymd';
      break;
    case(DATE_UNIX):
      $handler = 'date_views_timestamp_filter_handler';
      $ymd_handler = 'date_views_timestamp_handler_filter_ymd';
      break;
  }

  $format = $field['widget']['input_format'];

  // use this to default to current time
  $current = array('' => '<all>', 'now' => t('now'));
  $months  = ($current + drupal_map_assoc(range(1, 12), 'map_month'));
  $days    = ($current + drupal_map_assoc(range(1, 31)));

  $operator = array(
        '=' => t('is equal to'),
        '<>' => t('is not equal to'),
        '>' => t('greater than'),
        '>=' => t('greater than or equal to'),
        '<' => t('less than'),
        '<=' => t('less than or equal to'),
        );

  $filters = array(
    'default' => array(
      'name' => t(DATE_ISO),
      'operator' => $operator,
      'value' => date_views_handler_filter_date_value_form($field),
      'option' => 'string',
      'handler' => $handler,
      'type' => 'DATE',
      'extra' => array('column' => 'value', 'field' => $field),
      'help' => t('This filter allows events to be filtered by their date. Enter dates in the format: %format. Enter \'now\' to use the current time. You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. If you have the jscalendar module from jstools installed, you can use a popup date picker here.', array('%format' => $format)),
      ),
    'year' => array(
      'name' => t('Year'),
      'operator' => $operator,
      'handler' => $handler,
      'option' => 'string',
      'type' => 'YEAR',
      'extra' => array('column' => 'value', 'field' => $field),
      'help' => t('Filter by year. Enter \'now\' to use the current year.  You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. '),
      ),
    'month' => array(
      'name' => t('Month'),
      'operator' => $operator,
      'list' => $months,
      'list-type' => 'select',
      'handler' => $handler,
      'option' => 'string',
      'type' => 'MONTH',
      'extra' => array('column' => 'value', 'field' => $field),
      'help' => t('Filter by month. Enter \'now\' to use the current month.  You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. '),
      ),
    'day' => array(
      'name' => t('Day'),
      'operator' => $operator,
      'list' => $days,
      'list-type' => 'select',
      'handler' => $handler,
      'option' => 'string',
      'type' => 'DAY',
      'extra' => array('column' => 'value', 'field' => $field),
      'help' => t('Filter by day. Enter \'now\' to use the current day.  You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. '),
      ),
    );

    // Differentiate from and to dates with a prefix that is not likely to ever be used normally.
    if ($field['todate']) {
      $filters2 = array(
        'to|default' => array(
          'name' => t('To Date'),
          'operator' => $operator,
          'value' => date_views_handler_filter_date_value_form($field),
          'option' => 'string',
          'handler' => $handler,
          'type' => 'DATE',
          'extra' => array('column' => 'value2', 'field' => $field),
          'help' => t('This filter allows events to be filtered by their date. Enter dates in the format: %format. Enter \'now\' to use the current time. You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. If you have the jscalendar module from jstools installed, you can use a popup date picker here.', array('%format' => $format)),
          ),
        'to|year' => array(
          'name' => t('To Year'),
          'operator' => $operator,
          'handler' => $handler,
          'option' => 'string',
          'type' => 'YEAR',
          'extra' => array('column' => 'value2', 'field' => $field),
          'help' => t('Filter by year. Enter \'now\' to use the current year.  You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. '),
          ),
        'to|month' => array(
          'name' => t('To Month'),
          'operator' => $operator,
          'list' => $months,
          'list-type' => 'select',
          'handler' => $handler,
          'option' => 'string',
          'type' => 'MONTH',
          'extra' => array('column' => 'value2', 'field' => $field),
          'help' => t('Filter by month. Enter \'now\' to use the current month.  You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. '),
          ),
        'to|day' => array(
          'name' => t('To Day'),
          'operator' => $operator,
          'list' => $days,
          'list-type' => 'select',
          'handler' => $handler,
          'option' => 'string',
          'type' => 'DAY',
          'extra' => array('column' => 'value2', 'field' => $field),
          'help' => t('Filter by day. Enter \'now\' to use the current day.  You may enter a delta (in seconds) to the option that will be added to the time; this is most useful when combined with now. '),
          ),
       );
       $filters += $filters2;
    }
    return $filters;
}

/**
 *  Handler for date filter.
 */

function date_views_timestamp_filter_handler($op, $filter, $filterinfo, &$query) {
  // this is just a wrapper function that sets the date type
  return date_views_filter_handler($op, $filter, $filterinfo, $query, 'int');
}
function date_views_filter_handler($op, $filter, $filterinfo, &$query, $field_type = 'iso') {
  date_api_load('date_api_sql.inc');

  $value = $filter['value'];
  $type = $filterinfo['type'];

  // Do some validation of input values before creating any SQL.
  switch ($type) {
    case 'YEAR':
    case 'MONTH':
    case 'DAY':
      if (empty($value) || !($value == 'now' || is_numeric($value))) {
        return;
      }
      break;
    default:
      if ($field_type == 'int' && (empty($value) || !($value == 'now' || date_is_valid($value, DATE_UNIX)))) {
        return;
      }
      elseif (empty($value) || !($value == 'now' || date_is_valid($value, DATE_ISO))) {
        return;
      }
      break;
  }

  $field   = $filterinfo['extra']['field'];
  $column = $filterinfo['extra']['column'];
  $formats = date_get_formats($field);
  $db_info = $filterinfo['content_db_info'];
  $table   = 'node_data_'. $field['field_name'];
  $table_field   = $db_info['columns'][$column]['column'];

  // When using NOW(), must adjust out the server zone adj which may not match
  // the offset we really want, and add back the right offset.
  // This will be necessary any time the server timezone does not match the site or date zone
  // because the server is going to adjust the value of NOW() for the server timezone.
  switch ($type) {
    case('DATE'):
      $date = $value == 'now' ? date_sql('NOW', 'NOW()', $field_type, intval(date_views_offset($field) + $filter['options'])) : "'". str_replace('T', ' ', date_custom2iso($value, $formats['input']['text'])) ."'";
      break;
    default:
      $date = $value == 'now' ? date_sql($type, "NOW()", $field_type, intval(date_views_offset($field) + $filter['options'])) : $value;
      break;
  }
  $query->ensure_table($table);
  $query->add_where(date_sql($type, $table .".". $table_field, $field_type, date_views_offset($field)) .' '. $filter['operator'] .' '. $date);
}

/**
 * Provide a form gadget for dates.
 */
function date_views_handler_filter_date_value_form($field) {
  include_once(drupal_get_path('module', 'date') .'/date_elements.inc');
  $format = strtr($field['widget']['input_format'], date_jscalendar_replacements());

  $form = array(
    '#type' => 'textfield',
    '#attributes' => array('class' => 'jscalendar'),
    '#jscalendar_ifFormat' => $format,
    '#jscalendar_showsTime' => date_has_time($field['granularity']) ? 'true' : 'false',
    '#jscalendar_timeFormat' => $formats['input']['am_pm'] ? '12' : '24',
  );
  return $form;
}

/**
 *  Return field or value for a timezone offset.
 */
function date_views_offset($field) {
  switch ($field['tz_handling']) {
  case(DATE_ISO):
    $offset = 'node_data_'. $field['field_name'] .'.'. $field['field_name'] .'_offset';
    break;
  case('site'):
    $offset = variable_get('date_default_timezone', 0);
    break;
  default:
    $offset = '';
    break;
  }
  return $offset;
}

/**
 * Implementation of hook_views_arguments().
 */
function date_views_arguments() {
  $field_types = _content_field_types();
  $arguments = array();
  foreach (content_fields() as $field) {
    if ($field['type'] == DATE_ISO || $field['type'] == DATE_UNIX) {
      $db_info = content_database_info($field);
      $argument = array();
      $argument['name'] = $field_types[$field['type']]['label'] .($field['todate'] ? t(': From ') : ': '). $field['widget']['label'] .' ('. $field['field_name'] .')';
      $argument['handler'] = 'date_views_argument_range_handler';
      $argument['help'] = t("Defines an argument to filter for dates within a range, in the format 'YYYY-MM-DD--YYYY-MM-DD'. Many other options can be used in arguments. See !link for other examples.", array('!link' => l(t('help'), 'admin/help/date')));
      $argument['option'] = 'date_range_arg_options';
      $arguments['content: '. $field['field_name']] = $argument;

      if ($field['todate']) {
        $argument = array();
        $argument['name'] = $field_types[$field['type']]['label'] .t(': To '). $field['widget']['label'] .' ('. $field['field_name'] .')';
        $argument['handler'] = 'date_views_argument_range_handler';
        $argument['help'] = t("Defines an argument to filter for dates within a range, in the format 'YYYY-MM-DD--YYYY-MM-DD'. Many other options can be used in arguments. See !link for other examples.", array('!link' => l(t('help'), 'admin/help/date')));
        $argument['option'] = 'date_range_arg_options';
        $arguments['content: to|'. $field['field_name']] = $argument;
      }
    }
  }
  return $arguments;
}

/**
 *
 * Flexible date range argument handler
 *
 * Argument is based on ISO 8601 date duration and time interval standards
 *
 * See http://en.wikipedia.org/wiki/ISO_8601#Week_dates for definitions of ISO weeks
 * See http://en.wikipedia.org/wiki/ISO_8601#Duration for definitions of ISO duration and time interval
 *
 * Argument expects a value like 2006-01-01--2006-01-15, or 2006-W24, or @P1W
 * Separate from and to dates or date and period with a double hyphen (--)
 *
 * From and to dates in argument are ISO dates, but can be shortened and missing parts will be added
 * Omitted parts of ISO dates will be assumed to be the first possible (for the from date)
 * or the last possible (for the to date) value in that time period
 *
 * The 'to' portion of the argument can be eliminated if it is the same as the 'from' portion
 * Use @ instead of a date to substitute in the current date and time.
 *
 * Use periods (P1H, P1D, P1W, P1M, P1Y) to get next hour/day/week/month/year from now
 * Use date before P sign to get next hour/day/week/month/year from that date
 *
 * This module does not currently handle the option of using a period with an end date,
 * only a start date followed by a period.
 *
 * The groupby selector values are used only if a summary view of the argument is requested
 * possible values are by year, by month, by week, by day, and by hour
 *
 * if summaries are used, navigating to the view with no argument will display subtotals for the query,
 * grouped by the selected range, with a link to the complete query for each range
 *
 */
function date_views_timestamp_argument_range_handler($op, &$query, $argtype, $arg = '') {
  // this is just a wrapper function that sets the date type
  return date_views_argument_range_handler($op, $query, $argtype, $arg, 'int');
}
function date_views_argument_range_handler($op, &$query, $argtype, $arg = '', $field_type = 'iso') {
  static $format;

  date_api_load('date_api_sql.inc');

  $name = explode(':', is_array($argtype) ? $argtype['type'] : $argtype);
  $tofield_name = trim($name[1]);
  $field_name = substr($tofield_name, 0, 3) == 'to|' ? substr($tofield_name, 3) : $tofield_name;
  if ($field_name == $tofield_name) {
    $value = 'value';
  }
  else {
    $value = 'value2';
  }

  $field      = content_fields($field_name);
  $db_info    = content_database_info($field);
  $value      = $db_info['columns'][$value]['column'];
  $timezone   = $db_info['columns']['timezone']['column'];
  $table      = 'node_data_'. $field['field_name'];

  $offset = date_views_offset($field);

  switch ($op) {
    case 'summary':

      // in the summary operation, the arg contains the selected option
      $groupby = $arg;

      switch ($groupby) {
        case('year'):
          $format = 'Y';
          $fieldinfo['field'] = date_sql_concat(array(
              date_sql('YEAR', $table .'.'. $value, $field_type, $offset),
              ));
          break;
        case('month'):
          $format = 'F Y';
          $fieldinfo['field'] = date_sql_concat(array(
              date_sql('YEAR', $table .'.'. $value, $field_type), "'-'",
              date_sql_pad(date_sql('MONTH', $table .'.'. $value, $field_type, $offset)),
              ));
          break;
        case('day'):
          $format = 'F j Y';
          $fieldinfo['field'] = date_sql_concat(array(
              date_sql('YEAR', $table .'.'. $value, $field_type), "'-'",
              date_sql_pad(date_sql('MONTH', $table .'.'. $value, $field_type, $offset)), "'-'",
              date_sql_pad(date_sql('DAY', $table .'.'. $value, $field_type, $offset)),
              ));
          break;
        case('hour'):
          $format = 'F j Y - H';
          $fieldinfo['field'] = date_sql_concat(array(
              date_sql('YEAR', $table .'.'. $value, $field_type), "'-'",
              date_sql_pad(date_sql('MONTH', $table .'.'. $value, $field_type, $offset)), "'-'",
              date_sql_pad(date_sql('DAY', $table .'.'. $value, $field_type, $offset)), "'T'",
              date_sql_pad(date_sql('HOUR', $table .'.'. $value, $field_type, $offset)),
              ));
          break;
        case('week'):
          $format = 'F j Y (W)';
          $fieldinfo['field'] = date_sql_concat(array(
              date_sql('YEAR', $table .'.'. $value, $field_type, $offset), "'-W'",
              date_sql('WEEK', $table .'.'. $value, $field_type, $offset),
              ));
          break;
      }
      $fieldinfo['fieldname'] = 'range';

      $query->ensure_table($table);
      $query->add_field($value, $table);
      return $fieldinfo;
      break;

    case 'link':
      // links are created from date range groupings
      $time = $query->$value > '' ? ($field_type == 'iso' ? date_convert($query->$value, DATE_ISO, DATE_UNIX) : $query->$value) : '';
      return l(date_date($format, $time), $arg .'/'. $query->range);

    case 'sort':
      break;

    case 'filter':
      $range = date_views_date_range($arg);
      $query->ensure_table($table);
      $query->add_field($value, $table);
      $query->add_where(date_sql('DATE', $table .'.'. $value, $field_type, $offset) .">='". str_replace('T', ' ', $range[0]) ."'");
      $query->add_where(date_sql('DATE', $table .'.'. $value, $field_type, $offset) ."<='". str_replace('T', ' ', $range[1]) ."'");
      break;

    case 'title':
      $item = array(key($db_info['columns']) => $query);
      return content_format($field, $item, 'default');
  }
}

function date_views_date_range($arg) {
  if (stristr($arg, 'P')) {
    // for a date plus value, get the min and max values
    $range = date_plus_period_range($arg);
    $min_date = $range[0];
    $max_date = $range[1];
  }
  elseif (stristr($arg, '-W') && !stristr($arg, '--')) {
    // for a specified week, get the min and max values
    $range = date_iso_week_range($arg);
    $min_date = $range[0];
    $max_date = $range[1];
  }
  else {
    // for all other get the date range from the supplied argument
    $range = (array) explode('--', $arg);
    $min_date = date_range_value($range[0], 'min');
    $max_date = date_range_value(($range[1] ? $range[1] : $range[0]), 'max');
  }
  return array($min_date, $max_date);
}
/**
 *  Validate and pad date range argument element
 *
 *  @param $value - a full or partial ISO date from an argument
 *  @param $value_type - min or max, whether it is the from or the to part of the range
 *  @return complete, validated ISO date
 */
function date_range_value($value, $value_type = 'min') {
  $now   = date_date(DATE_FORMAT_ISO, time());
  if (trim($value) == '@' || trim($value) == '') return $now;

  switch (strlen($value)) {
  case(4):
    $return = ($value_type == 'min' ? $value .'-01-01T00:00:00' : $value .'-12-31T23:59:59');
    break;
  case(7):
    $return = ($value_type == 'min' ? $value .'-01T00:00:00' : $value .'-31T23:59:59');
    break;
  case(10):
    $return = ($value_type == 'min' ? $value .'T00:00:00' : $value .'T23:59:59');
    break;
  case(13):
    $return = ($value_type == 'min' ? $value .':00:00' : $value .':59:59');
    break;
  case(16):
    $return = ($value_type == 'min' ? $value .':00' : $value .':59');
    break;
  case(19):
    $return = $value;
    break;
  default:
    $return = $now;
  }
  // use regex to test for validity of constructed date
  return (preg_match(DATE_REGEX_ISO, $return) ? $return : $now);
}

/**
 *  Compute min and max dates for a week
 *
 *  based on ISO weeks, which start counting on the first Monday in a week that
 *  has at least 4 days in the current year
 *
 *  January 4 is always in the first ISO week of the year. Move ahead by the
 *  number of ISO weeks to find a date in the ISO week. Find the Monday of the
 *  ISO week for the first second of the week, move ahead 1 week and back
 *  1 second to find last second of the week.
 *
 *  @value - an argument in the format 2006-W20 (year + -W + week number)
 *  @return an array of ISO dates representing the first and last day in the week
 */
function date_iso_week_range($value) {
  $parts = explode('-W', $value);
  $year = $parts[0];
  $week = $parts[1];
  $date = date_create($year .'-01-04 00:00:00', timezone_open('UTC'));
  date_modify($date, '+'. $week .' weeks');
  date_modify($date, '-1 Monday');
  $min_date = date_format($date, DATE_FORMAT_ISO);
  date_modify($date, '+1 week');
  date_modify($date, '-1 second');
  $max_date = date_foramt($date, DATE_FORMAT_ISO);
  return array($min_date, $max_date);
}

/**
 *  Compute min and max dates for a P value
 *
 *  Min date is whatever is to the left of the period sign, defaults to
 *  current date. Use ical module to parse the period. Set end date to
 *  1 second before the end of the period, since we use <= operator.
 *
 *  @value = an argument in the format (start date)P#(period type)
 *     where (period type) can be Y (year), M (month), D (day), W (week), H (hour)
 *     i.e. P1Y or P90D or P1Y3M2D4H
 *  @return an array of ISO dates representing the first and last day in the range
 */
function date_plus_period_range($value) {
  $value = str_replace('--P', 'P', $value);
  $range = explode('P', $value);
  $min_date = date_range_value($range[0], 'min');
  date_api_load('date_api_ical.inc');
  $parsed = date_ical_parse_duration('', $range[1]);
  $max_date = date_create($min_date, timezone_open('UTC'));
  date_modify($max_date, '+'. intval($parsed['DURATION'] - 1) .' seconds');
  return array($min_date, date_format($max_date, DATE_FORMAT_ISO));
}

/**
 *  Define groupby options for date range summaries
 */
function date_range_arg_options() {
  return array(
    'year'  => t('summarize by year'),
    'month' => t('summarize by month'),
    'day'   => t('summarize by day'),
    'week'  => t('summarize by week'),
    'hour'  => t('summarize by hour')
    );
}

//============================== Date Browser ================================//

/**
 * Works only with views that use the date range argument
 * Adds this/next period navigation links to a date argument range view
 * Adds 'week of XXX', 'month of XXX' headings to views and blocks
 * Defaults blocks and views w/out arguments to current period to start paging
 * Choose period increments by selecting the option value of date range argument
 *  (year, month, week, day, hour)
 */

/**
 * Implementation of hook_views_style_plugins()
 */
function date_views_style_plugins() {

  $items = array();
  $items['date_views_browser'] = array(
    'name' => t('Date: Date Browser'),
    'theme' => 'date_views_browser_full_view',
    'summary_theme' => 'date_views_browser_summary_view',
    'needs_fields' => true,
    'needs_table_header' => true,
    'validate' => 'date_views_browser_get_views',
    'even_empty' => true,
  );
  return $items;
}
/**
 * Implementation of hook_views_pre_view()
 */
function date_views_pre_view($view, $items) {
  $date_views_browser_views = date_views_browser_get_views();
  if ($view->build_type == 'page' && in_array($view->name, array_keys($date_views_browser_views))) {
    // make sure the navigation link gets added to empty pages
    if (sizeof($items) == 0 && $view->build_type == 'page') {
      $period = $date_views_browser_views[$view->name]->options;
      return date_views_browser_navigation($view, $period);
    }
  }
}

/**
 *  Implementation of hook_views_query()
 *  Used to make sure view defaults to current date if no date selected
 */
function date_views_query_alter(&$query, &$view) {
  $date_views_browser_views = date_views_browser_get_views();
  if (in_array($view->name, array_keys($date_views_browser_views))) {
    $path = explode('/', $view->url);
    $pos = sizeof($path);
    if ($view->build_type == 'block' || arg($pos) == '') {
      $arg = NULL;
    }
    else {
      $arg = arg($pos);
    }
    if ($arg == NULL) {
      // if no argument specified, add the current date range to the query
      $arg        = date_views_browser_period_arg($arg, $view->argument[0]['options']);
      $name       = explode(':', $view->argument[0]['type']);
      $field_name = trim($name[1]);
      $field      = content_fields($field_name);
      $field_type = $field['type'] == DATE_UNIX ? 'int' : 'iso';
      $db_info    = content_database_info($field);
      $value      = $db_info['columns']['value']['column'];
      $table      = 'node_data_'. $field['field_name'];
      $offset     = date_views_offset($field);
      if ($range  = date_views_date_range($arg)) {
        $query->ensure_table($table);
        $query->add_field('nid', 'node');
        $query->add_field($value, $table);
        $query->add_where(date_sql('DATE', $table .'.'. $value, $field_type, $offset) .">='". str_replace('T', ' ', $range[0]) ."'");
        $query->add_where(date_sql('DATE', $table .'.'. $value, $field_type, $offset) ."<='". str_replace('T', ' ', $range[1]) ."'");
      }
    }

  }
}
/**
 * Find all the views that qualify for date browser treatment
 *
 * @param $view
 *   if called from Views UI validation, $edit_view will be populated,
 *   otherwise it will be empty. Use that to tell if the list
 *   needs to be refreshed.
 */
function date_views_browser_get_views($edit_view = FALSE) {
  static $date_views_browser_views;

  if (!isset($date_views_browser_views) || $view) {
    $date_views_browser_views = array();
    $arguments = date_views_arguments();
    $argument_list = "'". implode("','", array_keys($arguments)) ."'";
    if (!$argument_list) {
      return array();
    }
    $result = db_query("SELECT arg.*, view.name FROM {view_argument} arg INNER JOIN {view_view} view ON arg.vid=view.vid WHERE arg.type IN ($argument_list) AND view.page_type='date_views_browser'");
    while ($view = db_fetch_object($result)) {
      $date_views_browser_views[$view->name] = $view;
    }
    // Make sure the view being edited gets added to the list
    $date_views_browser_views[$edit_view->name] = $edit_view;
  }
  return $date_views_browser_views;
}

/**
 *  Return the correct period for the date range argument
 */
function date_views_browser_period($period = 'month') {
  switch ($period) {
  case('year'):
    return 'P1Y';
  case('week'):
    return 'P1W';
  case('day'):
    return 'P1D';
  case('hour'):
    return 'P1H';
  default:
    return 'P1M';
  }
}
/**
 *  Format an argument for the date range
 */
function date_views_browser_period_arg($arg = NULL, $period = 'month') {
  $range = date_views_date_range($arg);
  $date = date_create($range[0], timezone_open('UTC'));
  switch ($period) {
  case('week'):
    return date_format($date, 'Y-m-d') . date_views_browser_period($period);
  case('year'):
    return date_format($date, 'Y') . date_views_browser_period($period);
  case('day'):
    return date_format($date, 'Y-m-d') . date_views_browser_period($period);
  case('hour'):
    return date_format($date, 'Y-m-d\TH') . date_views_browser_period($period);
  default:
    return date_format($date, 'Y-m') . date_views_browser_period($period);
  }
}
/**
 *  Return label for current date range
 */
function date_views_browser_period_label($arg = NULL, $period = 'month') {
  $range = date_views_date_range($arg);
  $date = date_create($range[0], timezone_open('UTC'));
  return theme('date_views_browser_period_label', $period, date_format($date, DATE_UNIX));
}

/**
 *  Navigation links for the full view
 */
function date_views_browser_navigation($view, $period) {
  $path = explode('/', $view->url);
  $pos = sizeof($path);
  if (arg($pos) == '') {
    $arg = NULL;
  }
  else {
    $arg = arg($pos);
  }
  $range = date_views_date_range($arg);
  $date = date_create($range[0], timezone_open('UTC'));
  $prev_date = date_prev($date, 1, $period, DATE_OBJECT);
  $next_date = date_next($date, 2, $period, DATE_OBJECT);
  $prev = $view->url .'/'. $prev_date . date_views_browser_period($period);
  $next = $view->url .'/'. $next_date . date_views_browser_period($period);
  $label = date_views_browser_period_label($arg, $period);
  return theme('date_views_browser_navigation', $label, $period, $prev, $next, $view);
}

//============================== THEMES ================================//

/**
 *  Theme for the current period label name
 */
function theme_date_views_browser_period_label($period, $date) {
  switch ($period) {
    case 'year':
      $format = 'Y';
      break;
    case 'month':
      $format = 'F Y';
      break;
    case 'day':
      // Use just the year/month/day part of the short date format.
      $format = date_limit_format(variable_get('date_format_short', 'm/d/Y - H:i'), array('M', 'D', 'Y'));
      break;
    case 'hour':
      // Use just the year/month/day/hour part of the short date format.
      $format = date_limit_format(variable_get('date_format_short', 'm/d/Y - H:i'), array('M', 'D', 'Y', 'H'));
  }
  return t('%period of %date', array('%period' => ucwords($period), '%date' => date_format_date($date, 'custom', $format, 'UTC')));
}
/**
 *  Theme for page navigation
 */
function theme_date_views_browser_navigation($label, $period, $prev, $next, $view) {
    drupal_add_css(drupal_get_path('module', 'date_api') .'/date.css');
    $output  = '<div class="book-navigation date-browser-navigation">';
    $output .= '<div class="page-links">';
    $output .= l(t('‹ prev !period  ', array('!period' => $period)), $prev, array('class' => 'page-previous'));
    $output .= '<h3 class="date-browser-label"><span class="page-up">'. $label .'</span></h3>';
    $output .= l(t('  next !period  ›', array('!period' => $period)), $next, array('class' => 'page-next'));
    $output .= '</div></div>';
    return $output;
  return $output;
}
/**
 * Display a summary version of a view.
 */
function theme_date_views_browser_summary_view($view, $type, $level, $nodes, $args) {
  return theme('date_views_browser_full_view', $view, $nodes, $type);
}
/**
 *  View, themed so it can be overridden
 *  $display -- views_view_list, views_view_table, views_view_teasers, views_view_nodes
 */
function theme_date_views_browser_full_view($view, $nodes, $type) {

  $teasers = true;
  $links   = true;
  drupal_add_css(drupal_get_path('module', 'date_api') .'/date.css');

  $date_views_browser_views = date_views_browser_get_views();
  $period = $date_views_browser_views[$view->name]->options;
  switch ($type) {
    case('block'):
      $arg = date_views_browser_period_arg(NULL, $view->argument[0]['options']);
      if ($view->url) $url = $view->url .'/'. $arg;
      $output  .= '<h5 class="date-browser-block-label">'. l(date_views_browser_period_label(NULL, $period), $url) .'</h5>';
      $display = 'views_view_list';
      break;
    default:
      $output  .= date_views_browser_navigation($view, $period);
      $display = 'views_view_teasers';
      break;
  }
  $output .= theme($display, $view, $nodes, $type, $teasers, $links);

  return $output;
}