#include "LevelManager.h"

#include <CCDirector.h>

#include "Enemy.h"
#include "ExitDoor.h"
#include "PlatformCrumbling.h"
#include "PickupsManager.h"
#include "Settings.h"
#include "TextureManager.h"
#include "Travellator.h"

#include <cocos/2d/CCFastTMXLayer.h>

using cocos2d::FastTMXTiledMap;
using cocos2d::TMXObjectGroup;
using cocos2d::TMXMapInfo;
using cocos2d::PhysicsBody;
using cocos2d::Vec2;
using cocos2d::Size;
using cocos2d::PhysicsShapeBox;
using cocos2d::Sprite;
using cocos2d::ValueMap;
using cocos2d::ValueVector;

CLevelManager::CLevelManager()
	: m_pcCurrentLevel( nullptr )
	, m_pcColliderContainer( nullptr )
	, m_pcTextureManager( nullptr )
	, m_pcPickupsManager( nullptr )
	, m_bExitDoorExist( false )
	, m_iCurrentStage( -1 )
	, m_iNumOfTravellatorInStage( 0 )
{
	// Convert the current stage ID to a string
	m_sCurrentStage = std::to_string( m_iCurrentStage );
	// Creating platforms' vector
	m_pcPlatforms.resize( 0 );
	// Creating enemies' vector
	m_pcEnemies.resize( 0 );

	// Creating a vector to store all checkpoints in the game
	m_pcCheckpoints.resize( 0 );
	// Create a vector for all ports.
	m_pcPorts.resize( 0 );

	// Create an exit door.
	m_pcExitDoor = new CExitDoor();
}

CLevelManager::~CLevelManager()
{

	// Cycle over all the platforms' vector and safe destroy them
	for( CPlatformBase* pcPlatform : m_pcPlatforms )
	{
		m_pcCurrentLevel->removeChild( pcPlatform );
		CC_SAFE_DELETE( pcPlatform );
	}

	// Cycle over all the enemies' vector and safe destroy them
	for( CEnemy* pcEnemy : m_pcEnemies )
	{
		m_pcCurrentLevel->removeChild( pcEnemy );
		CC_SAFE_DELETE( pcEnemy );
	}

	// Removing pickups from map
	for( auto pickup : m_pcPickupsManager->GetPickups() )
	{
		m_pcCurrentLevel->removeChild( pickup );
	}

	// Cycle over all the ports' vector and safe destroy them
	for( CPort* pcPort : m_pcPorts )
	{
		m_pcCurrentLevel->removeChild( pcPort );
		CC_SAFE_DELETE( pcPort );
	}

	for( CCheckpoint* pcCheckpoint : m_pcCheckpoints )
	{
		m_pcCurrentLevel->removeChild( pcCheckpoint );
		CC_SAFE_DELETE( pcCheckpoint );
	}

	m_pcCurrentLevel->removeChild( m_pcExitDoor );
	CC_SAFE_DELETE( m_pcExitDoor );

}

