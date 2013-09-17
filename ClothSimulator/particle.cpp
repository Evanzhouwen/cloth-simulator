////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - particle.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "particle.h"
#include "shader.h"

namespace
{
    const float PARTICLE_VISUAL_RADIUS = 0.15f; ///< Visual draw radius of the particle
    const float PARTICLE_MASS = 1.0f;           ///< Mass in kg for single particle
}

Particle::Particle(LPDIRECT3DDEVICE9 d3ddev, const RenderCallbacks& callbacks) :
    m_acceleration(0.0f, 0.0f, 0.0f),
    m_oldPosition(0.0f, 0.0f, 0.0f),
    m_initialPosition(0.0f, 0.0f, 0.0f),
    m_position(0.0f, 0.0f, 0.0f),
    m_selected(false),
    m_pinned(false),
    m_index(NO_INDEX)
{
    m_collision.reset(new Collision(m_transform, 
        callbacks.getShader(ShaderManager::BOUNDS_SHADER)));

    m_collision->SetDraw(true);
    
    Transform::UpdateFn fullFn = std::bind(&Collision::FullUpdate, m_collision.get());
    Transform::UpdateFn positionalFn = std::bind(&Collision::PositionalUpdate, m_collision.get());
    m_transform.SetObserver(fullFn, positionalFn);
}

void Particle::Initialise(const D3DXVECTOR3& position, unsigned int index,
    std::shared_ptr<Collision::Geometry> geometry, const Collision::Data& data)
{
    m_collision->LoadInstance(data, geometry);
    ResetAcceleration();
    m_initialPosition = position;
    m_position = position;
    m_oldPosition = position;
    m_index = index;
    m_transform.SetPosition(m_position);
}

void Particle::ResetPosition()
{
    m_oldPosition = m_position = m_initialPosition;
    m_transform.SetPosition(m_position);
}

void Particle::ResetAcceleration()
{ 
    m_acceleration.x = 0.0f;
    m_acceleration.y = 0.0f;
    m_acceleration.z = 0.0f;
}

void Particle::ResetToPreviousPosition()
{
    m_position = m_oldPosition;
    m_transform.SetPosition(m_position);
    ResetAcceleration();
}

void Particle::PinParticle(bool pin)
{
    m_pinned = pin;
}

void Particle::SelectParticle(bool select)
{
    m_selected = select;
}

void Particle::DrawVisualMesh(const Matrix& projection, const Matrix& view)
{
    m_collision->DrawWithRadius(projection, view, PARTICLE_VISUAL_RADIUS);
}

void Particle::DrawCollisionMesh(const Matrix& projection, const Matrix& view)
{
    m_collision->Draw(projection, view);
}

Collision* Particle::GetCollision()
{
    return m_collision.get();
}

void Particle::SetColor(const D3DXVECTOR3& colour)
{
    m_collision->SetColor(colour);
}

void Particle::MovePosition(const D3DXVECTOR3& position)
{
    if(!m_pinned)
    {
        m_position += position;
        m_transform.SetPosition(m_position);
    }
};

void Particle::AddForce(const D3DXVECTOR3& force)
{    
    if(!m_pinned)
    {
        m_acceleration += force/PARTICLE_MASS;
    }
}

void Particle::Update(float damping, float timestepSqr)
{
    if(!m_pinned)
    {
        //verlet integration
        //X(t + ∆t) = 2X(t) - X(t - ∆t) + ∆t^2X▪▪(t)
        //X(t + ∆t) = X(t) + (X(t)-X(t - ∆t)) + ∆t^2X▪▪(t)
        //X(t + ∆t) = X(t) + X▪(t) + ∆t^2X▪▪(t)
        m_position += ((m_position-m_oldPosition)*damping) + (m_acceleration*timestepSqr);

        //save the old position
        m_oldPosition.x = m_transform.GetMatrix()._41;
        m_oldPosition.y = m_transform.GetMatrix()._42;
        m_oldPosition.z = m_transform.GetMatrix()._43;

        //reset acceleration
        ResetAcceleration();

        //update the collision mesh
        m_transform.SetPosition(m_position);
    }
}