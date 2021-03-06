////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - manipulator.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "manipulator.h"
#include "mesh.h"
#include "shader.h"
#include "collisionmesh.h"
#include "picking.h"

namespace
{
    const float TRANSLATION_SPEED = 13.0f; ///< Speed of translating a mesh
    const float ROTATION_SPEED = 5.0f; ///< Speed of rotating a mesh
    const float SCALE_SPEED = 10.0f; ///< Speed of scaling a mesh

    const float POINT_SIZE = 0.35f; ///< Scale of the animation points
    const float LINE_SPACING = 1.0f; ///< Spacing between spheres in the animation line
    const int MESH_SEGMENTS = 8; ///< Quality of the animation sphere

    const D3DXVECTOR3 ANIMATION_COLOR(1.0f, 1.0f, 1.0f); ///< Animation point color
    const D3DXVECTOR3 LINE_COLOR(0.5f, 0.5f, 1.0f); ///< Animation line color
    const std::string MODEL_FOLDER(".\\Resources\\Models\\"); ///< Folder for all models
};

Manipulator::Manipulator(EnginePtr engine) :
    m_engine(engine),
    m_selectedTool(NONE),
    m_selectedAxis(NO_AXIS),
    m_sphere(nullptr),
    m_saveAnimation(false)
{
    m_shader = engine->getShader(ShaderManager::TOOL_SHADER);
    D3DXCreateSphere(engine->device(), POINT_SIZE, 
        MESH_SEGMENTS, MESH_SEGMENTS, &m_sphere, NULL);

    m_tools.resize(MAX_TOOLS);
    m_tools[MOVE] = std::unique_ptr<Tool>(new Tool("move", MOVE, engine));
    m_tools[ROTATE] = std::unique_ptr<Tool>(new Tool("rotate", ROTATE, engine));
    m_tools[SCALE] = std::unique_ptr<Tool>(new Tool("scale", SCALE, engine));
    m_tools[ANIMATE] = std::unique_ptr<Tool>(new Tool("move", ANIMATE, engine));
}

Manipulator::~Manipulator()
{
}

Manipulator::Tool::Tool(const std::string& name, int index, EnginePtr engine)
{
    auto toolshader = engine->getShader(ShaderManager::TOOL_SHADER);

    axis.resize(MAX_AXIS);
    std::generate(axis.begin(), axis.end(), 
        [&engine](){ return Manipulator::MeshPtr(new Mesh(engine)); });

    axis[X_AXIS]->LoadMesh(engine->device(), MODEL_FOLDER + name + "X.obj", toolshader, X_AXIS);
    axis[Y_AXIS]->LoadMesh(engine->device(), MODEL_FOLDER + name + "Y.obj", toolshader, Y_AXIS);
    axis[Z_AXIS]->LoadMesh(engine->device(), MODEL_FOLDER + name + "Z.obj", toolshader, Z_AXIS);
}

void Manipulator::ChangeTool(ToolType type)
{
    // toggle off tool if selecting twice
    m_selectedTool = (m_selectedTool == type ? NONE : type);
    m_selectedAxis = NO_AXIS;
    m_saveAnimation = (m_selectedTool == ANIMATE);
}

std::string Manipulator::GetDescription(Manipulator::ToolAxis axis) const
{
    switch(axis)
    {
    case X_AXIS:
        return "X Axis";
    case Y_AXIS:
        return "Y Axis";
    case Z_AXIS:
        return "Z Axis";
    default:
        return "None";
    }
}

std::string Manipulator::GetDescription(Manipulator::ToolType type) const
{
    switch(type)
    {
    case MOVE:
        return "Move";
    case ROTATE:
        return "Rotate";
    case SCALE:
        return "Scale";
    case ANIMATE:
        return "Animate";
    case NONE:
    default:
        return "None";
    }
}

void Manipulator::MousePickTest(Picking& input)
{
    if(m_selectedTool != NONE)
    {
        if(m_selectedAxis != NO_AXIS)
        {
            m_tools[m_selectedTool]->axis[m_selectedAxis]->SetColor(1.0f, 1.0f, 1.0f);
        }

        ToolAxis selectedAxis = NO_AXIS;
        for(unsigned int i = 0; i < m_tools[m_selectedTool]->axis.size(); ++i)
        {
            if(m_tools[m_selectedTool]->axis[i]->MousePickingTest(input))
            {
                selectedAxis = static_cast<ToolAxis>(i);
            }
        }
        input.LockMesh(selectedAxis != NO_AXIS);
        m_selectedAxis = selectedAxis;

        if(m_selectedAxis != NO_AXIS)
        {
            m_tools[m_selectedTool]->axis[m_selectedAxis]->SetColor(1.0f, 0.0f, 0.0f);
        }
    }
}

