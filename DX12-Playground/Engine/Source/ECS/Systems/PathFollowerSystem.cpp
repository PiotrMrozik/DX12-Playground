#include <DX12LibPCH.h>
#include <ECS/Systems/PathFollowerSystem.h>

#include <DirectXMath.h>
#include <cmath>

using namespace DirectX;

void PathFollowerSystem::Update(World& world, float dt)
{
    for (Entity e : entities)
    {
        auto& pf = world.GetComponent<PathFollowerComponent>(e);
        if (!pf.path)
            continue;

        // Lazy arc-length cache.
        if (pf.arcLength < 0.f)
            pf.arcLength = pf.path->ArcLength();

        // Advance t based on world-space speed.
        if (pf.playing && pf.arcLength > 0.f)
        {
            const float step = (pf.speed / pf.arcLength) * dt;
            pf.t += step;

            if (pf.loop)
            {
                pf.t = std::fmod(pf.t, 1.f);
                if (pf.t < 0.f) pf.t += 1.f;
            }
            else
            {
                if (pf.t >= 1.f) { pf.t = 1.f; pf.playing = false; }
                if (pf.t <  0.f) { pf.t = 0.f; pf.playing = false; }
            }
        }

        // Write evaluated position.
        auto& tc  = world.GetComponent<TransformComponent>(e);
        tc.position = pf.path->Evaluate(pf.t);

        // Align yaw to tangent (rotation.y = atan2 in XZ-plane).
        if (pf.orientToPath)
        {
            XMFLOAT3 tangent = pf.path->Tangent(pf.t);
            tc.rotation.y = std::atan2f(tangent.x, tangent.z);
        }
    }
}
