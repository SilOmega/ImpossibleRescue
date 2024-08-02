#ifndef PORT_H
#define PORT_H


#include "Collider.h"
#include "SpriteObject.h"

#include "CCValue.h"
#include <ui/CocosGUI.h>

class CTextureManager;

//-----------------------------------------------------------------------------------------------------------------------------
// Class Name			: CPort
// Classes Inherited	: CSpriteObject, CCollider
// Purpose				: To handle the a port in the level. It can be activated on collision and "placed" if the player has "chip"
//-----------------------------------------------------------------------------------------------------------------------------
class CPort 
	: public CSpriteObject
	, public CCollider
{

private:
	// Pointer to port collider 
	cocos2d::PhysicsBody* m_pcCollider;
	// Pointer to texture manager
	CTextureManager& m_pcTextureManager;
	// Pointer to loading bar
	cocos2d::ui::LoadingBar* m_pcLoadingBar;

	// Boolean to check if the loading bar is filling
	bool m_IsFilling;
	// Boolean to check if the port is placed
	bool m_IsPlaced;
	// Time required to fill the loading bar
	float m_fLoadingTimeInSeconds;

	int m_iAudioID;
	// Pointer to the standing zone object
	CSpriteObject* m_pcStandingZone;

public:

	//-----------------------------------------------------------------------------------------------------------------------------
	// Constructor Name	: CPort()
	// Purpose			: Create a port with its basic elements so texture, empty collider and the loading bar used during port 
	//					: placement. Collider name is set to "Port + iiD)
	// Parameters		: rcTextureManager	- The texture manager used to set the texture
	//					: iID				- The unique ID used to identify this port during the game
	// Notes			: Collision trigger is positioned a bit lower than the sprite itself
	//-----------------------------------------------------------------------------------------------------------------------------
	CPort(  CTextureManager& rcTextureManager, const int iID );
	
	//-----------------------------------------------------------------------------------------------------------------------------
	// Destructor Name	: ~CPort()
	// Purpose			: Free memory used by the class
	//-----------------------------------------------------------------------------------------------------------------------------
	~CPort();

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: Initialise()
	// Parameters		: rcTiledObject		- The tilemap object from where the position/dimensions are taken
	// Purpose			: Position the port based on the given object values and resets it to original values. If the collider is
	//					: empty creates a physics shape based and initialise standing zone
	//-----------------------------------------------------------------------------------------------------------------------------
	void Initialise( const cocos2d::Value& rcTiledObject );

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: VTriggerResponse()
	// Purpose			: Handle the activation and placement of the port. When collision box is triggered, the loading bar will 
	//					: fill as long as the stands in it. On bar's completition it sends an event to the Exit Door to acknowledge 
	//					: the placement success
	//-----------------------------------------------------------------------------------------------------------------------------
	void VTriggerResponse() override;

	//-----------------------------------------------------------------------------------------------------------------------------
	// Function Name	: Reset()
	// Purpose			: Reset the port to default values
	//-----------------------------------------------------------------------------------------------------------------------------
	void Reset();
};
	

#endif // !PORT_H