void CLevelManager::Initialise( CTextureManager* pcTextureManager, CPickupsManager* pcPickupsManager, CHUD* pcHUD )
{
	m_pcHUD = pcHUD;

	CCASSERT( nullptr != pcTextureManager, "Texture Manager is null" );
	// Setting the texture manager in order to pass it to others classes
	m_pcTextureManager = pcTextureManager;

	CCASSERT( nullptr != pcPickupsManager, "Pickups Manager is null" );
	// Setting the pickups manager in order to retrieve the pickups vector
	m_pcPickupsManager = pcPickupsManager;

	LoadAllMaps();

	// Creating all platforms of all types based on the settings values
	TCreateEntities<CPlatformCrumbling>( m_pcPlatforms, Platforms::k_iMaxAmountOfCrumblingPlatforms, *m_pcTextureManager );
	TCreateEntities<CTravellator>( m_pcPlatforms, Platforms::k_iMaxAmountOfTravellatorsPlatforms, *m_pcTextureManager );

	// Creating all ports based on the settings values
	TCreateEntities<CPort>( m_pcPorts, Ports::k_iMaxAmountOfPorts, *m_pcTextureManager );
	// Creating all enemies based on the settings values
	TCreateEntities<CEnemy>( m_pcEnemies, Enemies::k_iMaxAmountOfEnemies, *m_pcTextureManager );

	TCreateEntities<CCheckpoint>( m_pcCheckpoints, 1, *m_pcTextureManager );

	// Create a collider for the map with no shape and all values set to 0
	CreateColliderContainer();

	// Create physics shapes for all map static objects and add them to the map's collider
	CreateCollidableObjects( "Stage Bounds" );
	CreateCollidableObjects( "Walls" );
	CreateCollidableObjects( "Floor" );
	CreateCollidableObjects( "Obstacles" );
	CreateCollidableObjects( "Climbable" );

	// Initialise the exit door
	m_pcExitDoor->Initialise( m_pcTextureManager );

	// Adding all pickups to the map
	for( auto pickup : m_pcPickupsManager->GetPickups() )
	{
		m_pcCurrentLevel->addChild( pickup, 1 );
	}

	// Loading a special stage, this call is used to properly initialise all objects which can be
	// placed in a stage. Their physics collider is set and cannot be reshaped from this point
	LoadNewStage( -1 );

}

void CLevelManager::Update( float fDeltaTime )
{
	// Call the update of the exit door
	m_pcExitDoor->VUpdate( fDeltaTime );
	// Call the update of the pickups manager
	m_pcPickupsManager->VUpdate( fDeltaTime );

	// Call the update of all the travellators in the current stage
	for( int i = 0; i < m_iNumOfTravellatorInStage; i++ )
	{
		CPlatformBase* pcPlatform = m_pcPlatforms[ i + Platforms::k_iMaxAmountOfCrumblingPlatforms ];
		pcPlatform->VUpdate( fDeltaTime );
	}

}

void CLevelManager::LoadAllMaps()
{
	// Create a map from the Tiled map of the first level
	m_pcCurrentLevel = cocos2d::FastTMXTiledMap::create( Levels::k_cLevelOne );

	CCASSERT( nullptr != m_pcCurrentLevel, "No level loaded" );

}

void CLevelManager::CreateColliderContainer()
{

	if( m_pcColliderContainer == nullptr )
	{
		// Create a physics body for the map and add it to the map
		m_pcColliderContainer = PhysicsBody::create();
		m_pcCurrentLevel->addComponent( m_pcColliderContainer );
	}
	else
	{
		// If the map has already a physics body then clean previous added shapes
		// in preparation to the new level initialisation
		m_pcColliderContainer->removeAllShapes();
	}

	// This collider will only contains walls so we don't need dynamic physics properties activated
	m_pcColliderContainer->setMass( 0.0f );
	m_pcColliderContainer->setMoment( 0.0f );
	m_pcColliderContainer->setDynamic( false );

#pragma region Collider Positioning Example
	// Collider position correction

	//-------------------------------------------	By default cocos2d position the collider at the
	//|											|	center of the screen as show by X.
	//|											|	Tilemap is rendered from the bottom left corner
	//|											|	of the screen.
	//|					  X						|	
	//|											|	
	//|											|	
	//|											|
	//-------------------------------------------

	//-------------------------------------------	To fix this problem the collider needs to be
	//|											|	shifted back of half the map's width and height.
	//|											|	
	//|											|	This offset is calculated multiplying map's
	//|					  						|	current dimensions in tiles by the tile's
	//|											|	dimensions and factoring the content scale   
	//|											|	factor derived from the design resolution
	//|											|	The result is shown as the new position of X
	//X------------------------------------------
#pragma endregion

	float fColliderOffsetX = -m_pcCurrentLevel->getMapSize().width * m_pcCurrentLevel->getTileSize().width *
		0.5f * m_pcCurrentLevel->getScaleX();
	float fColliderOffsetY = -m_pcCurrentLevel->getMapSize().height * m_pcCurrentLevel->getTileSize().height *
		0.5f * m_pcCurrentLevel->getScaleY();
	m_pcColliderContainer->setPositionOffset( Vec2( fColliderOffsetX, fColliderOffsetY ) /
		cocos2d::Director::getInstance()->getContentScaleFactor() );

	// Set a name for the map collider which is used in the collision manager
	m_pcColliderContainer->setName( "Environment" );
}

