#ifndef PLATFORMBASE_H
#define PLATFORMBASE_H


#include "Collider.h"
#include "SpriteObject.h"

#include <CCValue.h>
#include <cocos/physics/CCPhysicsBody.h>

class CTextureManager;

//-----------------------------------------------------------------------------------------------------------------------------
// Class Name			: CPlatformBase
// Classes Inherited	: CSpriteObject, CCollider
// Purpose				: To create and define a generic platform of a level.
//-----------------------------------------------------------------------------------------------------------------------------
class CPlatformBase:
	public CSpriteObject,
	public CCollider
{

protected:

	// Collider of the platform
	cocos2d::PhysicsBody* m_pcCollider;
	// Shape of the platform collider
	cocos2d::PhysicsShapeBox* m_pcBoxShape;
	// Platform can be triggered or not
	bool m_bCanBeTriggered;

public:

	//-----------------------------------------------------------------------------------------------------------------------------
	// Constructor name	: CPlatformBase()
	// Purpose			: This constructor will create a platform with an empty collider, no texture and set it invisible
	//-----------------------------------------------------------------------------------------------------------------------------
	CPlatformBase();

	//-----------------------------------------------------------------------------------------------------------------------------
	// Constructor name	: CPlatformBase()
	// Parameters		: iID				- Id of this platform in the platforms' vector, used for collision management
	// Purpose			: This constructor will create a platform with an empty collider named "Platform iID", no texture, 
	//					: set it invisible
	//-----------------------------------------------------------------------------------------------------------------------------
	CPlatformBase( const int iID );

	//-----------------------------------------------------------------------------------------------------------------------------
	// Destructor name	:  ~CPlatformBase()
	// Purpose			: This destructor will destroy nothing but the class itself
	//-----------------------------------------------------------------------------------------------------------------------------
	virtual ~CPlatformBase();

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function name	: Initialise()
	// Parameters		: rcTiledObject		-	Tiled object from where take position values
	// Purpose			: Set the position of the platform based on the parameters values and makes it visible
	//-----------------------------------------------------------------------------------------------------------------------------
	virtual void Initialise( const cocos2d::ValueMap& rcObjectValues );

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function name	: SetID()
	// Parameters		: iID				- integer used as the identifier
	// Purpose			: Set the name of the platform's collider to be "Platform + iID", used for trigger/collision identificaiton 
	//-----------------------------------------------------------------------------------------------------------------------------
	void SetID( const int iID );

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function name	: Reset()
	// Purpose			: Empty method, it has to be defined by children classes
	//-----------------------------------------------------------------------------------------------------------------------------
	virtual void Reset();
};

#endif // !PLATFORMBASE_H
