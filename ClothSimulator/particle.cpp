////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - particle.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "particle.h"
#include "shader.h"
#include "diagnostic.h"
#include <numeric>
#include <assert.h>

namespace
{
    const int MAX_FILTERING = 5;       ///< Maximum values used for filtering
    const float PARTICLE_MASS = 1.0f;  ///< Mass in kg for single particle
}

Particle::Particle(EnginePtr engine) :
    m_acceleration(0.0f, 0.0f, 0.0f),
    m_previousPosition(0.0f, 0.0f, 0.0f),
    m_initialPosition(0.0f, 0.0f, 0.0f),
    m_positionDelta(0.0f, 0.0f, 0.0f),
    m_position(0.0f, 0.0f, 0.0f),
    m_uvs(0.0f, 0.0f),
    m_selected(false),
    m_pinned(false),
    m_color(0.0f, 0.0f, 1.0f),
    m_index(NO_INDEX),
    m_visualRadius(0.0f)
{
    m_yFiltering.resize(MAX_FILTERING);
    m_collision.reset(new DynamicMesh(engine,
        std::bind(&Particle::MovePosition, this, std::placeholders::_1)));
}

void Particle::Initialise(const D3DXVECTOR3& position, 
                          const D3DXVECTOR2& uv, 
                          unsigned int index,
                          const CollisionMesh& mesh,
                          float visualRadius)
{
    m_visualRadius = visualRadius;
    m_uvs = uv;
    m_initialPosition = position;
    m_position = position;
    m_previousPosition = position;
    m_index = index;
    m_transform.SetPosition(m_position);
    MakeZeroVector(m_acceleration);

    m_yFiltering.assign(MAX_FILTERING, 0.0f);
    MakeZeroVector(m_positionDelta);
    
    m_collision->LoadInstance(mesh);
    m_collision->PositionalNonParentalUpdate(m_position);
    m_collision->SetRenderSolverDiagnostics(false);
}

void Particle::ResetPosition()
{
    m_positionDelta.x = 0.0f;
    m_positionDelta.y = 0.0f;
    m_positionDelta.z = 0.0f;
    m_yFiltering.assign(MAX_FILTERING, 0.0f);
    m_previousPosition = m_position = m_initialPosition;
    m_transform.SetPosition(m_position);
    m_collision->PositionalNonParentalUpdate(m_position);
}

bool Particle::RequiresSmoothing() const
{
    return !m_collision->IsCollidingWith(Geometry::BOX) &&
        !m_collision->IsCollidingWith(Geometry::CYLINDER);
}

void Particle::PinParticle(bool pin)
{
    m_pinned = pin;
}

void Particle::SelectParticle(bool select)
{
    m_selected = select;
}

void Particle::DrawVisualMesh(const Matrix& projection, 
                              const Matrix& view,
                              const D3DXVECTOR3& position)
{
    m_collision->DrawRepresentation(projection, view, 
        m_visualRadius, m_color, position);
}

void Particle::DrawCollisionMesh(const Matrix& projection, const Matrix& view)
{
    m_collision->DrawMesh(projection, view);
}

CollisionMesh& Particle::GetCollisionMesh()
{
    return *m_collision;
}

void Particle::SetColor(const D3DXVECTOR3& colour)
{
    m_color = colour;
}

void Particle::MovePosition(const D3DXVECTOR3& position)
{
    if(!m_pinned)
    {
        m_position += position;
        UpdateCollisionPosition();
    }
}

void Particle::AddForce(const D3DXVECTOR3& force)
{    
    if(!m_pinned)
    {
        m_acceleration += force/PARTICLE_MASS;
    }
}

void Particle::PreCollisionUpdate(float damping, float timestepSqr)
{
    if(!m_pinned && m_collision->IsCollidingWith(Geometry::NONE))
    {
        //verlet integration
        //X(t + ∆t) = 2X(t) - X(t - ∆t) + ∆t^2X▪▪(t)
        //X(t + ∆t) = X(t) + (X(t)-X(t - ∆t)) + ∆t^2X▪▪(t)
        //X(t + ∆t) = X(t) + X▪(t) + ∆t^2X▪▪(t)
        D3DXVECTOR3 update = ((m_position-m_previousPosition)*damping) 
            + (m_acceleration*timestepSqr);

        m_previousPosition = m_position;
        m_position += update;
    }
    else
    {
        m_previousPosition = m_position;
    }

    UpdateCollisionPosition();
    MakeZeroVector(m_acceleration);
}

void Particle::UpdateCollisionPosition()
{
    if(m_position != m_previousPosition)
    {
        m_collision->PositionalNonParentalUpdate(m_position);
    }
}

void Particle::FilterPosition()
{
    m_positionDelta = m_position - m_previousPosition;

    assert(m_yFiltering.size() == MAX_FILTERING);
    m_yFiltering.pop_front();
    m_yFiltering.push_back(m_positionDelta.y);

    // Only consider changes in y position that is greater than threshold
    float amount = m_positionDelta.y;
    int count = 1;
    const float threshold = 0.1f;
    for(float value : m_yFiltering)
    {
        if(fabs(m_positionDelta.y - value) > threshold)
        {
            amount += value;
            count++;
        }
    }

    m_positionDelta.y = amount / static_cast<float>(count);
    m_position = m_previousPosition + m_positionDelta;
}

void Particle::PostCollisionUpdate()
{
    FilterPosition();
    UpdateCollisionPosition();
    m_collision->UpdateCollision();
}

void Particle::UpdateDiagnostics(Diagnostic& renderer)
{
    if(renderer.AllowDiagnostics(Diagnostic::TEXT))
    {
        renderer.UpdateText(Diagnostic::TEXT, "Particle Delta",
            Diagnostic::WHITE, StringCast(m_positionDelta.y));;

        renderer.UpdateText(Diagnostic::TEXT, 
            "Particle Collision", Diagnostic::WHITE, std::string(
            (m_collision->IsCollidingWith(Geometry::NONE) ? "NONE " : "")) +
            (m_collision->IsCollidingWith(Geometry::SPHERE) ? "SPHERE " : "") +
            (m_collision->IsCollidingWith(Geometry::BOX) ? "BOX " : "") +
            (m_collision->IsCollidingWith(Geometry::CYLINDER) ? "CYLINDER " : ""));
    }
}