void CLevelManager::CreateCollidableObjects( const std::string& rsObjectGroup )
{
	// Get map rcObjects and make them collidable walls
	ValueVector& rcObjectsVector = m_pcCurrentLevel->getObjectGroup( rsObjectGroup )->getObjects();

	CCASSERT( !rcObjectsVector.empty(), rsObjectGroup.c_str() );

	// Adding shapes to the map collider based on the Tilemap group object and 
	// adjusting position with respect to the map's physics body
	for( cocos2d::Value& rcObject : rcObjectsVector )
	{
		ValueMap& rcObjectValues = rcObject.asValueMap();

		Size cShapeDimensions = Size( rcObjectValues[ "width" ].asFloat(), rcObjectValues[ "height" ].asFloat() );

		// Similarly to the map's collider position correction here another coordinate correction is required
		// but in this case the original point has to be shifted forward instead of back
		float fOffsetCorrectionX = rcObjectValues[ "x" ].asFloat() + rcObjectValues[ "width" ].asFloat() * 0.5f;
		float fOffsetCorrectionY = rcObjectValues[ "y" ].asFloat() + rcObjectValues[ "height" ].asFloat() * 0.5f;

		// Create a collider shape for a single wall and add it to map's collider
		PhysicsShapeBox* pCBox = PhysicsShapeBox::create( cShapeDimensions, cocos2d::PhysicsMaterial( 1.0f, 0.0f, 1.0f ),
			Vec2( fOffsetCorrectionX, fOffsetCorrectionY ) );
		m_pcColliderContainer->addShape( pCBox, false );

		// Set tag to identify obstacles from walls
		if( rsObjectGroup == "Stage Bounds" )	// 0
			pCBox->setTag( Environment::k_iBoundLayer );
		if( rsObjectGroup == "Walls" )			// 1
			pCBox->setTag( Environment::k_iWallLayer );
		if( rsObjectGroup == "Floor" )			// 2
			pCBox->setTag( Environment::k_iFloorLayer );
		if( rsObjectGroup == "Obstacles" )		// 3
			pCBox->setTag( Environment::k_iObstacleLayer );
		if( rsObjectGroup == "Climbable" )		// 4
			pCBox->setTag( Environment::k_iClimbLayer );

		// Set shape to collide and trigger only with the player
		pCBox->setCollisionBitmask( WALL_BITMASK_COLLIDER );
		pCBox->setCategoryBitmask( WALL_BITMASK_CATEGORY );
		pCBox->setContactTestBitmask( WALL_BITMASK_CONTACT );
	}
}

void CLevelManager::PickUpPositioning( const std::string& rsObjectGroup )
{
	// There is no object group with this name which means no object of this kind in this stage
	if( nullptr == m_pcCurrentLevel->getObjectGroup( rsObjectGroup ) )
	{
		return;
	}

	// Get map's current stage pickups as vector of values
	ValueVector& rcObjectsVector = m_pcCurrentLevel->getObjectGroup( rsObjectGroup )->getObjects();

	// Position and reset the pickups of the value's vector
	m_pcPickupsManager->PositionPickups( rcObjectsVector );
	m_pcPickupsManager->ResetPickups( rcObjectsVector );
}

void CLevelManager::ExitPositioning( const std::string& rsObjectGroup )
{
	// Get map's current stage exit door as map of values
	const ValueMap& rcObjectsValues = m_pcCurrentLevel->getObjectGroup( rsObjectGroup )->getObject( "ExitDoor " + m_sCurrentStage );

	CCASSERT( !rcObjectsValues.empty(), "Missing ExitDoor " + *m_sCurrentStage.c_str() );

	// Position is corrected with respect to the map's anchor point which is a the bottom left of the screen
	float fOffsetCorrectionX = rcObjectsValues.at( "x" ).asFloat() + rcObjectsValues.at( "width" ).asFloat() * 0.5f;
	float fOffsetCorrectionY = rcObjectsValues.at( "y" ).asFloat() + rcObjectsValues.at( "height" ).asFloat() * 0.5f;

	// Set position of the pickups based on the map's object positioning
	m_pcExitDoor->setPosition( fOffsetCorrectionX, fOffsetCorrectionY );

	// Add the pickup to the current map if not present already
	if( !m_bExitDoorExist )
	{
		m_pcCurrentLevel->addChild( m_pcExitDoor, 1 );
		m_bExitDoorExist = true;
	}
	// Reset the door to default values
	m_pcExitDoor->ResetDoor();

}

