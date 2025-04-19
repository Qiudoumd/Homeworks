#include "GameApp.h"
#include <XUtil.h>
#include <DXTrace.h>
using namespace DirectX;

int a = 0;
void Test(int x)
{
    a = x;
}



GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    m_TextureManager.Init(m_pd3dDevice.Get());
    m_ModelManager.Init(m_pd3dDevice.Get());

    // 务必先初始化所有渲染状态，以供下面的特效使用
    
    RenderStates::InitAll(m_pd3dDevice.Get());

    if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!m_SkyboxEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{

    D3DApp::OnResize();
    
    m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
    m_pDepthTexture->SetDebugObjectName("DepthTexture");

    // 摄像机变更显示
    if (m_pCamera != nullptr)
    {
        m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
        m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
        m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
        m_SkyboxEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
    }
}

void GameApp::UpdateScene(float dt)
{
    m_CameraController.Update(dt);

    // 拾取检测
    //
    ImVec2 mousePos = ImGui::GetMousePos();
    mousePos.x = std::clamp(mousePos.x, 0.0f, m_ClientWidth - 1.0f);
    mousePos.y = std::clamp(mousePos.y, 0.0f, m_ClientHeight - 1.0f);
    Ray ray = Ray::ScreenToRay(*m_pCamera, mousePos.x, mousePos.y);
    
    bool hitObject = false;
    std::string pickedObjStr = "None";
    int recognition = 0;//规定识别物体
    if (ray.Hit(m_BoundingBox))
    {
        pickedObjStr = "Box";
        hitObject = true;
        recognition = 1;
    }
    else if (ray.Hit(m_BoundingLeaf))
    {
        pickedObjStr = "Leaf";
        hitObject = true;
        recognition = 2;
    }
    if (hitObject == true && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        std::wstring wstr = L"You destroy ";
        wstr += UTF8ToWString(pickedObjStr) + L"!";
        MessageBox(nullptr, wstr.c_str(), L"Message", 0);
    }

    XMFLOAT3 center = m_pCamera->GetPosition();
    XMFLOAT3 extents(1.0f, 1.0f, 1.0f);
    m_BoundingFirstPersonCamera = DirectX::BoundingBox(center, extents);
    bool isIntersecting = m_BoundingBox.Intersects(m_BoundingFirstPersonCamera);

     //左键进行破坏操作
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        if (ray.Hit(m_BoundingBox))
        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box", Geometry::CreateBox(0.2f, 0.2f, 0.2f));
            pModel->SetDebugObjectName("Box");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box.SetModel(pModel);
            m_Box.GetTransform().SetPosition(0.0f,0.0f,0.0f);
            if (a == 1 || isIntersecting)
            {
               m_Box.GetTransform().SetPosition(10000.0f, 10000, 10000.0f);
            }
        }
        else if (ray.Hit(m_BoundingLeaf))
        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf", Geometry::CreateBox(0.2f, 0.2f, 0.2f));
            pModel->SetDebugObjectName("Leaf");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            bool test = m_BoundingLeaf.Intersects(m_BoundingBox);
            const int MAX_ITERATIONS = 4;
            int i = 1;
            int iterationCount = 0;
            while (!test && iterationCount < MAX_ITERATIONS)//重力检测
            {
                m_Leaf.GetTransform().SetPosition(0.0f, 3.0f-i, 0.0f);
                XMFLOAT3 centerLeaf2(0.0f, 3.0f-i, 0.0f);
                XMFLOAT3 extentsLeaf2(1.0f, 1.0f-i, 1.0f);
                m_BoundingLeaf2 = DirectX::BoundingBox(centerLeaf2, extentsLeaf2);
                i++;
                test = m_BoundingLeaf2.Intersects(m_BoundingBox);
                iterationCount++;
            }

        }
    }

    //右键放置方块
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        if (recognition == 1)
        {
            Model* pModel = m_ModelManager.CreateFromGeometry("NewBox", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("NewBox");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_NewBox.SetModel(pModel);
            m_NewBox.GetTransform().SetPosition(2.0, 2.0, 2.0f);
            Test(recognition);
        }
    }


    // 更新观察矩阵
    m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_BasicEffect.SetEyePos(m_pCamera->GetPosition());

    m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());

    if (ImGui::Begin("Static Cube Mapping"))
    {
        static int skybox_item = 0;
        static const char* skybox_strs[] = {
            "Daylight",
            "Sunset",
            "Desert"
        };
        if (ImGui::Combo("Skybox", &skybox_item, skybox_strs, ARRAYSIZE(skybox_strs)))
        {
            Model* pModel = m_ModelManager.GetModel("Skybox");
            switch (skybox_item)
            {
            case 0: 
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Daylight"));
                pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
                break;
            case 1: 
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Sunset"));
                pModel->materials[0].Set<std::string>("$Skybox", "Sunset");
                break;
            case 2: 
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Desert")); 
                pModel->materials[0].Set<std::string>("$Skybox", "Desert");
                break;
            }
        }
    }
    ImGui::End();
    ImGui::Render();
}

