#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <cocos/2d/CCTMXTiledMap.h>
#include <cocos/2d/CCTMXXMLParser.h>

#include "Enemy.h"
#include "PlatformBase.h"

class CExitDoor;
class CPickupsManager;
class CTextureManager;

//-----------------------------------------------------------------------------------------------------------------------------
// Class Name			: CLevelManager
// Purpose				: To handle the levels during runtime.
//-----------------------------------------------------------------------------------------------------------------------------
class CLevelManager
{

private:
	int m_iCurrentLevel;

	// Pointer to the current level
	cocos2d::TMXTiledMap* m_pcCurrentLevel;

	// Physics body of the whole map that will contains only static things
	cocos2d::PhysicsBody* m_pcColliderContainer;
	
	// Pointer to the texture manager needed for child classes of the map
	CTextureManager* m_pcTextureManager;

	// Vector of pointers to tiled maps which represents the levels
	std::vector<cocos2d::TMXTiledMap*> m_pcLevels;
	
	// Vector of pointers to store all platforms of the levels
	std::vector<CPlatformBase*>* m_pcPlatforms;

	// Vector of pointers to store all enemies of the levels
	std::vector<CEnemy*>* m_pcEnemies;

	CPickupsManager* m_pcPickupsManager;	
	
	CExitDoor* m_pcExitDoor;

	//-----------------------------------------------------------------------------------------------------------------------------
	// Constructor name	: LoadAllLevels()
	// Parameters		: None
	// Purpose			: Load all the levels of the game from the settings file
	//-----------------------------------------------------------------------------------------------------------------------------
	void LoadAllMaps();

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: CreateColliderContainer()
	// Purpose			: Create empty collider for the map and set its properties
	// Notes			: Collider position is adjusted to match map's rendering origin at the bottom left corner
	//-----------------------------------------------------------------------------------------------------------------------------
	void CreateColliderContainer();

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: CreateCollidableObjects( String rcObjectGroup)
	// Parameters		: rcObjectGroup		- String for the Object group name
	// Purpose			: Retrieve a specific object group from the tilemap and add new shape to map's collider
	//					: for every object in the object group
	// Notes			: Position of the added shapes is adjusted to match position in tilemap
	//-----------------------------------------------------------------------------------------------------------------------------
	void CreateCollidableObjects( const std::string& rcObjectGroup );

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: CreateResponsiveEntities()
	// Parameters		: T						- Specific class type of the entities to create
	//					    : storage				- Templated vector of the that specify where store new entities
	//					    : rcObjectGroup			- The specific tiled object group of the platforms
	// Purpose			: Create entities of the specified object group a room, store them and add them to map
	// Example			: TCreateResponsiveEntities<CEnemy>( m_pcEnemies, "Enemies" )
	//-----------------------------------------------------------------------------------------------------------------------------
	template<typename T, typename J = std::vector<T*>>
	void TCreateResponsiveEntities( J& rStorage, const std::string& rcObjectGroup )
	{
		// Retrieve the correct map object group
		cocos2d::ValueVector& rcObjectsVector = m_pcCurrentLevel->getObjectGroup( rcObjectGroup )->getObjects();

		CCASSERT( !rcObjectsVector.empty(), rcObjectGroup.c_str() );

		// Create and add enemies from the object of the map
		for( cocos2d::Value& rcObject : rcObjectsVector )
		{	
			// Set the name of the enemy to be like "Enemy id", used for collision management
			int iID = rStorage->size();
			// Create the enemy and stores it in the platform vecto
			rStorage->push_back( new T( rcObject, *m_pcTextureManager, iID)  );
			
			// Add the enemy to the current map
			m_pcCurrentLevel->addChild( rStorage->back() );
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: PickUpPositioning()
	// Parameters		: rcObjectGroup			- The specific tiled object group of the pickups
	// Purpose			: Position correctly all pickups of the current object group
	//---------------------------------------------------------------------------------------------------------------
	void PickUpPositioning( const std::string& rcObjectGroup );

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: PickUpPositioning()
	// Author			: Gaetano Trovato
	// Parameters		: rcObjectGroup			- The specific tiled object group of the pickups
	// Purpose			: Position correctly all pickups of the current object group
	//---------------------------------------------------------------------------------------------------------------
	void ExitPositioning( const std::string& rcObjectGroup );

public:

#pragma region Constructor/Destructors

	//-----------------------------------------------------------------------------------------------------------------------------
	// Constructor name	: CLevelManager()
	// Author			: Gaetano Trovato
	// Parameters		: None
	// Purpose			: This constructor will create nothing but the class itself and its member
	//-----------------------------------------------------------------------------------------------------------------------------
	CLevelManager();

	//-----------------------------------------------------------------------------------------------------------------------------
	// Destructor name	: ~CLevelManager()
	// Editors			: None
	// Parameters		: None
	// Purpose			: This destructor will safe delete all members of the class
	//-----------------------------------------------------------------------------------------------------------------------------
	~CLevelManager();

#pragma endregion

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function name	: Initialise()
	// Parameters		  : pcTextureManager		- The texture manager of the game
	//					      : pcPickupsManager		- The pickup manager of the game
	// Purpose			  : This function will load all the levels and create the correlated object from the Tiled maps
	//-----------------------------------------------------------------------------------------------------------------------------
	void Initialise(CTextureManager* pcTextureManager, CPickupsManager* pcPickupsManager, CExitDoor* pcExitDoor );


	#pragma region Getters and Setter
	
	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: GetPlatforms()
	// Editors			  : None
	// Purpose			  : Retrieve a pointer to the platform's vector used by the level's manager
	// Return			    : m_pcPlatforms
	//-----------------------------------------------------------------------------------------------------------------------------
	std::vector<CPlatformBase*>& GetPlatforms();
	
	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: GetEnemies()
	// Editors			: None
	// Purpose			: Retrieve a pointer to the enemies's vector used by the level's manager
	// Return			: m_pcEnemies
	//-----------------------------------------------------------------------------------------------------------------------------
	std::vector<CEnemy*>& GetEnemies();

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: GetCurrentLevel()
	// Purpose			: Retrieve a pointer to the current level
	// Return			: m_pcCurrentLevel
	//-----------------------------------------------------------------------------------------------------------------------------
	cocos2d::TMXTiledMap* GetCurrentLevel() const;

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: GetCurrentLevelID()
	// Purpose			: Get the integer id of the current level
	// Return			: m_iCurrentLevel
	//-----------------------------------------------------------------------------------------------------------------------------
	const int GetCurrentLevelID() const;

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: SetCurrentLevel()
	// Parameters		: iLevel		- Integer used to specify the number of the level in the level's vector
	// Purpose			: Set the current level using the provided id
	//-----------------------------------------------------------------------------------------------------------------------------
	void SetCurrentLevel(const int iLevel );

	#pragma endregion
};

#endif // LEVELMANAGER_H
