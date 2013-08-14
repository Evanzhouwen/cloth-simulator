////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - scene.h
////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "common.h"
#include <queue>
#include "GUICallbacks.h"

using namespace GUI;
class Shader;
class ClothSolver;
class Mesh;
class Picking;
class Manipulator;
struct RenderCallbacks;

/**
* Factory/Manager that creates and renders all objects in the scene
*/
class Scene
{
public:

    /**
    * Available objects to create
    */
    enum Object
    {
        BOX,
        SPHERE,
        CYLINDER,
        MAX_OBJECT
    };

    /**
    * Constructor
    * @param d3ddev the directx device
    * @param callbacks The callbacks for rendering a mesh
    */
    Scene(LPDIRECT3DDEVICE9 d3ddev, const RenderCallbacks& callbacks);

    /**
    * Draws all scene meshes
    * @param position The camera position
    * @param projection The camera projection matrix
    * @param view The camera view matrix
    */
    void Draw(const D3DXVECTOR3& position, const Transform& projection, const Transform& view);

    /**
    * Draws the scene manipulator tool
    * @param position The camera position
    * @param projection The camera projection matrix
    * @param view The camera view matrix
    */
    void DrawTools(const D3DXVECTOR3& position, const Transform& projection, const Transform& view);

    /**
    * Draws all scene mesh collisions
    * @param projection The camera projection matrix
    * @param view The camera view matrix
    */
    void DrawCollision(const Transform& projection, const Transform& view);

    /**
    * Removes the currently selected object if possible
    */
    void RemoveObject();

    /**
    * Removes all objects in the scene
    */
    void RemoveScene();

    /**
    * Adds an object to the scene
    * @param object the object to add
    */
    void AddObject(Object object);

    /**
    * Tests all scene objects for mouse picking
    * @param input the mouse picking object
    */
    void MousePickingTest(Picking& input);

    /**
    * Solves the collision between scene objects and the cloth
    * @param solver the cloth solver for object-cloth collisions
    */
    void SolveClothCollision(ClothSolver& solver);

    /**
    * Set the visibility of the collision mesh
    * @param visible whether the collision mesh if visible or not
    */
    void SetCollisionVisibility(bool visible);

    /**
    * Loads the gui callbacks
    * @param callbacks callbacks for the gui to fill in
    */
    void LoadGuiCallbacks(GuiCallbacks* callbacks);

private:

    /**
    * Sets the selected mesh
    * @param mesh the selected mesh
    */
    void SetSelectedMesh(const Mesh* mesh);

    /**
    * Prevent copying
    */
    Scene(const Scene&);
    Scene& operator=(const Scene&);

    typedef std::shared_ptr<Mesh> MeshPtr;

    LPDIRECT3DDEVICE9 m_d3ddev;                  ///< DirectX device
    std::queue<unsigned int> m_open;             ///< Indices for the avaliable meshes
    std::vector<MeshPtr> m_meshes;               ///< Changable meshes in the scene
    std::vector<MeshPtr> m_templates;            ///< Mesh templates for creating mesh instances
    std::shared_ptr<Manipulator> m_manipulator;  ///< manipulator tool for changing objects
    int m_selectedMesh;                          ///< Currently selected object
    SetFlag m_enableCreation;                    ///< Callback for enabled/disabling gui mesh creation
};
