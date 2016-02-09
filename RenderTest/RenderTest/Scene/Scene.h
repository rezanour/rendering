#pragma once

struct RenderView;
class Visual;
class Light;

// TODO: create a base class for this and try different approaches (ex: Bvh)
class Scene : NonCopyable
{
public:
    Scene();
    virtual ~Scene();

    void AddVisual(const std::shared_ptr<Visual>& visual);
    void RemoveVisual(const std::shared_ptr<Visual>& visual);

    void AddLight(const std::shared_ptr<Light>& light);
    void RemoveLight(const std::shared_ptr<Light>& light);

    // Query all visuals in the scene visible to 'view'
    void GetVisibleVisuals(const RenderView& view, std::vector<std::shared_ptr<Visual>>* visuals);

    // Query all lights in the scene that overlap 'view'
    void GetVisibleLights(const RenderView& view, std::vector<std::shared_ptr<Light>>* lights);

private:
    std::vector<std::shared_ptr<Light>> AllLights;
    std::vector<std::shared_ptr<Visual>> AllVisuals;
};
