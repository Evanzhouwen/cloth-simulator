////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - manipulator.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "manipulator.h"
#include "mesh.h"
#include "shader.h"
#include "diagnostic.h"

namespace
{
    const std::string MODEL_FOLDER(".\\Resources\\Models\\");
};

Manipulator::Manipulator(LPDIRECT3DDEVICE9 d3ddev, const RenderCallbacks& callbacks) :
    m_selectedTool(NONE)
{
    m_tools.resize(MAX_TOOLS);
    m_tools[MOVE] = std::shared_ptr<Tool>(new Tool(d3ddev, "move", MOVE, callbacks));
    //m_tools[ROTATE] = std::shared_ptr<Tool>(new Tool(d3ddev, "rotate", ROTATE, toolshader));
    //m_tools[SCALE] = std::shared_ptr<Tool>(new Tool(d3ddev, "scale", SCALE, toolshader));
}

Manipulator::Tool::Tool(LPDIRECT3DDEVICE9 d3ddev, const std::string& name, 
    int index, const RenderCallbacks& callbacks) :
        selectedAxis(NO_AXIS)
{
    auto toolshader = callbacks.getShader(ShaderManager::MAIN_SHADER);

    axis.resize(MAX_AXIS);
    std::generate(axis.begin(), axis.end(), 
        [&](){ return Manipulator::MeshPtr(new Mesh(callbacks)); });

    axis[X_AXIS]->Load(d3ddev, MODEL_FOLDER + name + "X.obj", toolshader, X_AXIS);
    axis[Y_AXIS]->Load(d3ddev, MODEL_FOLDER + name + "Y.obj", toolshader, Y_AXIS);
    axis[Z_AXIS]->Load(d3ddev, MODEL_FOLDER + name + "Z.obj", toolshader, Z_AXIS);
}

void Manipulator::ChangeTool(ToolType type)
{
    //toggle off tool if selecting twice
    m_selectedTool = (m_selectedTool == type ? NONE : type);
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

void Manipulator::Update(Picking& input, MeshPtr selectedMesh)
{
    if(m_selectedTool != NONE)
    {




    }
}

void Manipulator::Render(const Transform& projection, const Transform& view,
        const D3DXVECTOR3& position, const Manipulator::MeshPtr& selectedMesh)
{
    if(Diagnostic::AllowText())
    {
        Diagnostic::UpdateText("SelectedTool", Diagnostic::WHITE,
            GetDescription(m_selectedTool));
    }

    if(m_selectedTool != NONE)
    {
        std::for_each(m_tools[m_selectedTool]->axis.begin(), 
            m_tools[m_selectedTool]->axis.end(), [&](const MeshPtr& mesh)
        {
            mesh->SetPosition(selectedMesh->Position());
            mesh->DrawMesh(position, projection, view);
        });
    }
}