void GameApp::DrawScene()
{
    // 创建后备缓冲区的渲染目标视图
    if (m_FrameCount < m_BackBufferCount)
    {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
        CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
    }

    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), black);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ID3D11RenderTargetView* pRTVs[1] = { GetBackBufferRTV() };
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
    D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
    m_pd3dImmediateContext->RSSetViewports(1, &viewport);

    // 绘制模型
    m_BasicEffect.SetRenderDefault();
    m_BasicEffect.SetReflectionEnabled(true);
    //m_Sphere.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    m_BasicEffect.SetReflectionEnabled(false);
    //m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
   //m_Cylinder.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    //绘制地面

    
    {
        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Box.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Box1.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Box2.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Box3.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Box4.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Box5.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Box6.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Box7.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Box8.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    }

    //树木
    {
        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Tree.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Tree1.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Tree2.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
  
    }

    //树叶
    {
        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf1.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf2.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf3.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf4.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf5.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf6.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf7.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf8.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf9.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf10.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf11.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
        m_Leaf12.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    }


    {
        if (a == 1)
        {
            m_BasicEffect.SetReflectionEnabled(false); // 可根据需求设置反射效果
            m_NewBox.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

        }
    }



    // 绘制天空盒
    m_SkyboxEffect.SetRenderDefault();
    m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);


    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

