<?php

/**
 * @file
 * The personeel migration
 */

class UserMigration extends Migration {
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
    		'unr' =>array(
    			'type' => 'varchar',
    			'length' => 8,
    			'not null' => TRUE,
    			'description' => 'The KU Leuven unumber',
    		),	
    	),
    	MigrateDestinationUser::getKeySchema()
    );
    
    //source definition
    $columns = array(
    	0 => array('name','Name of the user'),
    	1 => array('unr','Unumber of the user'),
    	2 => array('email','Email of the user'),
    	3 => array('course','The course the user is involved'),
    	4 => array('monitor','Is this user a monitor?'),
    	5 => array('id','Database id'),
    	6 => array('roles','The courses involved (will be mapped to roles)'),
    	
    );
    
    //Make sure file is UTF encoded with BOM and has MAC linefeeds
    $file_path = 	DRUPAL_ROOT.'/'.drupal_get_path('module', 'vtmigrate').'/personeel.csv';
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
    $this->destination = new MigrateDestinationUser();
    
    //mapping
    $this->addFieldMapping('name','unr');
    $this->addFieldMapping('mail','email');
    //$this->addFieldMapping('role_names','roles')
    //		 ->separator(',');
    
     //skip these source fields
    $this->addUnmigratedSources(
    	array(
    		'name',
    		'course',
    		'monitor',
    		'id',
    		'roles',
    	)
    );
    //skip these destination fields
    $this->addUnmigratedDestinations(
    	array(
    		'is_new',
    		'pass',
    		'roles',
    		'theme',
    		'signature',
    		'signature_format',
    		'created',
    		'access',
    		'login',
    		'status',
    		'timezone',
    		'language',
    		'picture',
    		'init',
    		'path',
    		'pathauto',
    	)
    );
    
    //add dependency
    //$this->dependencies = array('UserRole');
    
	}

	//it is executing before saving migrated object to the destination
  public function prepare($user,stdClass $row){
  	//add all the role ids to the user account
  	$role_names = explode(',',$row->roles);
  	
  	//check if role didactisch team bestaat
  	$new_role = array('didactisch team');
  	foreach($new_role as $r){
  		if(!user_role_load_by_name($r)){
  			//create it
  			$role = new stdClass();
  			$role->name = $r;
  			user_role_save($role);
  		}
  	}
  	$role = user_role_load_by_name('didactisch team');
 		//$user role is a mapping from role id to role name
 		$user->roles[$role->rid] = $role->name;
  	//drupal_set_message(print_r($user,true),'warning');
  	//set the roles
  	//$user->roles = $role_ids;
  }

}