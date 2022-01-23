//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    float time = float(timer.GetTotalSeconds());

    m_world = Matrix::CreateRotationY(time);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_model->Draw(context, *m_states, m_world, m_view, m_proj);

    m_deviceResources->PIXEndEvent();

    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::LightBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{

}

void Game::OnDeactivated()
{
    
}

void Game::OnSuspending()
{
   
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 1000;
    height = 800;
}
#pragma endregion

#pragma region Direct3D Resources

void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);

    m_fxFactory = std::make_unique<EffectFactory>(device);

    m_model = Model::CreateFromSDKMESH(device, L"model.sdkmesh", *m_fxFactory, ModelLoader_CounterClockwise);

    m_world = Matrix::Identity;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    m_view = Matrix::CreateLookAt(Vector3(100.0f, 0.0f, 0.0f), Vector3::Zero, Vector3::UnitY);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 6.f,
        float(size.right) / float(size.bottom), 0.1f, 600.0f);

    // Backflip car
    /*m_view = Matrix::CreateLookAt(Vector3(100.0f, 0.0f, 0.0f), Vector3::Zero, Vector3::Down);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 6.f,
        float(size.right) / float(size.bottom), 0.1f, 600.0f);*/
}

void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_model.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
