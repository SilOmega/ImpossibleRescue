#include "PlatformBase.h"

#include "Settings.h"
#include "TextureManager.h"

using cocos2d::Size;
using cocos2d::Vec2;
using cocos2d::Value;
using cocos2d::PhysicsMaterial;

CPlatformBase::CPlatformBase()
	: m_pcCollider( nullptr )
	, m_bCanBeTriggered( true )
	, m_pcBoxShape( nullptr )
{
	// Create platform collider and set it to ignore gravity
	m_pcCollider = cocos2d::PhysicsBody::create();
	m_pcCollider->setDynamic( false );

	// Add the collider to the platform
	addComponent( m_pcCollider );
	// Not visible by default
	setVisible( false );

}

CPlatformBase::CPlatformBase( const int iID )
	: CPlatformBase()
{
	// Setting the name of the platform with its ID, used for collision management
	m_pcCollider->setName( "Platform " + std::to_string( iID ) );
}

CPlatformBase::~CPlatformBase() {}

void CPlatformBase::Initialise( const cocos2d::ValueMap& rcObjectValues )
{
	CCASSERT( !rcObjectValues.empty(), "No values in the tiled object" );

	// Set position of the platform to the coordinates given by the parameters values
	setPosition( rcObjectValues.at( "x" ).asFloat(), rcObjectValues.at( "y" ).asFloat() );

	setVisible( true );
}

void CPlatformBase::SetID( const int iID )
{
	// Setting the name of the platform with its ID, used for collision management
	m_pcCollider->setName( "Platform " + std::to_string( iID ) );
}

void CPlatformBase::Reset() {}
