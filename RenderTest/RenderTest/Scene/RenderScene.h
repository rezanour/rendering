#pragma once

class RenderVisual;
struct RenderView;

// TODO: create a base class for this and try different approaches (ex: Bvh)
class RenderScene : NonCopyable
{
public:
    RenderScene();
    virtual ~RenderScene();

    void AddVisual(const std::shared_ptr<RenderVisual>& visual);
    void RemoveVisual(const std::shared_ptr<RenderVisual>& visual);

    // Fills in visuals list. visualCount is equal to max size of visuals on way in, and set to 
    // number of actual visuals filled in on return. Function returns false if couldn't fit all visuals
    void GetVisibleVisuals(const RenderView& view, std::vector<std::shared_ptr<RenderVisual>>* visuals);

private:
    std::vector<std::shared_ptr<RenderVisual>> AllVisuals;
};
