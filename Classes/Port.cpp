#include "Port.h"

#include <AudioEngine.h>
#include <CCEventCustom.h>
#include <CCEventDispatcher.h>

#include "TextureManager.h"
#include "Settings.h"

using cocos2d::Vec2;
using cocos2d::PhysicsShapeBox;

CPort::CPort( CTextureManager& rcTextureManager, const int iID )
	: m_pcCollider( nullptr )
	, m_pcTextureManager( rcTextureManager )
	, m_IsFilling( false )
	, m_IsPlaced( false )
	, m_fLoadingTimeInSeconds( 2.0 )
	, m_iAudioID( 0 )
	, m_pcStandingZone( nullptr )
{

	// Initialise the port's sprite using the texture manager
	CreateSprite( m_pcTextureManager.GetTexture( EGameTextures::Port ), true, 1, 2 );

	// Create an empty sprite for the standing zone 
	m_pcStandingZone = new CSpriteObject();

	// Create port collider
	m_pcCollider = cocos2d::PhysicsBody::create();
	m_pcCollider->setDynamic( false );
	addComponent( m_pcCollider );

	// Setting the name of the platform with its ID, used for collision management
	m_pcCollider->setName( "Port " + std::to_string( iID ) );

	// Create a loading bar that will be used as visual timer for port placement
	m_pcLoadingBar = cocos2d::ui::LoadingBar::create( "MP_Meter2.png" );
	m_pcLoadingBar->setScale( 0.06f );
	// Set bar's filling direction from left to right
	m_pcLoadingBar->setDirection( cocos2d::ui::LoadingBar::Direction::LEFT );
	// Position the bar a bit higher than the port sprite
	m_pcLoadingBar->setPosition( Vec2( 0.0f, getContentSize().height ) );
	m_pcLoadingBar->setAnchorPoint( Vec2( 0.25f, 0 ) );

	setVisible( false );

	addChild( m_pcLoadingBar );
}

CPort::~CPort() {}

void CPort::Initialise( const cocos2d::Value& rcTiledObject )
{
	const cocos2d::ValueMap& rcObjectValues = rcTiledObject.asValueMap();

	CCASSERT( !rcObjectValues.empty(), "No values in the tiled object" );

	if( m_pcCollider->getShape( 0 ) == nullptr )
	{
		// Copy the tiled object's size to a variable
		Vec2 cObjectSize = Vec2( rcObjectValues.at( "width" ).asFloat(), rcObjectValues.at( "height" ).asFloat() );

		// Storing a negative offset on the y axis
		Vec2 cColliderOffsetFromSprite = Vec2( 0.0f, -16.0f );
		// Create physics shape of the object size placed lower than the port itself
		PhysicsShapeBox* pcBoxShape = PhysicsShapeBox::create( cocos2d::Size( cObjectSize ), 
			cocos2d::PhysicsMaterial( 1.0, 0.0, 1.0 ), cColliderOffsetFromSprite );
		m_pcCollider->addShape( pcBoxShape, false );

		pcBoxShape->setCategoryBitmask( TRIGGER_BITMASK_CATEGORY );
		pcBoxShape->setCollisionBitmask( TRIGGER_BITMASK_COLLIDER );
		pcBoxShape->setContactTestBitmask( TRIGGER_BITMASK_CONTACT );

		// Create a sprite for the standing zone
		m_pcStandingZone->CreateSprite( m_pcTextureManager.GetTexture( EGameTextures::PortStandingZone ), false );
		// Making the standing zone's sprite a bit transparent
		m_pcStandingZone->setOpacity( 170 );
		// Scale it to the size of the actual trigger physics shape and place it at the same position
		m_pcStandingZone->setScaleX( cObjectSize.x / m_pcStandingZone->getContentSize().width );
		m_pcStandingZone->setScaleY( 1.0f / 16.0 );
		m_pcStandingZone->setPosition( -Vec2( getContentSize().width * getScaleX() * 0.10f, 10.0f ) );

		addChild( m_pcStandingZone );

	}

	// Position the port in the coordinates given by the tiled object
	float fOffsetCorrectionX = rcObjectValues.at( "x" ).asFloat();
	float fOffsetCorrectionY = rcObjectValues.at( "y" ).asFloat() + rcObjectValues.at( "height" ).asFloat() * 0.5f;
	setPosition( fOffsetCorrectionX, fOffsetCorrectionY );

	Reset();
}


void CPort::VTriggerResponse()
{
	// If the port is placed do nothing
	if( !m_IsPlaced )
	{
		// If the port is not placed and the loading bar is not filling
		if( !m_IsFilling )
		{
			if( Audio::k_iAudioEnabled )
			{
				m_iAudioID = cocos2d::AudioEngine::play2d( "/Audio/turbolift_05.ogg", false, 0.9f );
			}

			// Schedule the filling function to fill the bar over x amount of seconds
			this->schedule( [=]( float delta ){

				// Retrieve and update the loading bar percentage
				float percent = m_pcLoadingBar->getPercent();
				percent++;
				m_pcLoadingBar->setPercent( percent );
				
				// If the bar is fully filled
				if( percent >= 100.0f )
				{
					// Bar is now placed
					m_IsPlaced = !m_IsPlaced;

					// Create and send an event to acknowledge the activation
					cocos2d::EventCustom portActivated( "Port_Activated" );
					_eventDispatcher->dispatchEvent( &portActivated );

					// Create and send an event to acknowledge the usage of chip
					cocos2d::EventCustom chipUsed( "Chip_Used" );
					_eventDispatcher->dispatchEvent( &chipUsed );

					// Set the animation state of the port to on | Nikodem Hamrol
					SetAnimationState( GetSpriteFrameHeight(), false, 0.0f, 1 );

					// Unschedule the filling function
					this->unschedule( "updateLoadingBar" );

					// Deactivate the loading bar and standing zone if the port has been placed
					m_pcLoadingBar->setVisible( false );
					m_pcStandingZone->setVisible( false );
				}
			}, m_fLoadingTimeInSeconds * 0.01f, "updateLoadingBar" );

			// Bar is currently filling by this point
			m_IsFilling = !m_IsFilling;
		}
		// If the port is filling (at this point it means the player stepped out trigger collider
		else
		{
			cocos2d::AudioEngine::stop( m_iAudioID );
			// Unschedule the filling function
			this->unschedule( "updateLoadingBar" );
			// Reset set percentage to 0
			m_pcLoadingBar->setPercent( 0.0 );
			// The bar is not filling anymore
			m_IsFilling = !m_IsFilling;
		}
	}

}

void CPort::Reset()
{
	// Set the class members to default values
	m_IsPlaced = false;
	m_IsFilling = false;
	m_pcLoadingBar->setPercent( 0.0f );
	m_pcLoadingBar->setVisible( true );
	m_pcStandingZone->setVisible( true );
	setVisible( true );
	// Set the state state of the port to on | Nikodem Hamrol
	SetAnimationState( 0, false, 0.0f, 2 );
}

