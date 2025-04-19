#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <CameraController.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
#include <Collision.h>
#include <ModelManager.h>
#include <TextureManager.h>

class GameApp : public D3DApp
{
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

private:
    bool InitResource();
    
private:

    TextureManager m_TextureManager;
    ModelManager m_ModelManager;
    Transform m_Transform = {};								// 物体变换信息

    BasicEffect m_BasicEffect;		            			    // 对象渲染特效管理
    SkyboxEffect m_SkyboxEffect;							    // 天空盒特效管理

    std::unique_ptr<Depth2D> m_pDepthTexture;                   // 深度缓冲区

    GameObject m_Ground;										// 地面
    GameObject m_Skybox;                                        // 天空盒
    GameObject m_Box;
    GameObject m_NewBox;
    GameObject m_Box1;
    GameObject m_Box2;
    GameObject m_Box3;
    GameObject m_Box4;
    GameObject m_Box5;
    GameObject m_Box6;
    GameObject m_Box7;
    GameObject m_Box8;
    GameObject m_Tree;
    GameObject m_Tree1;
    GameObject m_Tree2;
    GameObject m_Leaf;
    GameObject m_Leaf1;
    GameObject m_Leaf2;
    GameObject m_Leaf3;
    GameObject m_Leaf4;
    GameObject m_Leaf5;
    GameObject m_Leaf6;
    GameObject m_Leaf7;
    GameObject m_Leaf8;
    GameObject m_Leaf9;
    GameObject m_Leaf10;
    GameObject m_Leaf11;
    GameObject m_Leaf12;
    ComPtr<ID3D11ShaderResourceView> m_pTreeCrate;			    // 木头纹理
    ComPtr<ID3D11ShaderResourceView> m_Clouds;
    DirectX::BoundingBox m_BoundingBox;				    // 箱子的包围盒
    DirectX::BoundingBox m_BoundingLeaf;
    DirectX::BoundingBox m_BoundingLeaf2;
    DirectX::BoundingBox m_BoundingFirstPersonCamera;				    // 摄像机的包围盒

    std::shared_ptr<FirstPersonCamera> m_pCamera;			    // 摄像机
    FirstPersonCameraController m_CameraController;             // 摄像机控制器 
};


#endif