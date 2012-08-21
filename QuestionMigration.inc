<?php

/**
 * @file
 * The personeel migration
 */

class QuestionMigration extends Migration {
	public function __construct() {
    // Always call the parent constructor first for basic setup
    parent::__construct();

    // With migrate_ui enabled, migration pages will indicate people involved in
    // the particular migration, with their role and contact info. We default the
    // list in the shared class; it can be overridden for specific migrations.
    $this->team = array(
      new MigrateTeamMember('Pieter-Jan Drouillon', 'Pieter-Jan.Drouillon@mirw.kuleuven.be', t('Migration Lead')),
    );
    $this->description = t('Migrate questions from the source database to drupal users');
    
    //map table
    $this->map=new MigrateSQLMap($this->machineName,
    	array(
    		'idnr' =>array(
    			'type' => 'int',
    			//'length' => 11,
    			'not null' => TRUE,
    			'description' => 'The id of the question',
    		),	
    	),
    	MigrateDestinationNode::getKeySchema()
    );
    
    //source definition
    $columns = array(
    	0 => array('idnr','Database id'),
    	1 => array('title','Question title'),
    	2 => array('mnr','mnr of student asking the question'),
    	3 => array('email','Email of the stydebte'),
    	4 => array('date','When was the question asked'),
    	5 => array('year','Year of time '),
    	6 => array('questionnr','Nr of the question'),
    	7 => array('responder','Person who responded to the question'),
    	8 => array('course','Course question belongs to'),
    	9 => array('chapter','Chapter questione belongs to'),
    	10 => array('stars','#stars (obsolete)'),
    	11 => array('question','The question itself'),
    	12 => array('answer','Answer to the question'),
    	13 => array('time','Time of asking'),
    	14 => array('busy','Is someone busy answering the question'),
    	15 => array('images','Any images included in the answer?'),
    	16 => array('files','Files included in the answer?'),
    	
    );
    
    //Make sure file is UTF encoded with BOM and has MAC linefeeds
    $file_path = 	DRUPAL_ROOT.'/'.drupal_get_path('module', 'vtmigrate').'/vragen.csv';
    //drupal_set_message(fopen($file_path,'r'));
    //drupal_set_message($file_path);
    $this->source = new MigrateSourceCSV($file_path,
    	$columns,
    	array(
    		'header_rows' => 1,
    		'delimiter' => ';',
    	)
    );
    
    //destination
    $this->destination = new MigrateDestinationNode('question');
    
    //mapping
    //$this->addFieldMapping('uid')
    //			->defaultValue(1);
    //$this->addFieldMapping('promote')->defaultValue(0);
    //$this->addFieldMapping('sticky')->defaultValue(0);
    //$this->addFieldMapping('is_new')->defaultValue(0);
    $this->addFieldMapping('title','title');
    $this->addFieldMapping('body','question')
    	->arguments(
    		array(
    			'format' => 'full_html',
    		)
    	);
    $this->addFieldMapping('field_answer','answer')
    	->arguments(
    		array(
    			'format' => 'full_html',
    		)
    	);
    $this->addFieldMapping('created','date');
    $this->addFieldMapping('field_course_term','course');
    //$this->addFieldMapping('field_responder','responder')
    //			->sourceMigration('User')
    //			->defaultValue(1);
    //$this->addFieldMapping('mail','email');
    //$this->addFieldMapping('role_names','roles')
    //		 ->separator(',');
    
    
    
    //we gaan de source kolom 'files' mappen
    //maar in de prepareRow function gaan wie die overriden met de juiste URI van de files die bij deze vraag horen
    //check drupal.org/node/1069620
    ////arguments for file handling
    /*
    $file_arguments = MigrateFileFieldHandler::arguments($preserve_files = TRUE);
    $this->addFieldMapping('field_attachments')
		     	->separator('#')
    			->arguments($file_arguments);
    */
    //aanpassingen volgens btmash.com/article/2011-03-25/migrating-content-part-2-nodes
    //no source dir, copy the files, and rename it when file already exists
    //$file_arguments = MigrateFileFieldHandler::arguments(NULL, 'file_copy', FILE_EXISTS_RENAME);
    //^^file arguments are obsolete^^
    //do it like on acquia.com blog post
    //add mapping
    $this->addFieldMapping('field_attachments','files')
    	->separator('#');
    $this->addFieldMapping('field_images','images')
    	->separator('#');
    //all imported question are answered
     $this->addFieldMapping('field_state')
     			->defaultValue('answered');
		
				
     //skip these source fields
    $this->addUnmigratedSources(
    	array(
    		'email',
    		//'date',
    		'year',
    		'questionnr',
    		'stars',
    		'time',
    		'busy',
    		//'images',
    		//'files',
    		'course',
    		'chapter',
    		//'answer',
    		'responder',
    		'mnr',
    	)
    );
    //skip these destination fields
    $this->addUnmigratedDestinations(
    	array(
    		'revision_uid',
    		'changed',
    		'revision',
    		'log',
    		'tnid',
    		'comment',
    		//'field_attachments',
    		//'field_course_term',
    		//'created',
    		'status',
    		'language',
    		'path',
    		'pathauto',
    		//'field_answer',
    		'field_responder',
    		'uid',
    		'promote',
    		'sticky',
    		'is_new',
    		//'field_state',
    	)
    );
    
    //add dependency
    $this->dependencies = array('AllChapterInOne');
    
	}
	public function convertDateToTimestamp($d,$t,$row){
		//$d has format 'Sunday 3 June 2012' or 'Sun 23 May 1999' or  'Saturday 8 May'
		//convert this first to format '30-June 2008
		$parts = explode(' ',$d);
		$date = $parts[1].'-'.$parts[2].' '.$row->year;
		$timestamp = strtotime($date);
		//soms is het jaar niet goed bv een vraag van 23 mei 2007 bij de vragentrommel van 2007;. Kan niet want die vraag werd gesteld in 2008
		//dus maanden jan, feb, ... augustus horen bij  vragentrommel 2007, maar werden in het jaar 2008 gesteld
		//get the month of timestamp. If smaller than 9 then add a year
		if(date('n',$timestamp) < 9){
			$timestamp = strtotime('+1 year',$timestamp);
		}
		//now the time
		//remove the u to get 16.24
		$time = str_replace('u','',$t);
		//split on '.'
		$parts = explode('.',$time);
		//add hours and minutes
		//drupal_set_message(print_r($parts,true),'warning');
		$timestamp = strtotime("+ $parts[0] hours +$parts[1] minutes",$timestamp);
		return $timestamp;
	}
	public function prepareRow($row){
 		//setting creationtime to correct timestamp
 		$row->date = $this->convertDateToTimestamp($row->date,$row->time,$row);
 		
 		//handling files
 		//enkel antwoord kan files bevatten. Er wordt verwezen naar files via a tags 
 		//checking for real files (maple files mainly)		
 		$files = array();
 		$nr_matches = preg_match_all('/<a href="(\S+)"/i',$row->answer,$matches,PREG_SET_ORDER);
 		//drupal_set_message("#match: ".print_r($nr_matches,true),'warning');
 		if($nr_matches > 0){
 			drupal_set_message("#filematch: ".print_r($matches,true),'warning');
			
			$base_url = 'https://mirw.kuleuven.be/vragentrommel/trommelroot/';
			//all matches are stored in an array at $matches[0]

			foreach($matches as $match){
 				/*
 				$file_url = $base_url . $match[1];
 				//according btmash.com blogpost
 				$current_file_data = array(
 					'path' => $file_url,
 				);
 				*/
				$files[] = $base_url .$match[1];
 			}
 			
 			//drupal_set_message(print_r($row,true),'error');
		
			
		}
		
		//add the files to $row->images
		$row->files = implode('#',$files);
		//checking for images
		//handling the images
		$images = array();
		$nr_matches = preg_match_all('/<img src=([^\s\>]+)\>/i',$row->answer,$matches,PREG_SET_ORDER);
 		//drupal_set_message("#match: ".print_r($nr_matches,true),'warning');
 		if($nr_matches > 0){
 			drupal_set_message("#image match: ".print_r($matches,true),'warning');
			$base_url = 'https://mirw.kuleuven.be/vragentrommel/trommelroot/';
			//all matches are stored in an array at $matches[0]

			foreach($matches as $match){
 				
 				$file_url = $base_url . $match[1];
 				/*
 				//according btmash.com blogpost
 				$current_file_data = array(
 					'path' => $file_url,
 				);
 				*/
				$images[] = $file_url;
				//drupal_set_message($file_url,'error');
 			}
 			
 			
		
			
		}
		//add the files to $row->images
		$row->images = implode('#',$images);
		drupal_set_message(print_r($files,true),'error');
		drupal_set_message(print_r($images,true),'error');
		if(count($files)>1){
			drupal_set_message('More than 1 file!!!','error');
		}
		if(count($images)>1){
			drupal_set_message('More than 1 image!!!','error');
		}
		return TRUE;
 		
  }
  

