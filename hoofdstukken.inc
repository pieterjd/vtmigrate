<?php

/**
 * @file
 * A basic migration example.
 */

/**
 * To define a migration process from a set of source data to a particular
 * kind of Drupal object (for example, a specific node type), you define
 * a class derived from Migration. You must define a constructor to initialize
 * your migration object. By default, your class name will be the "machine name"
 * of the migration, by which you refer to it. Note that the machine name is
 * case-sensitive.
 *
 * In any serious migration project, you will find there are some options
 * which are common to the individual migrations you're implementing. You can
 * define an abstract intermediate class derived from Migration, then derive your
 * individual migrations from that, to share settings, utility functions, etc.
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
    	array('chapterid' =>array(
    		'type' => 'int',
    		'not null' => TRUE,
    		)	
    	),
    	MigrateDestination::getKeySchema()
    );
    
    //source definition
    $columns = array(
    	0 => array('chapterid','Id of the chapter'),
    	1 => array('course','The course the cpahter belongs to'),
    	2 => array('year','The year of the course'),
    	3 => array('part','Is this a part (container for real chapters) or a chapter?'),
    	4 => array('title','Title of the part or chapter'),
    );
    	
    $this->source = new MigrateSourceCSV(drupal_get_path('module', 'vtmigrate').'/hoofdstukken.csv',
    	$columns,
    	array(
    		'header_row' => 1,
    	)
    );
    
    //destination
    $this->destination = new MigrateDestinationTerm('chapters');
    
    //mapping
    $this->addFieldMapping('name','title');
	}

}