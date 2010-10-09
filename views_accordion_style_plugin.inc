<?php
// $Id$
/**
 * @file
 * Provide an accordion style plugin for Views. This file is autoloaded by views.
 */

/**
  * Implementation of views_plugin_style().
  */
class views_accordion_style_plugin extends views_plugin_style {
  /**
   * Set default options
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['use-grouping-header'] = array('default' => 0);
    $options['collapsible'] = array('default' => 0);
    $options['row-start-open'] = array('default' => 0);
    $options['animated'] = array('default' => 'slide');
    $options['autoheight'] = array('default' => 1);
    $options['event'] = array('default' => 'click');
    $options['fillspace'] = array('default' => 0);
    $options['navigation'] = array('default' => 0);
    $options['clearstyle'] = array('default' => 0);

    /*
     * @TODO:
     * 1. Figure out how to provide support for jQuery UI themes
     * 2. Provide support for missing accordion options:
     *    disabled
     *    icons
     *    navigationFilter
     * See http://jqueryui.com/demos/accordion/
     */
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['grouping']['#prefix'] = '<div class="form-item">'. t('<strong>IMPORTANT:</strong> The <em>first field</em> in order of appearance <em>will</em> be the one used as the "header" or "trigger" of the accordion action.') .'</div>';
     // available valid options for grouping (used for use-grouping-header #dependency)
    foreach ($this->display->handler->get_handlers('field') as $field => $handler) {
      $options[] = $field;
    }
    // Find out how many items the display is currently configured to show (row-start-open)
    $maxitems = $this->display->handler->get_option('items_per_page');
    $maxitems = ($maxitems == 0) ? 10 : $maxitems;  // if items_per_page is set to unlimitted (0), 10 rows will be what the user gets to choose from.

    // Setup our array of options for choosing which row should start opened (row-start-open)
    $rsopen_options = array();
    for ($i = 1; $i <= $maxitems; $i++){
      $rsopen_options[] = t('Row ') . $i;
    }
    $rsopen_options['none'] = t('None');

    $animated_options = array(
      'none' => t('None'),
      'slide' => t('Slide'),
      'bounceslide' => t('Bounceslide'),
      // @TODO: figure out what other animations core comes with that we can use with the accordion
    );

    $form['use-grouping-header'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use the group header as the Accordion header'),
      '#default_value' => $this->options['use-grouping-header'],
      '#description' => t('If checked, the Group\'s header will be used to open/close the accordion.'),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-style-options-grouping' => $options),
    );
    $form['row-start-open'] = array(
      '#type' => 'select',
      '#title' => t('Row to display opened on start'),
      '#default_value' => $this->options['row-start-open'],
      '#description' => t('Choose which row should start opened when the accordion first loads. If you want all to start closed, choose "None", and make sure to have "Allow for all rows to be closed" on below.'),
      '#options' => $rsopen_options,
    );
    $form['collapsible'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow for all rows to be closed'),
      '#default_value' => $this->options['collapsible'],
      '#description' => t('If you check this on, when the user clicks on an opened item, it will close.'),
    );
    $form['animated'] = array(
      '#type' => 'select',
      '#title' => t('Animation effect'),
      '#default_value' => $this->options['animated'],
      '#description' => t('Choose what animation effect you would like to see, or "None" to disable it.'),
      '#options' => $animated_options,
    );
     $form['autoheight'] = array(
      '#type' => 'checkbox',
      '#title' => t('Autoheight'),
      '#default_value' => $this->options['autoheight'],
      '#description' => t('If set, the highest content part is used as height reference for all other parts. Provides more consistent animations.'),
    );
    $form['event'] = array(
      '#type' => 'select',
      '#title' => t('Event'),
      '#default_value' => $this->options['event'],
      '#description' => t('The event on which to trigger the accordion.'),
      '#options' => array(
        'click' => t('Click'),
        'mouseover' => t('Mouseover'),
      ),
    );
    $form['fillspace'] = array(
      '#type' => 'checkbox',
      '#title' => t('Fillspace'),
      '#default_value' => $this->options['fillspace'],
      '#description' => t('If set, the accordion completely fills the height of the parent element. Overrides autoheight.'),
    );
    $form['navigation'] = array(
      '#type' => 'checkbox',
      '#title' => t('Navigation'),
      '#default_value' => $this->options['navigation'],
      '#description' => t('If set, looks for the anchor that matches location.href and activates it. Great for href-based state-saving. Use navigationFilter to implement your own matcher.'),
    );
    $form['clearstyle'] = array(
      '#type' => 'checkbox',
      '#title' => t('Clearstyle'),
      '#default_value' => $this->options['clearstyle'],
      '#description' => t("If set, clears height and overflow styles after finishing animations. This enables accordions to work with dynamic content. Won't work together with autoHeight."),
    );
  }

  /**
   * Render the display in this style.
   */
  function render() {
    $output = parent::render();

    // dpm($this->display->handler->get_option('fields')); // for dev-troubleshooting

    drupal_add_library('system', 'ui.accordion');
    drupal_add_js(drupal_get_path('module', 'views_accordion') .'/views-accordion.js');

    // Add the appropiate effect library if necessary
    // @TODO: figure out what other animations core comes with that we can use with the accordion
    $effect = $this->options['animated'];
    if(($effect !== 'none') && ($effect !== 'slide')){
      switch($effect) {
          case 'bounceslide':
            $library = 'effects.bounce';
            break;
      }
      if(isset($library)){
        drupal_add_library('system', $library);
      }
    }

    // Preparing the js variables and adding the js to our display
    // we do it here so we dont have it run once every group
    $view_settings['collapsible'] = $this->options['collapsible'];
    $view_settings['rowstartopen'] = ($this->options['row-start-open'] == 'none') ? FALSE : $this->options['row-start-open'] + 0; // forces it to add it as an integrer instead of a string - ugly hack?
    $view_settings['animated'] = ($this->options['animated'] == 'none') ? FALSE : $this->options['animated'];
    $view_settings['autoheight'] = $this->options['autoheight'];
    $view_settings['event'] = $this->options['event'];
    $view_settings['fillspace'] = $this->options['fillspace'];
    $view_settings['navigation'] = $this->options['navigation'];
    $view_settings['clearstyle'] = $this->options['clearstyle'];

    $view_settings['grouping'] =  $this->options['grouping'] ? 1 : 0;

    $view_settings['display'] =  $this->view->current_display;
    $view_settings['viewname'] = $this->view->name;
    $view_settings['usegroupheader'] = $view_settings['grouping'] ? $this->options['use-grouping-header'] : 0;

    $accordion_id = 'views-accordion-'. $this->view->name .'-'. $this->view->current_display;

    if ($view_settings['usegroupheader'] == 1) {
      $view_settings['header'] = 'h3.'. $accordion_id .'-header';
    }

    // Used to get the first field to be used as the accordion header
    if ($view_settings['usegroupheader'] == 0) {
      $i = 0;
      foreach ($this->view->field as $id => $value) {
        if (($i == 0) && ($value->options['exclude'] == 0)) {
          $view_settings['header'] = 'views-field-'. str_replace('_' , '-' , $id);
          $i++;
        }
      }
    }

    drupal_add_js(array('views_accordion' => array($accordion_id => $view_settings)), 'setting');

    return $output;
  }
}