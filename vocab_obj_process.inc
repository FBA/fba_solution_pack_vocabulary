<?php

/**
 * Display Vocab/Geo Object Streams
 */
class FBAVocabSolutionPack {

  private $pid = NULL;

  /**
   * Constructor
   * @param type $pid
   */
  function __construct($pid) {
    $this->pid = $pid;
  }

  /**
   * Show MADS
   * @return type
   */
  function showVocabFormattedObjectDetail() {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'islandora_content_model_forms', 'ObjectLinker');
    global $base_url;

    $item = new fedora_item($this->pid);
    $MADSDom = DOMDocument::loadXML($item->get_datastream_dissemination('MADS'));

    if ($MADSDom != NULL) {
      $authority = trim($MADSDom->getElementsByTagName('authority')->item(0)->nodeValue);
      $variant = trim($MADSDom->getElementsByTagName('variant')->item(0)->nodeValue);

      $notes = $MADSDom->getElementsByTagName('note');
      foreach ($notes as $node) {
        if ($node->hasAttribute('type')) {
          $typeatt = $node->getAttribute('type');
          $noteval = $node->nodeValue;

          switch ($typeatt) {
            case 'scope': {
                $scope = $noteval;
                break;
              }
            case 'definition': {
                $definition = $noteval;
                break;
              }
            case 'definition source': {
                $urlofdefsource = $noteval;
                break;
              }
            case 'approved date': {
                $dateapprovedsolrformat = new DateTime($noteval);
                $dateapproved = $dateapprovedsolrformat->format('d-M-Y');

                break;
              }
            default: {
                // could be "validity"
                break;
              }
          }
        }
      }

      $relateds = $MADSDom->getElementsByTagName('related');
      foreach ($relateds as $node) {
        $relval = trim($node->nodeValue);
        if (!empty($relval)) { //no point in processing null data
          if ($node->hasAttribute('type')) {
            $typeatt = $node->getAttribute('type');

            switch ($typeatt) {
              case 'broader': {
                  $broaderterms[] = $relval;

                  break;
                }
              case 'narrower': {
                  $narrowerterms[] = $relval;
                  break;
                }
              case 'other': {
                  if ($node->hasAttribute('otherType')) {
                    $othertypeatt = $node->getAttribute('otherType');

                    switch ($othertypeatt) {
                      case 'used for': {
                          $usedforterms[] = $relval;
                          break;
                        }
                      case 'use': {
                          $useinsteadterms[] = $relval;
                          break;
                        }
                      case 'preferred unit': {
                          $preferredunits = $relval;
                          break;
                        }
                    }
                  }
                  break;
                }
            }
          }
          else {
            //no type attribute therefore assumption = "related term"
            $relatedterms[] = $relval;
          }
        }
      }
    }

    $output = array(); //this will be the formatted html to be sent to the screen
    //each array entry ends up as new <tr>

