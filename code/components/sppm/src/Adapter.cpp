#include "SPPMRenderer.hpp"

#include "component/RenderComponent.hpp"
#include "server/Server.hpp"

namespace sppm {
using namespace std;
using namespace NRenderer;

// 继承RenderComponent, 复写render接口
class Adapter : public RenderComponent {
  void render(SharedScene spScene) {
    auto width = spScene->renderOption.width;
    auto height = spScene->renderOption.height;
    auto iternum = spScene->renderOption.samplesPerPixel;
    auto mdepth = spScene->renderOption.depth;

    // 初始化渲染器
    size_t photonPIter =width * height;  // photon_per_iter
    auto renderer =
        SPPMRenderer(spScene, photonPIter, iternum, mdepth, 20, 0.003, 0.3);

    // render
    auto pixels = renderer.render();
    getServer().screen.set(pixels.data(), width, height);
  }
};
}  // namespace sppm

// REGISTER_RENDERER(Name, Description, Class)
REGISTER_RENDERER(SPPM,
                  "Stochastic Progressive Photon Mapping\n"
                  " - sppm.scn是漫反射场景\n"
                  " - test_material.scn包含金属与绝缘体\n"
                  " - 采样数，即迭代次数\n",
                  sppm::Adapter);