void Manipulator::UpdateState(Manipulator::MeshPtr mesh, const D3DXVECTOR2& direction, 
    const Matrix& world, const Matrix& invProjection, bool pressed, float deltatime)
{
    if(m_selectedTool == NONE)
    {
        return;
    }

    // Ensure the tool axis are aligned with the mesh axis
    std::for_each(m_tools[m_selectedTool]->axis.begin(), 
        m_tools[m_selectedTool]->axis.end(), [&mesh](const MeshPtr& axis)
    {
        axis->SetRotationMatrix(mesh->GetRotationMatrix());
    });

    if(pressed)
    {
        if(m_selectedAxis != NO_AXIS && D3DXVec2Length(&direction) > 0.0f)
        {
            D3DXVECTOR3 axis;
            switch(m_selectedAxis)
            {
            case X_AXIS:
                axis = mesh->Right();
                break;
            case Y_AXIS:
                axis = mesh->Up();
                break;
            case Z_AXIS:
                axis = mesh->Forward();
                break;
            }

            D3DXVECTOR3 mouseDirection(direction.x, direction.y, CAMERA_NEAR);
            mouseDirection.x *= -1.0f;

            // Transform the screen space mouse direction into global 3D coordinates
            // Camera world matrix is the inverse view matrix
            D3DXVec3TransformNormal(&mouseDirection, &mouseDirection, &invProjection.GetMatrix());
            D3DXVec3TransformNormal(&mouseDirection, &mouseDirection, &world.GetMatrix());

            D3DXVec3Normalize(&mouseDirection, &mouseDirection);
            D3DXVec3Normalize(&axis, &axis);

            const float dot = D3DXVec3Dot(&axis, &mouseDirection);
            const float angle = RadToDeg(std::acos(dot));
            const float speed = fabs(dot) * (angle > 90.0f ? -1.0f : 1.0f) * deltatime;

            if(m_engine->diagnostic()->AllowDiagnostics(Diagnostic::MESH))
            {
                m_engine->diagnostic()->UpdateLine(Diagnostic::MESH,
                    "MouseDirection3D", Diagnostic::WHITE, mesh->Position(),
                    mesh->Position() + mouseDirection * 20.0f);

                m_engine->diagnostic()->UpdateText(Diagnostic::MESH,
                    "MovementDot", Diagnostic::WHITE, StringCast(dot));

                m_engine->diagnostic()->UpdateText(Diagnostic::MESH,
                    "MovementAngle", Diagnostic::WHITE, StringCast(angle));
            }

            switch(m_selectedTool)
            {
            case ROTATE:
                RotateMesh(mesh, speed * ROTATION_SPEED);
                break;
            case MOVE:
                mesh->ResetAnimation();
                TranslateMesh(mesh, speed * TRANSLATION_SPEED);
                break;
            case SCALE:
                ScaleMesh(mesh, speed * SCALE_SPEED);
                break;
            case ANIMATE:
                AnimateMesh(mesh, speed * TRANSLATION_SPEED);
                break;
            }
        }
    }

    if(m_selectedTool == ANIMATE)
    {
        if(mesh->GetAnimationPoints().empty() || (!pressed && m_saveAnimation))
        {
            m_saveAnimation = false;
            mesh->SavePosition();
        }
    }
}

void Manipulator::TranslateMesh(Manipulator::MeshPtr mesh, float value)
{
    if(m_selectedAxis == X_AXIS)
    {
        mesh->Translate(value, 0.0f, 0.0f);
    }
    else if(m_selectedAxis == Y_AXIS)
    {
        mesh->Translate(0.0f, value, 0.0f);
    }
    else if(m_selectedAxis == Z_AXIS)
    {
        mesh->Translate(0.0f, 0.0f, value);
    }
}

void Manipulator::RotateMesh(Manipulator::MeshPtr mesh, float value)
{
    if(m_selectedAxis == X_AXIS)
    {
        mesh->RotateAroundAxis(value, mesh->Up());
    }
    else if(m_selectedAxis == Y_AXIS)
    {
        mesh->RotateAroundAxis(value, mesh->Forward());
    }
    else if(m_selectedAxis == Z_AXIS)
    {
        mesh->RotateAroundAxis(value, mesh->Right());
    }
}

