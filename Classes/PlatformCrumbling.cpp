#include "PlatformCrumbling.h"

#include "cocos/2d/CCActionInterval.h"
#include "cocos/2d/CCActionInstant.h"

#include "Settings.h"
#include "TextureManager.h"

using cocos2d::Vec2;
using cocos2d::ValueMap;


CPlatformCrumbling::CPlatformCrumbling( CTextureManager& rcTextureManager, const int iID )
	: CPlatformBase( iID )
	, v2StartingPos( Vec2::ZERO )
{
	// Initialise the platform's sprite
	CreateSprite( rcTextureManager.GetTexture( EGameTextures::Platform ), false );
}

CPlatformCrumbling::~CPlatformCrumbling() {}

void CPlatformCrumbling::Initialise( const ValueMap& rcObjectValues )
{
	// Call base class initialisation
	CPlatformBase::Initialise( rcObjectValues );

	// Store the values from the paramters
	float fHeight = rcObjectValues.at( "height" ).asFloat();
	float fWidth = rcObjectValues.at( "width" ).asFloat();

	// Rescale the size of the platform to match the one specified by the object's values
	setScaleX( fWidth / getContentSize().width );
	setScaleY( fHeight / getContentSize().height );

	// Create a physics shape if not preset
	if( nullptr == m_pcCollider->getShape( 0 ) )
	{
		// Create physics shape of the platform size 
		m_pcBoxShape = cocos2d::PhysicsShapeBox::create( getContentSize(), cocos2d::PhysicsMaterial( 1.0, 0.0, 1.0 ),
			getContentSize() * 0.5f );
		m_pcCollider->addShape( m_pcBoxShape, false );

		// Set shape to collide with player
		m_pcBoxShape->setCategoryBitmask( PLATFORM_BITMASK_CATEGORY );
		m_pcBoxShape->setCollisionBitmask( PLATFORM_BITMASK_COLLIDER );
		m_pcBoxShape->setContactTestBitmask( PLATFORM_BITMASK_CONTACT );
	}

	// Store the starting position
	v2StartingPos = getPosition();

	// Reset the platform for precaution
	Reset();
}

void CPlatformCrumbling::VCollisionResponse()
{

	// Activate the response once until platform get re-initialised
	if( m_bCanBeTriggered )
	{
		// Move the platform down half of its height in 1 second
		cocos2d::MoveBy* pcMoveBy = cocos2d::MoveBy::create( 1.0f, Vec2( 0, -m_pcBoxShape->getSize().height * 0.25f ) );
		// Fade out the platform's sprite over 1 second
		cocos2d::FadeOut* pcFadeOut = cocos2d::FadeOut::create( 1.0f );
		// Disable collider, 
		cocos2d::CallFunc* pcDisable = cocos2d::CallFunc::create( [&]() { m_pcCollider->setEnabled( false ); } );
		// Move and fade simultaneously
		cocos2d::Spawn* pcSpawn = cocos2d::Spawn::createWithTwoActions( pcMoveBy, pcFadeOut );
		// Perform spawn first then disable
		cocos2d::Sequence* pcSequence = cocos2d::Sequence::create( pcSpawn, pcDisable, nullptr );
		runAction( pcSequence );
		// Cannot be triggered again
		m_bCanBeTriggered = false;

	}

}

void CPlatformCrumbling::Reset()
{
	this->stopAllActions();

	// Set position to the one specified originally in initialisation
	setPosition( v2StartingPos );
	// Platform can be triggered again
	m_bCanBeTriggered = true;
	// Enable the collider
	m_pcCollider->setEnabled( true );
	// Set opacity to max to make che platform visible
	setOpacity( 255 );
}
