#include "Precomp.h"
#include "RenderScene.h"
#include "RenderVisual.h"
#include "CoreGraphics/RenderingCommon.h"

RenderScene::RenderScene()
{
}

RenderScene::~RenderScene()
{
}

void RenderScene::AddVisual(const std::shared_ptr<RenderVisual>& visual)
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

void RenderScene::RemoveVisual(const std::shared_ptr<RenderVisual>& visual)
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

void RenderScene::GetVisibleVisuals(const RenderView& view, std::vector<std::shared_ptr<RenderVisual>>* visuals)
{
    UNREFERENCED_PARAMETER(view);

    visuals->clear();

    // There is no culling in this render scene impl
    visuals->resize(AllVisuals.size());
    std::copy(AllVisuals.begin(), AllVisuals.end(), visuals->begin());
}