void CLevelManager::EnemiesPositioning( const std::string& rsObjectGroup )
{
	// Get map's current stage enemies as object group
	TMXObjectGroup* rcObjectGroup = m_pcCurrentLevel->getObjectGroup( rsObjectGroup );

	// There is no object group with this name which means no object of this kind in this stage
	if( nullptr == rcObjectGroup )
	{
		return;
	}

	ValueVector& rcObjectsVector = rcObjectGroup->getObjects();

	// IF there are enemies in the current stage this vector should NOT be empy
	CCASSERT( !rcObjectsVector.empty(), *rsObjectGroup.c_str() + " empty" );

	// Do this if loading the "pre-initialisation" stage
	if( -1 == m_iCurrentStage )
	{
		// Initialise all enemies of the enemy vector with the values from the object vector
		for( CEnemy* pcEnemy : m_pcEnemies )
		{
			pcEnemy->Initialise( rcObjectsVector[ 0 ] );
		}
	}
	// Do this for every normal stage
	else
	{
		// Initialise the amount of enemies present in the current stage with the objects vector's values
		for( unsigned int i = 0; i < rcObjectsVector.size(); i++ )
		{
			CEnemy* pcEnemy = m_pcEnemies[ i ];
			pcEnemy->Initialise( rcObjectsVector[ i ], true );
		}
	}

}

void CLevelManager::CheckpointPositioning( const std::string& rsObjectGroup )
{
	ValueMap& rcObjectsValues = m_pcCurrentLevel->getObjectGroup( rsObjectGroup )->getObject( "Checkpoint" + m_sCurrentStage );

	// There is no object group with this name which means no object of this kind in this stage
	if( rcObjectsValues.size() == 0 )
	{
		return;
	}

	CCheckpoint* pcCheckpoint = m_pcCheckpoints[ 0 ];
	pcCheckpoint->Initialise( rcObjectsValues, m_pcHUD, m_iCurrentStage );
}

