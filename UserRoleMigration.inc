<?php

/**
 * @file
 * The personeel migration
 */

class UserRoleMigration extends Migration {
	public function __construct() {
    // Always call the parent constructor first for basic setup
    parent::__construct();

    // With migrate_ui enabled, migration pages will indicate people involved in
    // the particular migration, with their role and contact info. We default the
    // list in the shared class; it can be overridden for specific migrations.
    $this->team = array(
      new MigrateTeamMember('Pieter-Jan Drouillon', 'Pieter-Jan.Drouillon@mirw.kuleuven.be', t('Migration Lead')),
    );
    $this->description = t('Migrate users from the source database to drupal users');
    
    //map table
    $this->map=new MigrateSQLMap($this->machineName,
    	array(
    		'role' =>array(
    			'type' => 'varchar',
    			'length' => 14,
    			'not null' => TRUE,
    			'description' => 'The KU Leuven unumber',
    		),	
    	),
    	MigrateDestinationRole::getKeySchema()
    );
    
    //source definition
    $columns = array(
    	0 => array('role','The course the user is involved'),
    	
    );
    
    //Make sure file is UTF encoded with BOM and has MAC linefeeds
    $file_path = 	DRUPAL_ROOT.'/'.drupal_get_path('module', 'vtmigrate').'/roles.csv';
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
    $this->destination = new MigrateDestinationRole();
    
    //mapping
    $this->addFieldMapping('name','role');
    
    //skip these destination fields
    $this->addUnmigratedDestinations(
    	array(
    		'weight',
    	)
    );
    
    
    
	}

	

}