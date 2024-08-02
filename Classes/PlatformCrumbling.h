#ifndef PLATFORMCRUMBLING_H
#define PLATFORMCRUMBLING_H

#include "PlatformBase.h"

class CTextureManager;

//-----------------------------------------------------------------------------------------------------------------------------
// Class name			: CTravellator
// Classes Inherited	: CSpriteObject
// Purpose				: To create and define a platform that lowers and disappear if the player is standing on it
//-----------------------------------------------------------------------------------------------------------------------------
class CPlatformCrumbling:
	public CPlatformBase
{

private:
	// Starting position in the room in case the platform needs to be regenerated
	cocos2d::Vec2 v2StartingPos;

public:

	//-----------------------------------------------------------------------------------------------------------------------------
	// Constructor name	: CPlatformCrumbling()
	// Parameters		: rcTextureManager		- The texture manager used to load the platform sprite
	//					: iID					- Id of this platform in the platforms' vector, used for collision management
	// Purpose			: This constructor will create the platform with an empty collider and a sprite. The collider's name is set
	//					: to "Platform iID"
	//-----------------------------------------------------------------------------------------------------------------------------
	CPlatformCrumbling( CTextureManager& rcTextureManager, const int iID );

	//-----------------------------------------------------------------------------------------------------------------------------
	// Destructor name	: CPlatformCrumbling()
	// Purpose			: This destructor will destroy nothing but the class itself
	//-----------------------------------------------------------------------------------------------------------------------------
	~CPlatformCrumbling();

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function name	: Initialise()
	// Parameters		: rcObjectValues
	// Purpose			: Based on the parameter's values position and scales the platform. Saves the starting position and resets
	//					: it to the original state. If the collider is empty creates an appropriated physics shape
	//-----------------------------------------------------------------------------------------------------------------------------
	void Initialise( const cocos2d::ValueMap& rcObjectValues );

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function name	: VCollisionResponse()
	// Purpose			: Lowers the platform and disable its collider after 1 second
	//-----------------------------------------------------------------------------------------------------------------------------
	void VCollisionResponse() override;

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function name	: Reset()
	// Purpose			: This function will reset the platform to its original position and state
	//-----------------------------------------------------------------------------------------------------------------------------
	void Reset() override;
};

#endif // !PLATFORMCRUMBLING_H
