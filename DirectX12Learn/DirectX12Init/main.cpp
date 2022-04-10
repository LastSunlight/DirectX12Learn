#pragma once

#include "GameCore.h"

using namespace GameCore;

class D3DInitApp final : public IGameApp
{

public:
    D3DInitApp() = default;

    virtual void Startup() override {}
    virtual void Cleanup() override {}

    virtual void Update(float deltaT) override {}
    virtual void RenderScene() override {}
};


CREATE_APPLICATION( D3DInitApp )