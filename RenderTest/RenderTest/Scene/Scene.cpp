#include "Precomp.h"
#include "Scene.h"
#include "Visual.h"
#include "Light.h"
#include "CoreGraphics/RenderingCommon.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::AddVisual(const std::shared_ptr<Visual>& visual)
{
#ifdef _DEBUG
    for (auto& vis : AllVisuals)
    {
        if (vis == visual)
        {
            assert(false);
            return;
        }
    }
#endif
    AllVisuals.push_back(visual);
}

void Scene::RemoveVisual(const std::shared_ptr<Visual>& visual)
{
    for (auto it = AllVisuals.begin(); it != AllVisuals.end(); ++it)
    {
        if (*it == visual)
        {
            AllVisuals.erase(it);
            return;
        }
    }
}

void Scene::AddLight(const std::shared_ptr<Light>& light)
{
#ifdef _DEBUG
    for (auto& l : AllLights)
    {
        if (l == light)
        {
            assert(false);
            return;
        }
    }
#endif
    AllLights.push_back(light);
}

void Scene::RemoveLight(const std::shared_ptr<Light>& light)
{
    for (auto it = AllLights.begin(); it != AllLights.end(); ++it)
    {
        if (*it == light)
        {
            AllLights.erase(it);
            return;
        }
    }
}

void Scene::GetVisibleVisuals(const RenderView& view, std::vector<std::shared_ptr<Visual>>* visuals)
{
    UNREFERENCED_PARAMETER(view);

    visuals->clear();

    // There is no culling in this render scene impl
    visuals->resize(AllVisuals.size());
    std::copy(AllVisuals.begin(), AllVisuals.end(), visuals->begin());
}

void Scene::GetVisibleLights(const RenderView& view, std::vector<std::shared_ptr<Light>>* lights)
{
    UNREFERENCED_PARAMETER(view);

    lights->clear();

    // There is no culling in this render scene impl
    lights ->resize(AllLights.size());
    std::copy(AllLights.begin(), AllLights.end(), lights->begin());
}