void CLevelManager::PlatformsPositioning( const std::string& rsObjectGroup )
{
	// Get map's current stage platforms as object group
	TMXObjectGroup* rcObjectGroup = m_pcCurrentLevel->getObjectGroup( rsObjectGroup );

	// There is no object group with this name which means no object of this kind in this stage
	if( nullptr == rcObjectGroup )
	{
		return;
	}

	ValueVector& rcObjectsVector = rcObjectGroup->getObjects();

	// IF there are platforms in the current stage this vector should NOT be empy
	CCASSERT( !rcObjectsVector.empty(), *rsObjectGroup.c_str() + "empty" );

	// Platform index
	int iPlatformID = 0;
	// Number of travellators and crumblings counted until now
	int iPlatformTravellatorCount = 0;
	int iPLatformCrumblingCount = 0;

	for( unsigned int i = 0; i < rcObjectsVector.size(); ++i )
	{
		iPlatformID = 0;

		const ValueMap& rcObjectValues = rcObjectsVector[ i ].asValueMap();

		// Position is corrected with respect to the map's anchor point which is a the bottom left of the screen
		float fOffsetCorrectionX = rcObjectValues.at( "x" ).asFloat() + rcObjectValues.at( "width" ).asFloat() * 0.5f;
		float fOffsetCorrectionY = rcObjectValues.at( "y" ).asFloat() + rcObjectValues.at( "height" ).asFloat() * 0.5f;

		std::string cType = rcObjectValues.at( "type" ).asString();

		// Do this if the current platform type is crumbling
		if( cType == "Crumbling" )
		{
			// Do this if loading the "pre-initialisation" stage
			if( -1 == m_iCurrentStage )
			{
				// Initialise all the crumbling platform in the platforms' vector based on the object values
				for( unsigned int j = 0; j < Platforms::k_iMaxAmountOfCrumblingPlatforms; j++ )
				{
					CPlatformBase* pcPlatform = m_pcPlatforms[ iPlatformID + j ];
					pcPlatform->Initialise( rcObjectValues );
				}

			}
			// Do this for every normal stage
			else
			{
				iPlatformID = +iPLatformCrumblingCount;
				// Initialise all the crumbling platform in the current stage based on the object values
				CPlatformBase* pcPlatform = m_pcPlatforms[ iPlatformID ];
				pcPlatform->Initialise( rcObjectValues );

				iPLatformCrumblingCount++;
			}
			continue;
		}

		// Travellator are stored after crumbling platforms so we skip crumbling indices
		iPlatformID += Platforms::k_iMaxAmountOfCrumblingPlatforms;

		// Do this if the current platform type is travellator
		if( cType == "Travellator" )
		{
			// Do this if loading the "pre-initialisation" stage
			if( -1 == m_iCurrentStage )
			{
				// Initialise all the travellators in the platforms' vector based on the object values
				for( unsigned int j = 0; j < Platforms::k_iMaxAmountOfTravellatorsPlatforms; j++ )
				{
					// Set position of the pickups based on the map's object positioning
					CPlatformBase* pcPlatform = m_pcPlatforms[ iPlatformID + j ];
					pcPlatform->Initialise( rcObjectValues );

				}
			}
			// Do this for every normal stage
			else
			{
				iPlatformID += iPlatformTravellatorCount;

				// Initialise all the travellators in the current stage based on the object values
				CPlatformBase* pcPlatform = m_pcPlatforms[ iPlatformID ];
				pcPlatform->Initialise( rcObjectValues );
				// Increase the amount of travellators counted until now
				iPlatformTravellatorCount++;
			}
		}
	}

	m_iNumOfTravellatorInStage = iPlatformTravellatorCount;

}

void CLevelManager::PortsPositioning( const std::string& rsObjectGroup )
{
	// Get map's current stage ports as vector of values
	ValueVector& rcObjectsVector = m_pcCurrentLevel->getObjectGroup( rsObjectGroup )->getObjects();

	CCASSERT( !rcObjectsVector.empty(), rsObjectGroup.c_str() );
	// Crossreference between the amount of pickups and ports within the stage to ensure progression
	CCASSERT( m_pcPickupsManager->GetActiveAmountOfKeys() == rcObjectsVector.size(),
		"Chips amount not match port amount in stage" );

	// Do this if loading the "pre-initialisation" stage
	if( -1 == m_iCurrentStage )
	{
		// Initialise all ports of the ports' vector with the values from the object vector
		for( CPort* pcPort : m_pcPorts )
		{
			pcPort->Initialise( rcObjectsVector[ 0 ] );
		}
	}
	// Do this for every normal stage
	else
	{
		// Initialise the amount of ports present in the current stage with the object's vector values
		for( unsigned int i = 0; i < rcObjectsVector.size(); i++ )
		{
			CPort* pcPort = m_pcPorts[ i ];
			pcPort->Initialise( rcObjectsVector[ i ] );
		}
	}

	// Set the amount of ports activatable in the current stage based on the size of the object vector
	m_pcExitDoor->SetAmountOfPortsInAStage( rcObjectsVector.size() );
}

void CLevelManager::LoadNewStage( const int iStageNumber )
{
	// Set the current stage to the parameter value passed through.
	m_iCurrentStage = iStageNumber;

	if( m_iCurrentStage == 1 && Audio::k_iAudioEnabled )
	{
		cocos2d::AudioEngine::play2d( "/Audio/Alexander Zhelanov-Battle_1.ogg", false, 0.2f );
	}

	m_sCurrentStage = std::to_string( m_iCurrentStage );

	// Position all platforms of the current stage
	PlatformsPositioning( "Platforms " + m_sCurrentStage );
	// Position all pickups of the stage level
	PickUpPositioning( "Pickups " + m_sCurrentStage );
	// Position all enemies of the current stage
	EnemiesPositioning( "Enemies " + m_sCurrentStage );
	// Position all ports of the current stage
	PortsPositioning( "Ports " + m_sCurrentStage );
	// Position all checkpoints of the current stage
	CheckpointPositioning( "Checkpoints" );
	// Position the exit door of the current stage
	ExitPositioning( "ExitDoors" );
}

