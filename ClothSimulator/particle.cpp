
#include "particle.h"
#include "collision.h"

namespace
{
    const int PARTICLE_MESH_QUALITY = 8;          ///< Collision/visual mesh smoothness of the sphere
    const float PARTICLE_VISUAL_RADIUS = 0.5f;    ///< Visual draw radius of the particle
    const float PARTICLE_MASS = 1.0f;             ///< Mass in kg for single particle
}

Particle::Particle(LPDIRECT3DDEVICE9 d3ddev, float radius, const D3DXVECTOR3& position, unsigned int index) :
    m_pinned(false),
    m_selected(false),
    m_initialPosition(position.x, position.y, position.z),
    m_position(position.x, position.y, position.z),
    m_oldPosition(position.x, position.y, position.z),
    m_index(index)
{
    m_collision.reset(new CollisionSphere(d3ddev, m_transform, radius, PARTICLE_MESH_QUALITY));
    m_collision->SetDraw(true);

    Transform::UpdateFn fullFn = std::bind(&CollisionSphere::FullUpdate, m_collision.get());
    Transform::UpdateFn positionalFn = std::bind(&CollisionSphere::PositionalUpdate, m_collision.get());
    m_transform.SetObserver(fullFn, positionalFn);
    m_transform.SetPosition(m_position);
}

void Particle::ResetPosition()
{
    m_oldPosition = m_position = m_initialPosition;
    m_transform.SetPosition(m_position);
}

void Particle::ResetAcceleration()
{ 
    m_acceleration.SetToZero(); 
}

void Particle::PinParticle(bool pin)
{
    m_pinned = pin;
}

void Particle::SelectParticle(bool select)
{
    m_selected = select;
}

void Particle::DrawVisualMesh(const Transform& projection, const Transform& view)
{
    m_collision->DrawWithRadius(projection, view, PARTICLE_VISUAL_RADIUS);
}

void Particle::DrawCollisionMesh(const Transform& projection, const Transform& view)
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

void Particle::MovePosition(const FLOAT3& v)
{
    if(!m_pinned)
    {
        m_position += v;
        m_transform.SetPosition(m_position);
    }
};

void Particle::AddForce(const FLOAT3& force)
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
        //VERLET INTEGRATION
        //X(t + ∆t) = 2X(t) - X(t - ∆t) + ∆t^2X▪▪(t)
        //X(t + ∆t) = X(t) + (X(t)-X(t - ∆t)) + ∆t^2X▪▪(t)
        //X(t + ∆t) = X(t) + X▪(t) + ∆t^2X▪▪(t)
        m_position += ((m_position-m_oldPosition)*damping) + (m_acceleration*timestepSqr);

        //save the old position
        m_oldPosition.x = m_transform.Matrix._41;
        m_oldPosition.y = m_transform.Matrix._42;
        m_oldPosition.z = m_transform.Matrix._43;

        //reset acceleration
        m_acceleration.SetToZero();

        //update the collision mesh
        m_transform.SetPosition(m_position);
    }
}