bool GameApp::InitResource()
{
    // ******************
    // 初始化天空盒相关

    m_TextureManager.CreateFromFile("..\\Texture\\grass.dds");
    m_TextureManager.CreateFromFile("..\\Texture\\leaf.dds");
    m_TextureManager.CreateFromFile("..\\Texture\\soil.dds");
    m_TextureManager.CreateFromFile("..\\Texture\\tree.dds");
    m_TextureManager.CreateFromFile("..\\Texture\\rockblock.dds");
    
    ComPtr<ID3D11Texture2D> pTex;
    D3D11_TEXTURE2D_DESC texDesc;
    std::string filenameStr;
    std::vector<ID3D11ShaderResourceView*> pCubeTextures;
    std::unique_ptr<TextureCube> pTexCube;
    // Daylight
    {
        m_TextureManager.AddTexture("Daylight", m_TextureManager.CreateFromFile("..\\Texture\\grasscube1024.dds", false, true));
    }
    
    // Sunset
    {
        filenameStr = "..\\Texture\\sunset0.bmp";
        pCubeTextures.clear();
        for (size_t i = 0; i < 6; ++i)
        {
            filenameStr[17] = '0' + (char)i;
            pCubeTextures.push_back(m_TextureManager.CreateFromFile(filenameStr));
        }
        pCubeTextures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
        pTex->GetDesc(&texDesc);
        pTexCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        pTexCube->SetDebugObjectName("Sunset");
        for (uint32_t i = 0; i < 6; ++i)
        {
            pCubeTextures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
            m_pd3dImmediateContext->CopySubresourceRegion(pTexCube->GetTexture(), D3D11CalcSubresource(0, i, 1), 0, 0, 0, pTex.Get(), 0, nullptr);
        }
        m_TextureManager.AddTexture("Sunset", pTexCube->GetShaderResource());
    }
    
    // Desert
    m_TextureManager.AddTexture("Desert", m_TextureManager.CreateFromFile("..\\Texture\\desertcube1024.dds", false, true));
    //m_TextureManager.AddTexture("Desert", m_TextureManager.CreateFromFile("..\\Texture\\Clouds.dds", false, true));

    m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Daylight"));
    
    // ******************
    // 初始化游戏对象 
    // 地面
    {
        Model* pModel = m_ModelManager.CreateFromGeometry("Ground", Geometry::CreatePlane(XMFLOAT2(100.0f, 100.0f), XMFLOAT2(10.0f, 10.0f)));
        pModel->SetDebugObjectName("Ground");
        m_TextureManager.CreateFromFile("..\\Texture\\floor.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\floor.dds");
        pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
        m_Ground.SetModel(pModel);
        m_Ground.GetTransform().SetPosition(0.0f, -3.0f, 0.0f);

    }
    //泥土地面
    {
        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Box");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box.SetModel(pModel);
            m_Box.GetTransform().SetPosition(0.0f, 0.0f, 0.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box1", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Box1");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box1.SetModel(pModel);
            m_Box1.GetTransform().SetPosition(1.0f, 0.0f, 0.0f);
        }


        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box2", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Box2");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box2.SetModel(pModel);
            m_Box2.GetTransform().SetPosition(2.0f, 0.0f, 0.0f);
        }


        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box3", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Box3");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box3.SetModel(pModel);
            m_Box3.GetTransform().SetPosition(0.0f, 0.0f, 1.0f);
        }


        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box4", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Box4");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box4.SetModel(pModel);
            m_Box4.GetTransform().SetPosition(0.0f, 0.0f, 2.0f);
        }


        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box5", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Box5");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box5.SetModel(pModel);
            m_Box5.GetTransform().SetPosition(1.0f, 0.0f, 1.0f);
        }


        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box6", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Box6");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box6.SetModel(pModel);
            m_Box6.GetTransform().SetPosition(2.0f, 0.0f, 1.0f);
        }


        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box7", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Box7");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box7.SetModel(pModel);
            m_Box7.GetTransform().SetPosition(1.0f, 0.0f, 2.0f);
        }


        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Box8", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Box8");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\soil.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Box8.SetModel(pModel);
            m_Box8.GetTransform().SetPosition(2.0f, 0.0f, 2.0f);
        }
    }

    //树木
    {
        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Tree", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Tree");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\tree.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Tree.SetModel(pModel);
            m_Tree.GetTransform().SetPosition(1.0f, 1.0f, 1.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Tree1", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Tree1");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\tree.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Tree1.SetModel(pModel);
            m_Tree1.GetTransform().SetPosition(1.0f, 2.0f, 1.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Tree2", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Tree2");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\tree.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Tree2.SetModel(pModel);
            m_Tree2.GetTransform().SetPosition(1.0f, 3.0f, 1.0f);
        }

    }
    
    //树叶
    {
        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf.SetModel(pModel);
            m_Leaf.GetTransform().SetPosition(0.0f, 3.0f, 0.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf1", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf1");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf1.SetModel(pModel);
            m_Leaf1.GetTransform().SetPosition(1.0f, 3.0f, 0.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf2", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf2");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf2.SetModel(pModel);
            m_Leaf2.GetTransform().SetPosition(2.0f, 3.0f, 0.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf3", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf3");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf3.SetModel(pModel);
            m_Leaf3.GetTransform().SetPosition(0.0f, 3.0f, 1.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf4", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf4");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf4.SetModel(pModel);
            m_Leaf4.GetTransform().SetPosition(0.0f, 3.0f, 2.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf5", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf5");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf5.SetModel(pModel);
            m_Leaf5.GetTransform().SetPosition(2.0f, 3.0f, 1.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf6", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf6");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf6.SetModel(pModel);
            m_Leaf6.GetTransform().SetPosition(2.0f, 3.0f, 2.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf7", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf7");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf7.SetModel(pModel);
            m_Leaf7.GetTransform().SetPosition(1.0f, 3.0f, 2.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf8", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf8");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf8.SetModel(pModel);
            m_Leaf8.GetTransform().SetPosition(0.0f, 4.0f, 1.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf9", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf9");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf9.SetModel(pModel);
            m_Leaf9.GetTransform().SetPosition(1.0f, 4.0f, 1.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf10", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf10");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf10.SetModel(pModel);
            m_Leaf10.GetTransform().SetPosition(1.0f, 4.0f, 2.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf11", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf11");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf11.SetModel(pModel);
            m_Leaf11.GetTransform().SetPosition(1.0f, 4.0f, 0.0f);
        }

        {
            Model* pModel = m_ModelManager.CreateFromGeometry("Leaf12", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
            pModel->SetDebugObjectName("Leaf12");
            pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\leaf.dds");
            pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
            pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_Leaf12.SetModel(pModel);
            m_Leaf12.GetTransform().SetPosition(2.0f, 4.0f, 1.0f);
        }
    }


    // 天空盒立方体
    Model* pModel = m_ModelManager.CreateFromGeometry("Skybox", Geometry::CreateBox());
    pModel->SetDebugObjectName("Skybox");
    pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
    m_Skybox.SetModel(pModel);
    // ******************
    // 初始化摄像机
    //
    auto camera = std::make_shared<FirstPersonCamera>();
    m_pCamera = camera;
    m_CameraController.InitCamera(camera.get());
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
    camera->LookTo(XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));


    m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());
    m_SkyboxEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_SkyboxEffect.SetProjMatrix(camera->GetProjMatrixXM());

    
    //初始化Leaf碰撞盒
    XMFLOAT3 centerLeaf(0.0f, 3.0f, 0.0f);
    XMFLOAT3 extentsLeaf(1.0f, 1.0f, 1.0f);
    m_BoundingLeaf = DirectX::BoundingBox(centerLeaf, extentsLeaf);














    // ******************
    // 初始化不会变化的值
    //

    // 方向光
    DirectionalLight dirLight[4]{};
    dirLight[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
    dirLight[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    dirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
    dirLight[1] = dirLight[0];
    dirLight[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
    dirLight[2] = dirLight[0];
    dirLight[2].direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
    dirLight[3] = dirLight[0];
    dirLight[3].direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);
    for (int i = 0; i < 4; ++i)
        m_BasicEffect.SetDirLight(i, dirLight[i]);

    return true;
}

