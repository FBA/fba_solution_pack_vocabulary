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
    $predicate = '';
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

      //We don't do anything with recordCreationDate/recordChangeDate at the moment
      //but here's the code just in case
      //      $recordInfo = $MADSDom->getElementsByTagName('recordInfo')->item(0);
      //      if ($recordInfo->hasChildNodes()) {
      //        foreach ($recordInfo->childNodes as $p) {
      //          $nname = $p->nodeName;
      //nname might have "mads:" prefix on it. If so, remove.
      //          $pos = strpos($nname, 'mads:');
      //          if ($pos === false) {
      //thats ok, mads: wasn't a prefix
      //          }
      //          else {
      //            $nname = substr($nname, 5); //5 is length of 'mads:'
      //          }
      //          switch ($nname) {
      //            case 'recordCreationDate': {
      //                break;
      //              }
      //            case 'recordChangeDate': {
      //                break;
      //              }
      //            default: {
      //                break;
      //              }
      //          }
      //        }
      //      }
      //      else {
      //placeholder
      //      }
      // End of paragraph
    }

    $output = array(); //this will be the formatted html to be sent to the screen
    //each array entry ends up as new <tr>

    if ($authority) {
      // The custom section for a preferred term
      //DTC117 SF 20131105 Remove this line completely from display
      //$output[] = array('<h1>' . $authority . '</h1>');
      //DTC117
      $formattedhtml = '<p><h1>' . $authority . '</h1></p>';
      //Definition
      $formattedhtml .= '<p><strong>Definition</strong>';
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

      $linkGeoHeader = 'Correlative geographic terms';
      $linkGeoSparqlQuery = 'SELECT ?title ?pid WHERE { ?pid <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> <info:fedora/fba:geopref_CModel>; <fedora-rels-ext:isMemberOfCollection> <info:fedora/fba:geopref_collection>; <http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasCorrelative> <info:fedora/' . $this->pid . '> } ORDER BY ?title';
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
          $formattedhtml .= '<a href="' . $base_url . '/archive/' . $linkGeoterm['pid'] . '">' . $linkGeotermTitle . '</a>';
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
      $formattedhtml .= '<p><h1>' . $variant . '</h1></p>';
      // Use Instead
      $formattedhtml .= '<p><strong><span style="color: red;">Use this term instead:</span></strong>&nbsp;' . _get_html_string_of_object_urls($useinsteadterms) . '</p>';
    }
    //Common footer section for both types of terms
    $formattedhtml .= '<p><strong>Approved Date</strong><br/>' . $dateapproved . '</p>';

    if ($authority) {
      $predicate = 'http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasKeyword';
      $header = 'Items having a keyword or observed-property relationship to this subject term';
      $this->createListOfLinks($predicate, $header, $formattedhtml);
    }

    $formattedhtml .= '<p><a href="mailto:vocabulary@fba.org.uk?Subject=DTC%20Vocab%20enquiry%20-%20';
    if ($authority) {
      $formattedhtml .= rawurlencode($authority);
    }
    else {
      $formattedhtml .= rawurlencode($variant);
    }

    $formattedhtml .= ' ">Click here to give feedback on this term or suggest an update.</a></p>';
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
    $predicate = '';

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

      //We don't do anything with recordCreationDate/recordChangeDate at the moment
      //but here's the code just in case
      //      $recordInfo = $MADSDom->getElementsByTagName('recordInfo')->item(0);
      //      if ($recordInfo->hasChildNodes()) {
      //        foreach ($recordInfo->childNodes as $p) {
      //          $nname = $p->nodeName;
      //nname might have "mads:" prefix on it. If so, remove.
      //          $pos = strpos($nname, 'mads:');
      //          if ($pos === false) {
      //thats ok, mads: wasn't a prefix
      //          }
      //          else {
      //            $nname = substr($nname, 5); //5 is length of 'mads:'
      //          }
      //          switch ($nname) {
      //            case 'recordCreationDate': {
      //                break;
      //              }
      //            case 'recordChangeDate': {
      //                break;
      //              }
      //            default: {
      //                break;
      //              }
      //          }
      //        }
      //      }
      //      else {
      //placeholder
      //      }
      // End of paragraph
    }

    $output = array(); //this will be the formatted html to be sent to the screen
    //each array entry ends up as new <tr>

    if ($authority) {
      // The custom section for a preferred term
      //DTC117 SF 20131105 Remove this line completely from display
      //$output[] = array('<h1>' . $authority . '</h1>');
      //DTC117
      $formattedhtml = '<p><h1>' . $authority . '</h1></p>';
      //Definition
      $formattedhtml .= '<p><strong>Definition</strong>';
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

      $linkVocabHeader = 'Correlative subject terms';
      $linkVocabSparqlQuery = 'SELECT ?title ?pid WHERE { ?pid <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> <info:fedora/fba:vocabpref_CModel>; <fedora-rels-ext:isMemberOfCollection> <info:fedora/fba:vocabpref_collection>; <http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasCorrelative> <info:fedora/' . $this->pid . '> } ORDER BY ?title';

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
          $formattedhtml .= '<a href="' . $base_url . '/archive/' . $linkVocabterm['pid'] . '">' . $linkVocabtermTitle . '</a>';
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
      $formattedhtml .= '<p><h1>' . $variant . '</h1></p>';
      // Use Instead
      $formattedhtml .= '<p><strong><span style="color: red;">Use this term instead:</span></strong>&nbsp;' . _get_html_string_of_object_urls($useinsteadterms) . '</p>';
    }
    //Common footer section for both types of terms
    $formattedhtml .= '<p><strong>Approved Date</strong><br/>' . $dateapproved . '</p>';

    if ($authority) {
      $predicate = 'http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasGeographicKeyword';
      $header = 'Items having a keyword relationship to this geographic term';
      $this->createListOfLinks($predicate, $header, $formattedhtml);
    }

    $formattedhtml .= '<p><a href="mailto:vocabulary@fba.org.uk?Subject=DTC%20Vocab%20enquiry%20-%20';
    if ($authority) {
      $formattedhtml .= rawurlencode($authority);
    }
    else {
      $formattedhtml .= rawurlencode($variant);
    }
    $formattedhtml .= ' ">Click here to give feedback on this term or suggest an update.</a></p>';
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

  /**
   * Create a list of links to fedora objects that have an RDF that has this
   * Fedora object as an 'object' component of a 'subject-predicate-object'
   * triplet and if such links exist, append them as a list to the
   * $formattedhtml variable. The list created is appropriate to the 
   * website.
   * @return none
   */
  function createListOfLinks(&$predicate, &$header, &$formattedhtml) {
      module_load_include('inc', 'fba_utility', 'GenericFunctions');
      global $base_url;
      $linkArray = array();
      /* Get the base urls of the websites for which website-specific      */
      /* branching is required. Include them in the switch statement       */
      /* below if links to items should be displayed via the websites.     */
      /* This will prevent unnecessary Sparql queries being executed.      */
      $data_local = base_url_equiv('data_local');
      $ea_greylit = base_url_equiv('ea_greylit');
      $env_data = base_url_equiv('env_data');
      $a_salmon_t = base_url_equiv('a_salmon_t');
      switch ($base_url) {
        case $data_local:
        case $ea_greylit:
        case $env_data:
        case $a_salmon_t:
          /* A balance has to be achieved between the performance of the     */
          /* SPARQL queries generated by this function and their complexity. */
          /* When a new website or collection has to be catered for decide   */
          /* whether modifying one or more of the existing queries or        */
          /* creating a new query whose results can be merged into the array */
          /* is more appropriate. data_local should be able to view links    */
          /* to all the collections.                                         */
          /* As a general principle, the env_data website contains everything*/
          /* that is publishable. The FBA's data archive site (URL as yet    */
          /* unknown) will display only the data that is published by the    */
          /* FBA. The data_local website is currently serving as the website */
          /* ALL data can be viewed i.e. including objects and datastreams   */
          /* that have to be private due to reasons of copyright or whatever.*/

          if (!strcmp($base_url, $ea_greylit) || !strcmp($base_url, $data_local) || !strcmp($base_url, $env_data)) {
            $linkQuery = 'SELECT ?title ?pid WHERE { ?pid <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> ?model; <fedora-rels-ext:isMemberOfCollection> ?collection; <' . $predicate . '> <info:fedora/' . $this->pid . '> FILTER (regex(str(?collection), "^info:fedora/ealit:collection$", "i") && (regex(str(?model), "^info:fedora/fba:library_monograph_cmodel$", "i") || regex(str(?model), "^info:fedora/fba:library_article_cmodel$", "i") || regex(str(?model), "^info:fedora/fba:library_chapter_cmodel$", "i")))}';
            $linkArray = get_link_array($linkQuery);
          }

          if (!strcmp($base_url, $a_salmon_t) || !strcmp($base_url, $data_local) || !strcmp($base_url, $env_data)) {
            $linkQuery = 'SELECT ?title ?pid WHERE { ?pid <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> <info:fedora/fba:library_monograph_cmodel>; <fedora-rels-ext:isMemberOfCollection> ?collection; <' . $predicate . '> <info:fedora/' . $this->pid . '> FILTER (regex(str(?collection), "^info:fedora/xastj:collection$", "i") || regex(str(?collection), "^info:fedora/xastbb:collection$", "i") || regex(str(?collection), "^info:fedora/xastsacr:collection$", "i"))}';
            $linkAstArray = get_link_array($linkQuery);
            if (count($linkAstArray)) {
              if (count($linkArray)) {
                $linkArray = array_merge($linkArray, $linkAstArray);
              } else {
                $linkArray = $linkAstArray;
              }
            }
          }

          if (!strcmp($base_url, $data_local) || !strcmp($base_url, $env_data)) {
            /*
             * A rather ugly work-around is required here due to the existence
             * of the hasObservedProperty RDF which points to objects in the
             * same collection as the hasKeyword RDF and basically should
             * be handled in the same way. Therefore the SPARQL query generated
             * for the hasKeyword RDF should include the hasObservedProperty RDF
             * where as the SPARQL query generated for the hasGeographicKeyword
             * should not include it. Therefore the processing branches at this
             * point to allow for this. It is possible that one object might
             * have both hasKeyword and hasObservedProperty RDFs pointing to
             * the same keyword object, therefore DISTINCT is specified in the
             * relevant select to ensure that duplicate entries do not appear 
             * in the list.
             */

            if (!strcmp($predicate, 'http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasKeyword')) {
              $linkQuery = 'SELECT DISTINCT ?title ?pid WHERE { ?pid <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-rels-ext:isMemberOfCollection> ?collection; ?predicate <info:fedora/' . $this->pid . '> FILTER ((regex(str(?collection), "^info:fedora/dtcwensum:collection$", "i") || regex(str(?collection), "^info:fedora/dtceden:collection$", "i") || regex(str(?collection), "^info:fedora/dtcavon:collection$", "i") || regex(str(?collection), "^info:fedora/ghgno:collection$", "i") || regex(str(?collection), "^info:fedora/ghgch:collection$", "i") || regex(str(?collection), "^info:fedora/fbads:collection$", "i")) && (regex(str(?predicate), "^http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasKeyword$", "i") || regex(str(?predicate), "^http://fba.org.uk/schemas/fba/fba_rdf/1.0.0/hasObservedProperty$", "i")))}';
            } else {
              $linkQuery = 'SELECT ?title ?pid WHERE { ?pid <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-rels-ext:isMemberOfCollection> ?collection; <' . $predicate . '> <info:fedora/' . $this->pid . '> FILTER (regex(str(?collection), "^info:fedora/dtcwensum:collection$", "i") || regex(str(?collection), "^info:fedora/dtceden:collection$", "i") || regex(str(?collection), "^info:fedora/dtcavon:collection$", "i") || regex(str(?collection), "^info:fedora/ghgno:collection$", "i") || regex(str(?collection), "^info:fedora/ghgch:collection$", "i") || regex(str(?collection), "^info:fedora/fbads:collection$", "i"))}';
            }
            $linkAedaArray =  get_link_array($linkQuery);
            if (count($linkAedaArray)) {
              if (count($linkArray)) {
                $linkArray = array_merge($linkArray, $linkAedaArray);
              } else {
                $linkArray = $linkAedaArray;
              }
            }

            $linkQuery = 'SELECT ?title ?pid WHERE { ?pid <http://purl.org/dc/elements/1.1/title> ?title; <fedora-model:state> <fedora-model:Active>; <fedora-model:hasModel> ?model; <fedora-rels-ext:isMemberOfCollection> ?collection; <' . $predicate . '> <info:fedora/' . $this->pid . '> FILTER ((regex(str(?collection), "^info:fedora/fbaiasub:1695$", "i") || regex(str(?collection), "^info:fedora/fbaia:83$", "i") || regex(str(?collection), "^info:fedora/yyy:collection$", "i")) && (regex(str(?model), "^info:fedora/fba:sp_basic_image$", "i") || regex(str(?model), "^info:fedora/fba:library_monograph_cmodel$", "i")))}';
            $linkFbaiaAndEprintArray = get_link_array($linkQuery);
            if (count($linkFbaiaAndEprintArray)) {
              if (count($linkArray)) {
                $linkArray = array_merge($linkArray, $linkFbaiaAndEprintArray);
              } else {
                $linkArray = $linkFbaiaAndEprintArray;
              }
            }
          }
          break;
      }

      if (count($linkArray) > 0) {
       // Perform a case insensitive sort on the combined array
        // and add the content wrapped in list-html to the rest
        // of the html.
        uasort($linkArray, function($a, $b){
          return strcasecmp($a['title'],$b['title']);
        });
        $linkHeader = '<b>' . $header . '</b>';
        $linkFormattedHtml = '<div style="clear:both"><p>' . $linkHeader;
        $linkFormattedHtml .= '<ul style="list-style:none">';
        foreach ($linkArray as $linkArrayElement) {
          $linkFormattedHtml .= '<li>' . '<a href="' . $base_url . '/archive/' . $linkArrayElement['pid'] . '">' . $linkArrayElement['title'] . '</a></li>';
        }
        $linkFormattedHtml .= '</ul></p></div>';
        $formattedhtml .= $linkFormattedHtml;
      }
  }
}