	//it is executing before saving migrated object to the destination
  public function prepare($question,stdClass $row){
  	//setting question and answer filterformat to filtered_html
  	//changed: is set in constructor
  	//$question->body['und'][0]['value_format'] = 'filtered_html';
  	//$question->body['und'][0]['format'] = 'filtered_html';
  	
  	//$question->field_answer['und'][0]['value_format'] = 'filtered_html';
  	//$question->field_answer['und'][0]['format'] = 'filtered_html';
  	
  	
  	//setting the field_course_term with the right term
  	//drupal_set_message(print_r($row,true),'warning');
  	$parent = vtmigrate_get_term(array($row->course,"$row->year"));
		//drupal_set_message(print_r($parent,true),'warning');
		$deepest_term = vtmigrate_get_chapter_term($row->course,"$row->year",$row->chapter,$parent->tid);
		//drupal_set_message(print_r($deepest_term,true),'warning');
		$tree = taxonomy_get_parents_all($deepest_term->tid);
		//empty the cours terms
		$question->field_course_term['und'] = array();
		//add all terms of the tree
		foreach($tree as $index => $term){
	
			$question->field_course_term['und'][] = array('tid' => $term->tid);
			
		}
		//add the uid of the user asking the question
		
		//check username is of format u\d{7}
		if(preg_match('/[mrsu]\d{7}/',$row->mnr) == 1){
			//valid username, lookup user
			$user = user_load_by_name($row->mnr);
			if($user == FALSE){
				//unsuccessfully loaded user, lets create it in there
				$new_user = array(
      		'name' => $row->mnr,
		      'pass' => 'aaa',
    		  'mail' => $row->email,
				);
				$user = user_save(null,$new_user);
			}
			$question->uid = $user->uid;
		}
		//add the uid of the responder
		$question->field_responder['und'] = array();
		//check username is of format u\d{7}
		if(preg_match('/[mrsu]\d{7}/',$row->responder) == 1){
			//valid username, lookup user
			$user = user_load_by_name($row->responder);
			if($user != FALSE){
				//successfully loaded user, lets put it in there
				$question->field_responder['und'][] = array(
					'uid' => $user->uid,
				);
			}
		}
		//drupal_set_message(print_r($question,true),'warning');
  	
  }

}