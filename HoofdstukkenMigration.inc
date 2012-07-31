<?php

/**
 * @file
 * The hoofdstukken migration
 */

/*
Abstract class setting up source and destination. It's left to subclasses for the mapping.

Each row contains the fields course, year , title

In the vocabulary, the hierarchy should be:
course
	|_ year
			|_ part
					|_chapter title
			
*/
abstract class HoofdstukkenMigration extends Migration {
	public function __construct() {
    // Always call the parent constructor first for basic setup
    parent::__construct();

    // With migrate_ui enabled, migration pages will indicate people involved in
    // the particular migration, with their role and contact info. We default the
    // list in the shared class; it can be overridden for specific migrations.
    $this->team = array(
      new MigrateTeamMember('Pieter-Jan Drouillon', 'Pieter-Jan.Drouillon@mirw.kuleuven.be', t('Migration Lead')),
    );
    $this->description = t('Migrate chapters from the source database to taxonomy terms');
    
    //map table
    $this->map=new MigrateSQLMap($this->machineName,
    	array(
    		'chapterid' =>array(
    			'type' => 'int',
    			'not null' => TRUE,
    			'description' => 'The chapter ID',
    		),
    		'course' => array(
    			'type' => 'varchar',
    			'length' => 10,
    			'not null' => TRUE,
    			'description' => 'The course ID',
    		),
    		'year' => array(
    			'type' => 'int',
    			'not null' => TRUE,
    			'description' => 'The year of the course',
    		),	
    	),
    	MigrateDestinationTerm::getKeySchema()
    );
    
    //source definition
    $columns = array(
    	0 => array('chapterid','Id of the chapter'),
    	1 => array('course','The course the chapter belongs to'),
    	2 => array('year','The year of the course'),
    	3 => array('part','Is this a part (container for real chapters) or a chapter?'),
    	4 => array('title','Title of the part or chapter'),
    	5 => array('parttitle','Title of the part the chapter belongs to'),
    );
    
    //Make sure file is UTF encoded with BOM and has MAC linefeeds
    $file_path = 	DRUPAL_ROOT.'/'.drupal_get_path('module', 'vtmigrate').'/hoofdstukken-v6.csv';
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
    $this->destination = new MigrateDestinationTerm('chapters');
    
    
    
	}
	/**
	* helper function
	eg. vtmigrate_get_term(array('a','b'),2,7)
	will return the term id of the term b if the following situation holds:
	term with id 7 from voc 2
	|_ a
	   |_ b
	*
	*/
	public function get_term($term_array,$parent_id=0){
	$result = null;
	if(count($term_array) == 1){
		//trivial case: only 1 name in array
		$current_terms = array_values(taxonomy_get_term_by_name($term_array[0]));
		if($current_terms != null){
			$current_term = $current_terms[0];
			$parent_term = taxonomy_term_load($parent_id);
			$parents = taxonomy_get_parents($current_term->tid);
			
			foreach($parents as $p){
				if($p->tid == $parent_term->tid){
					//everything is ok
					//change result
					$result = $current_term;
	
				}
			}
		}
	}
	else{
		//recursive case
		//first element of $term_array will be the parent parameter of the recursive call
		$terms = array_values(taxonomy_get_term_by_name($term_array[0]));
		$first_term = $terms[0];
		//remove first element of $term_array
		unset($term_array[0]);
		//recursive call
		//call to array_values is required, so element at position 1 moves to position 0
		$result = vtmigrate_get_term(array_values($term_array),$first_term->tid);
		
		
	}
	return $result;
}
	

}





/**
Class to migrate all chapters in one
*/
class  AllChapterInOneMigration extends HoofdstukkenMigration{
	public function __construct() {
    // Always call the parent constructor first for basic setup
    parent::__construct();
    //dsm('In CourseMigration subclass');
    //dsm($this->source);
    //dsm($this->destination);
    //dsm($this->map);
    $this->description = t('Migrate the course field from the source database to taxonomy terms');
    //mapping
    $this->addFieldMapping('name','title');
    $this->addFieldMapping('weight','chapterid');
    
    //skip these source fields
    $this->addUnmigratedSources(
    	array(
    		//'title',
    		'year',
    		'part',
    		//'chapterid',
    		'parttitle',
    		'course',
    	)
    );
    //skip these destination fields
    $this->addUnmigratedDestinations(
    	array(
    		'description',
    		'format',
    		//'weight',
    		'parent_name',
    		'parent',
    		'path',
    		'pathauto',
    	)
    );
   
  }
  //should this row be processed or not
	public function prepareRow($row){
  	//only process the chapter entries!
  	return $row->part == 0;
  }
  //it is executing before saving migrated object to the destination
  public function prepare($term,stdClass $row){
  	//retrieve the vocabulary
  	$voc = taxonomy_vocabulary_machine_name_load('chapters');
  	//create all parents
		//term contains the term representing the chapter title
		
		//course term
		$course_term = taxonomy_get_term_by_name($row->course);
		if(empty($course_term)){
			//course doesn't exist yet, let's create it
			$course_term = (object) array(
					'vid' => $voc->vid,
					'name' => $row->course,
			);
			
			taxonomy_term_save($course_term);
		}
		else{
			//course term exits, let's get it out the array
			$values = array_values($course_term);
			$course_term = $values[0];
		}
		
		
		//year term
		$year_term = vtmigrate_get_term(array($row->year),$course_term->tid);
		if(is_null($year_term)){
			//create year term
			$year_term = (object) array(
					'vid' => $voc->vid,
					'name' => $row->year,
					'parent' => $course_term->tid,
				);
				taxonomy_term_save($year_term);
		}
		//checken op of er een part is
		if(strlen($row->parttitle)>3){
			//er is een part, checken of het al gemaakt is
			$part_term = vtmigrate_get_term(array($row->parttitle),$year_term->tid);
			if(is_null($part_term) and strcmp($row->parttitle,'') != 0 and strlen($row->parttitle)>4){
				//create part term if not present yet and not equal to empty string and long enough
				$part_term = (object) array(
						'vid' => $voc->vid,
						'name' => $row->parttitle,
						'parent' => $year_term->tid,
					);
				
				taxonomy_term_save($part_term);
				//drupal_set_message('partterm created: '.print_r($part_term,true),'warning');
			}
			//en de part als parent zetten
			$term->parent = $part_term->tid;
		}
		else{
			//er is geen part dus jaar is de parent
			$term->parent = $year_term->tid;
		}
		
		//term contains the term representing the chapter title
		
		
		
  
  }
  
	
}