void Manipulator::ScaleMesh(Manipulator::MeshPtr mesh, float value)
{
    const auto shape = mesh->GetCollisionMesh().GetShape();
    if(shape == Geometry::SPHERE)
    {
        // Sphere scales uniformly
        mesh->Scale(value, value, value);
    }
    else if(shape == Geometry::CYLINDER)
    {
        // Cylinder scales uniformly across x/y axis
        if(m_selectedAxis == Z_AXIS)
        {
            mesh->Scale(0.0f, 0.0f, value);
        }
        else
        {
            mesh->Scale(value, value, 0.0f);
        }
    }
    else
    {
        if(m_selectedAxis == X_AXIS)
        {
            mesh->Scale(value, 0.0f, 0.0f);
        }
        else if(m_selectedAxis == Y_AXIS)
        {
            mesh->Scale(0.0f, value, 0.0f);
        }
        else if(m_selectedAxis == Z_AXIS)
        {
            mesh->Scale(0.0f, 0.0f, value);
        }
    }
}

void Manipulator::AnimateMesh(MeshPtr mesh, float value)
{
    m_saveAnimation = true;
    TranslateMesh(mesh, value);
}

void Manipulator::UpdateDiagnostics(const MeshPtr selectedMesh)
{
    const float length = 5.0f;
    const auto position = selectedMesh->Position();

    m_engine->diagnostic()->UpdateText(Diagnostic::MESH, "SelectedTool", 
        Diagnostic::WHITE, GetDescription(m_selectedTool));

    m_engine->diagnostic()->UpdateText(Diagnostic::MESH, "SelectedAxis", 
        Diagnostic::WHITE, GetDescription(m_selectedAxis));

    m_engine->diagnostic()->UpdateText(Diagnostic::MESH, "AnimationPoints",
        Diagnostic::WHITE, StringCast(selectedMesh->GetAnimationPoints().size()));

    m_engine->diagnostic()->UpdateLine(Diagnostic::MESH, "MeshXaxis", 
        Diagnostic::YELLOW, position, position + (selectedMesh->Right() * length));

    m_engine->diagnostic()->UpdateLine(Diagnostic::MESH, "MeshYaxis", 
        Diagnostic::RED, position, position + (selectedMesh->Up() * length));

    m_engine->diagnostic()->UpdateLine(Diagnostic::MESH, "MeshZaxis", 
        Diagnostic::GREEN, position, position + (selectedMesh->Forward() * length));
}

void Manipulator::Render(const Matrix& projection, const Matrix& view,
        const D3DXVECTOR3& position, const Manipulator::MeshPtr& selectedMesh)
{
    if(m_selectedTool != NONE)
    {
        if(m_selectedTool == ANIMATE)
        {
            // As the mesh is moving, draw white dots when 
            // it stops and blue dots for the motion in between

            m_shader->SetTechnique(DxConstant::DefaultTechnique);
            const std::vector<D3DXVECTOR3> points = selectedMesh->GetAnimationPoints();

            Transform world;
            world.SetPosition(points[0]);
            RenderSphere(m_shader, projection, view, ANIMATION_COLOR, world);
            const float lineScale = 0.5f;

            for(unsigned int i = 1; i < points.size(); ++i)
            {
                world.SetPosition(points[i]);
                RenderSphere(m_shader, projection, view, ANIMATION_COLOR, world);
            
                world.SetScale(lineScale);
                D3DXVECTOR3 line(points[i]-points[i-1]);
                const float length = D3DXVec3Length(&line);
                line /= length;

                const int spheres = static_cast<int>(std::ceil(length / LINE_SPACING));
                for(int j = 1; j < spheres; ++j)
                {
                    world.SetPosition(points[i-1] + (line * (j * LINE_SPACING)));
                    RenderSphere(m_shader, projection, view, LINE_COLOR, world);
                }
                world.SetScale(1.0f);
            }
        }

        std::for_each(m_tools[m_selectedTool]->axis.begin(), m_tools[m_selectedTool]->axis.end(), 
            [&selectedMesh, &position, &projection, &view](const MeshPtr& mesh)
            {
                mesh->SetPosition(selectedMesh->Position());
                mesh->DrawMesh(position, projection, view);
            });
    }
}

void Manipulator::RenderSphere(LPD3DXEFFECT effect,
    const Matrix& projection, const Matrix& view,
    const D3DXVECTOR3& color, const Transform& world)
{
    D3DXMATRIX wvp = world.GetMatrix() * view.GetMatrix() * projection.GetMatrix();
    effect->SetMatrix(DxConstant::WordViewProjection, &wvp);
    effect->SetFloatArray(DxConstant::VertexColor, &color.x, 3);

    UINT nPasses = 0;
    effect->Begin(&nPasses, 0);
    for(UINT iPass = 0; iPass < nPasses; iPass++)
    {
        effect->BeginPass(iPass);
        m_sphere->DrawSubset(0);
        effect->EndPass();
    }
    effect->End();
}