void CLevelManager::ResetCurrentStage()
{
	// Get the pickups of the current stage and reset them
	TMXObjectGroup* rcPickupsObjectGroup = m_pcCurrentLevel->getObjectGroup( "Pickups " + m_sCurrentStage );
	ValueVector& rcPickupsObjectsVector = rcPickupsObjectGroup->getObjects();
	m_pcPickupsManager->ResetPickups( rcPickupsObjectsVector );

	// Reset the ports of the current stage based on how many chips are in the stage
	for( int i = 0; i < m_pcPickupsManager->GetActiveAmountOfKeys(); i++ )
	{
		CPort* pcPort = m_pcPorts[ i ];
		pcPort->Reset();
	}

	// Get the object group of the platfoms in the current stage
	TMXObjectGroup* rcPlatformsObjectGroup = m_pcCurrentLevel->getObjectGroup( "Platforms " + m_sCurrentStage );

	// If there are platforms in the stage reset them
	if( nullptr != rcPlatformsObjectGroup )
	{

		ValueVector& rcPlatformsObjectsVector = rcPlatformsObjectGroup->getObjects();

		int iPlatformID = 0;
		int iPlatformTravellatorCount = 0;
		int iPlatformCrumblingCount = 0;
		// Reset all the specific platforms present in the current stage
		for( unsigned int i = 0; i < rcPlatformsObjectsVector.size(); ++i )
		{
			iPlatformID = 0;

			ValueMap& rcObjectValues = rcPlatformsObjectsVector[ i ].asValueMap();

			if( rcObjectValues[ "type" ].asString() == "Crumbling" )
			{
				// Get the correct crumbling platform ID
				iPlatformID += iPlatformCrumblingCount;
				CPlatformBase* pPlatform = m_pcPlatforms[ iPlatformID ];
				// Reset the platform to its original state within the current room
				pPlatform->Reset();
				iPlatformCrumblingCount++;
				continue;
			}

			//iPlatformID += Platforms::k_iMaxAmountOfCrumblingPlatforms;

			//if( rcObjectValues[ "type" ].asString() == "Travellator" )
			//{
			//	iPlatformTravellatorCount++;
			//	iPlatformID += iPlatformTravellatorCount;
			//	   Set position of the pickups based on the map's object positioning
			//	( *m_pcPlatforms )[ iPlaftormID ]->Initialise( rcObjectValues );
			//}

		}
	}


	// Reset the door to the standard values
	m_pcExitDoor->ResetDoor();
}

void CLevelManager::HideSecondaryBackground()
{
	// Hide the secondary background if is visible
	if( m_pcCurrentLevel->getLayer( "Second Background" )->isVisible() == true )
	{
		m_pcCurrentLevel->getLayer( "Second Background" )->setVisible( false );
	}
	// Make the secondary background visible if it is not
	else
	{
		m_pcCurrentLevel->getLayer( "Second Background" )->setVisible( true );
	}
}

std::vector<CPlatformBase*>& CLevelManager::GetPlatforms()	{ return m_pcPlatforms; }

std::vector<CEnemy*>& CLevelManager::GetEnemies()			{ return m_pcEnemies; }

std::vector<CPort*>& CLevelManager::GetPorts()				{ return m_pcPorts; }

std::vector<CCheckpoint*>& CLevelManager::GetCheckpoints()	{ return m_pcCheckpoints; }

FastTMXTiledMap* CLevelManager::GetCurrentLevel() const		{ return m_pcCurrentLevel; }

const int CLevelManager::GetCurrentLevelID() const			{ return m_iCurrentStage; }

void CLevelManager::SetCurrentLevel( const int iLevel )		{ m_iCurrentStage = iLevel; }