    if ($authority) {
      // The custom section for a preferred term
      //DTC117 SF 20131105 Remove this line completely from display
      //$output[] = array('<h1>' . $authority . '</h1>');
      //DTC117
      //Definition
      $formattedhtml = '<p><strong>Definition</strong>';
      if ($urlofdefsource) {
        //URL of Definition Source
        $formattedhtml .= '&nbsp;(<a href="' . $urlofdefsource . '" target="_blank">' . $urlofdefsource . '</a>)';
      }
      $formattedhtml .= '</p>';
      $formattedhtml .= '<p>';
      if ($definition) {
        $formattedhtml .= $definition;
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</p>';
      $formattedhtml .= '<p><strong>Preferred Units:</strong>&nbsp;';
      if ($preferredunits) {
        $formattedhtml .= _get_html_string_of_object_urls($preferredunits);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</p>';

      $formattedhtml .= '<p><strong>Scope Note:</strong><br/>';
      if ($scope) {
        $formattedhtml .= $scope;
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</p>';

      $formattedhtml .= '<table><tr><th>Broader Terms</th><th>Narrower Terms</th><th>Related Terms</th></tr>';
      $formattedhtml .= '<tr valign="top">';
      $formattedhtml .= '<td width="33%">';
      if ($broaderterms) {
        $formattedhtml .= _get_html_string_of_object_urls($broaderterms);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</td>';

      $formattedhtml .= '<td width="33%">';
      if ($narrowerterms) {
        $formattedhtml .= _get_html_string_of_object_urls($narrowerterms);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</td>';

      $formattedhtml .= '<td width="33%">';
      if ($relatedterms) {
        $formattedhtml .= _get_html_string_of_object_urls($relatedterms);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</td>';
      $formattedhtml .= '</tr>';
      $formattedhtml .= '</table>';

      $formattedhtml .= '<p><strong>This term is used for the following terms:</strong>&nbsp;<br/>';
      if ($usedforterms) {
        $formattedhtml .= _get_html_string_of_object_urls($usedforterms);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</p>';

      // Search for reciprocal links in geo collection.

      $linkGeoHeader = 'Related geographic terms';
      $linkGeoSparqlQuery = 'SELECT ?title ?object WHERE { ?object <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> <info:fedora/fba:geopref_CModel>; <fedora-rels-ext:isMemberOfCollection> <info:fedora/fba:geopref_collection>; <http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasCorrelative> <info:fedora/' . $this->pid . '> } ORDER BY ?title LIMIT 30';
      $linkGeoArray = get_link_array($linkGeoSparqlQuery);

      if ($linkGeoArray) {
        $formattedhtml .= '<table><tr><th>' . $linkGeoHeader . '</th></tr>';
        foreach ($linkGeoArray as $linkGeoterm) {
          if ($linkGeoterm['title']) {
            $linkGeotermTitle = $linkGeoterm['title'];
          }
          else {
            $linkGeotermTitle = 'N/A';
          }

          $formattedhtml .= '<tr valign="top">';
          $formattedhtml .= '<td width="100%">';
          $formattedhtml .= '<a href="' . $base_url . '/fedora/repository/' . $linkGeoterm['object'] . '">' . $linkGeotermTitle . '</a>';
          $formattedhtml .= '</td>';
          $formattedhtml .= '</tr>';
        }
        $formattedhtml .= '</table>';

        $formattedhtml .= '</p></div>';
      }
    }
    else {
      // The custom section for a non-preferred term
      //DTC117 SF 20131105 Remove this line completely from display
      //$output[] = array('<h1>' . $variant . '</h1>');
      //DTC117
      // Use Instead
      $formattedhtml .= '<p><strong><span style="color: red;">Use this term instead:</span></strong>&nbsp;' . _get_html_string_of_object_urls($useinsteadterms) . '</p>';
    }
    //Common footer section for both types of terms
    $formattedhtml .= '<p><strong>Approved Date</strong><br/>' . $dateapproved . '</p>';

    if ($authority) {
      /* If a drupal multisite is pointing at one fedora repository,       */
      /* and control over which items are displayed on specific websites   */
      /* is required, include the base URL of all the websites that should */
      /* display links in the following switch-clause, and remove the      */
      /* processing in the 'default:' branch.                              */

      switch ($base_url) {
        case "http://toplevelsite.org":
        case "http://specificsite.org":
          /* To avoid Sparql queries becoming overly complex, when a new     */
          /* website has to be catered for, just add another header/query    */
          /* pair to the array for the particular website.                   */
          /* Replace http://toplevelsite.org with the base URL of the site   */
          /* that is to display all the links and repeat the entire if-clause*/
          /* below for each specific site that should display links for      */
          /* a particular collection. Also set the relevant collection ids,  */
          /* content model ids and predicates.                               */
          /* So, for each website repeat the entire if-clause STARTING HERE: */

          if (!strcmp($base_url, "http://specificsite.org") || !strcmp($base_url, "http://toplevelsite.org") ) {

            if (!strcmp($base_url, "http://toplevelsite.org")) {
              $linkHeader = 'Specific Site items having a keyword relationship to this subject term';
            }
            elseif (!strcmp($base_url, "http://specificsite.org")) {
              $linkHeader = 'Items having a keyword relationship to this subject term';
            }
            $linkQuery = 'SELECT ?title ?object WHERE { ?object <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> ?model; <fedora-rels-ext:isMemberOfCollection> ?collection; <http://specific_predicate> <info:fedora/' . $this->pid . '> FILTER (regex(str(?collection), "^info:fedora/specific_namespace:collection$", "i") && (regex(str(?model), "^info:fedora/specific_namespace:specific_content_model1$", "i") || regex(str(?model), "^info:fedora/specific_namespace:specific_content_model2$", "i") || regex(str(?model), "^info:fedora/specific_namespace:specific_content_model3$", "i")))} ORDER BY $title LIMIT 300';

            $linkSparqlQueries[] = array("header" => $linkHeader, "query" => $linkQuery);
          }

          /* ... AND FINISHING HERE  */
          break;
        default:
          /* This is the default processing that works for the collection id  */
          /* and content model id associated with the standard Islandora      */
          /* PDF Solution Pack. Remove the processing in this branch and      */
          /* follow the instructions above if non-standard processing is      */
          /* required.                                                        */

          $linkHeader = 'PDFs having a keyword relationship to this subject term';
          $linkQuery = 'SELECT ?title ?object WHERE { ?object <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> ?model; <fedora-rels-ext:isMemberOfCollection> ?collection; <http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasKeyword> <info:fedora/' . $this->pid . '> FILTER (regex(str(?collection), "^info:fedora/islandora:sp_pdf_collection$", "i") && regex(str(?model), "^info:fedora/islandora:sp_strict_pdf$", "i"))} ORDER BY $title LIMIT 300';
          $linkSparqlQueries[] = array("header" => $linkHeader, "query" => $linkQuery);
      }

      foreach ($linkSparqlQueries as $linkSparqlQuery) { 
        $linkFormattedHtml = get_link_list($linkSparqlQuery['header'], $linkSparqlQuery['query']);

        if (strlen($linkFormattedHtml) > 0) {
          $customLinkFormattedHtml = '<div style="clear:both">' . $linkFormattedHtml . '</div>';
          $formattedhtml .= $customLinkFormattedHtml;
        }
      }
    }

    $output[] = array($formattedhtml);
    $dataStreamBody = theme('table', $headers, $output);

    $collection_fieldset = array(
      '#collapsible' => FALSE,
      '#value' => $dataStreamBody,
    );
    $tabset = array();
    $tabset['vocabtermdetail'] = array(
      '#type' => 'tabpage',
      '#title' => 'Vocabulary Term Details',
      '#content' => theme('fieldset', $collection_fieldset),
    );

    return $tabset;
  }

  /**
   * Show MADS
   * @return type
   */
  function showGeoFormattedObjectDetail() {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    global $base_url;

    $item = new fedora_item($this->pid);
    $madsdom = DOMDocument::loadXML($item->get_datastream_dissemination('MADS'));

    if ($madsdom != NULL) {
      $authority = trim($madsdom->getElementsByTagName('authority')->item(0)->nodeValue);
      $variant = trim($madsdom->getElementsByTagName('variant')->item(0)->nodeValue);

      $notes = $madsdom->getElementsByTagName('note');
      foreach ($notes as $node) {
        if ($node->hasAttribute('type')) {
          $typeatt = $node->getAttribute('type');
          $noteval = $node->nodeValue;

          switch ($typeatt) {
            case 'scope': {
                $scope = $noteval;
                break;
              }
            case 'definition': {
                $definition = $noteval;
                break;
              }
            case 'definition source': {
                $urlofdefsource = $noteval;
                break;
              }
            case 'approved date': {
                //make the solr format date more readable.
                $dateapprovedsolrformat = new DateTime($noteval);
                $dateapproved = $dateapprovedsolrformat->format('d-M-Y');
                break;
              }
            default: {
                // could be "validity"
                break;
              }
          }
        }
      }

      $relateds = $madsdom->getElementsByTagName('related');
      foreach ($relateds as $node) {
        $relval = trim($node->nodeValue);
        if (!empty($relval)) { //no point in processing null data
          if ($node->hasAttribute('type')) {
            $typeatt = $node->getAttribute('type');

            switch ($typeatt) {
              case 'broader': {
                  $broaderterms[] = $relval;

                  break;
                }
              case 'narrower': {
                  $narrowerterms[] = $relval;
                  break;
                }
              case 'other': {
                  if ($node->hasAttribute('otherType')) {
                    $othertypeatt = $node->getAttribute('otherType');

                    switch ($othertypeatt) {
                      case 'used for': {
                          $usedforterms[] = $relval;
                          break;
                        }
                      case 'use': {
                          $useinsteadterms[] = $relval;
                          break;
                        }
                      case 'preferred unit': {
                          $preferredunits = $relval;
                          break;
                        }
                      case 'geographic feature': {
                          $geographicfeature = $relval;
                          break;
                        }
                      case 'is geographic feature of': {
                          $isgeofeatureof = $relval;
                          break;
                        }
                    }
                  }
                  break;
                }
            }
          }
          else {
            //no type attribute therefore assumption = "related term"
            $relatedterms[] = $relval;
          }
        }
      }
    }

    $output = array(); //this will be the formatted html to be sent to the screen
    //each array entry ends up as new <tr>

    if ($authority) {
      // The custom section for a preferred term
      //DTC117 SF 20131105 Remove this line completely from display
      //$output[] = array('<h1>' . $authority . '</h1>');
      //DTC117
      //Definition
      $formattedhtml = '<p><strong>Definition</strong>';
      if ($urlofdefsource) {
        //URL of Definition Source
        $formattedhtml .= '&nbsp;(<a href="' . $urlofdefsource . '" target="_blank">' . $urlofdefsource . '</a>)';
      }
      $formattedhtml .= '</p>';
      $formattedhtml .= '<p>';
      if ($definition) {
        $formattedhtml .= $definition;
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</p>';
      $formattedhtml .= '<p><strong>Preferred Units:</strong>&nbsp;';
      if ($preferredunits) {
        $formattedhtml .= _get_html_string_of_object_urls($preferredunits);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</p>';

      $formattedhtml .= '<p><strong>Scope Note:</strong><br/>';
      if ($scope) {
        $formattedhtml .= $scope;
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</p>';

      $formattedhtml .= '<table><tr><th>Broader Terms</th><th>Narrower Terms</th><th>Related Terms</th></tr>';
      $formattedhtml .= '<tr valign="top">';
      $formattedhtml .= '<td width="33%">';
      if ($broaderterms) {
        $formattedhtml .= _get_html_string_of_object_urls($broaderterms);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</td>';

      $formattedhtml .= '<td width="33%">';
      if ($narrowerterms) {
        $formattedhtml .= _get_html_string_of_object_urls($narrowerterms);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</td>';

      $formattedhtml .= '<td width="33%">';
      if ($relatedterms) {
        $formattedhtml .= _get_html_string_of_object_urls($relatedterms);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</td>';
      $formattedhtml .= '</tr>';
      $formattedhtml .= '</table><br/>';

      //GF and IGF
      $formattedhtml .= '<table><tr><th>Geographical Feature</th><th>Is Geographical Feature of</th></tr>';
      $formattedhtml .= '<tr valign="top">';
      $formattedhtml .= '<td width="50%">';
      if ($geographicfeature) {
        $formattedhtml .= _get_html_string_of_object_urls($geographicfeature);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</td>';

      $formattedhtml .= '<td width="50%">';
      if ($isgeofeatureof) {
        $formattedhtml .= _get_html_string_of_object_urls($isgeofeatureof);
      }
      else {
        $formattedhtml .= 'N/A';
      }
      $formattedhtml .= '</td>';

      $formattedhtml .= '</td>';
      $formattedhtml .= '</tr>';
      $formattedhtml .= '</table>';
      // end of GF and IGF

      $formattedhtml .= '<p><strong>This term is used for the following terms:</strong>&nbsp;<br/>';
      if ($usedforterms) {
        $formattedhtml .= _get_html_string_of_object_urls($usedforterms);
      }
      else {
        $formattedhtml .= 'N/A';
      }

      // Search for reciprocal links in vocab collection
      // and also for links to PDF objects.
      module_load_include('inc', 'islandora_content_model_forms', 'ObjectLinker');

      $linkVocabHeader = 'Related subject terms';
      $linkVocabSparqlQuery = 'SELECT ?title ?object WHERE { ?object <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> <info:fedora/fba:vocabpref_CModel>; <fedora-rels-ext:isMemberOfCollection> <info:fedora/fba:vocabpref_collection>; <http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasCorrelative> <info:fedora/' . $this->pid . '> } ORDER BY ?title LIMIT 30';

      $linkVocabArray = get_link_array($linkVocabSparqlQuery);

      if ($linkVocabArray) {
        $formattedhtml .= '<table><tr><th>' . $linkVocabHeader . '</th></tr>';
        foreach ($linkVocabArray as $linkVocabterm) {
          if ($linkVocabterm['title']) {
            $linkVocabtermTitle = $linkVocabterm['title'];
          }
          else {
            $linkVocabtermTitle = 'N/A';
          }

          $formattedhtml .= '<tr valign="top">';
          $formattedhtml .= '<td width="100%">';
          $formattedhtml .= '<a href="' . $base_url . '/fedora/repository/' . $linkVocabterm['object'] . '">' . $linkVocabtermTitle . '</a>';
          $formattedhtml .= '</td>';
          $formattedhtml .= '</tr>';
        }
        $formattedhtml .= '</table>';

        $formattedhtml .= '</p></div>';
      }

      $formattedhtml .= '</p>';
    }
    else {
      // The custom section for a non-preferred term
      //DTC117 SF 20131105 Remove this line completely from display
      //$output[] = array('<h1>' . $variant . '</h1>');
      //DTC117
      // Use Instead
      $formattedhtml .= '<p><strong><span style="color: red;">Use this term instead:</span></strong>&nbsp;' . _get_html_string_of_object_urls($useinsteadterms) . '</p>';
    }
    //Common footer section for both types of terms
    $formattedhtml .= '<p><strong>Approved Date</strong><br/>' . $dateapproved . '</p>';

    if ($authority) {
      /* If a drupal multisite is pointing at one fedora repository,       */
      /* and control over which items are displayed on specific websites   */
      /* is required, include the base URL of all the websites that should */
      /* display links in the following switch-clause, and remove the      */
      /* processing in the 'default:' branch.                              */

      switch ($base_url) {
        case "http://toplevelsite.org":
        case "http://specificsite.org":
          /* To avoid Sparql queries becoming overly complex, when a new     */
          /* website has to be catered for, just add another header/query    */
          /* pair to the array for the particular website.                   */
          /* Replace http://toplevelsite.org with the base URL of the site   */
          /* that is to display all the links and repeat the entire if-clause*/
          /* below for each specific site that should display links for      */
          /* a particular collection. Also set the relevant collection ids,  */
          /* content model ids and predicates.                               */
          /* So, for each website repeat the entire if-clause STARTING HERE: */

          if (!strcmp($base_url, "http://specificsite.org") || !strcmp($base_url, "http://toplevelsite.org") ) {

            if (!strcmp($base_url, "http://toplevelsite.org")) {
              $linkHeader = 'Specific Site items having a keyword relationship to this geographic term';
            }
            elseif (!strcmp($base_url, "http://specificsite.org")) {
              $linkHeader = 'Items having a keyword relationship to this geographic term';
            }
            $linkQuery = 'SELECT ?title ?object WHERE { ?object <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> ?model; <fedora-rels-ext:isMemberOfCollection> ?collection; <http://specific_predicate> <info:fedora/' . $this->pid . '> FILTER (regex(str(?collection), "^info:fedora/specific_namespace:collection$", "i") && (regex(str(?model), "^info:fedora/specific_namespace:specific_content_model1$", "i") || regex(str(?model), "^info:fedora/specific_namespace:specific_content_model2$", "i") || regex(str(?model), "^info:fedora:specifici_namespace:specific_content_model3$", "i")))} ORDER BY $title LIMIT 300';

            $linkSparqlQueries[] = array("header" => $linkHeader, "query" => $linkQuery);
          }

          /* ... AND FINISHING HERE  */
          break;
        default:
          /* This is the default processing that works for the collection id  */
          /* and content model id associated with the standard Islandora      */
          /* PDF Solution Pack. Remove the processing in this branch and      */
          /* follow the instructions above if non-standard processing is      */
          /* required.                                                        */

          $linkHeader = 'PDFs having a keyword relationship to this geographic term';
          $linkQuery = 'SELECT ?title ?object WHERE { ?object <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> ?model; <fedora-rels-ext:isMemberOfCollection> ?collection; <http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasGeographicKeyword> <info:fedora/' . $this->pid . '> FILTER (regex(str(?collection), "^info:fedora/islandora:sp_pdf_collection$", "i") && regex(str(?model), "^info:fedora/islandora:sp_strict_pdf$", "i"))} ORDER BY $title LIMIT 300';
          $linkSparqlQueries[] = array("header" => $linkHeader, "query" => $linkQuery);
      }

      foreach ($linkSparqlQueries as $linkSparqlQuery) { 
        $linkFormattedHtml = get_link_list($linkSparqlQuery['header'], $linkSparqlQuery['query']);

        if (strlen($linkFormattedHtml) > 0) {
          $customLinkFormattedHtml = '<div style="clear:both">' . $linkFormattedHtml . '</div>';
          $formattedhtml .= $customLinkFormattedHtml;
        }
      }
    }

    $output[] = array($formattedhtml);
    $dataStreamBody = theme('table', $headers, $output);

    $collection_fieldset = array(
      '#collapsible' => FALSE,
      '#value' => $dataStreamBody,
    );
    $tabset = array();
    $tabset['geotermdetail'] = array(
      '#type' => 'tabpage',
      '#title' => 'Geographical Term Details',
      '#content' => theme('fieldset', $collection_fieldset),
    );

    return $tabset;
  }
}

