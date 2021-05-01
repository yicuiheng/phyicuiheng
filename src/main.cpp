#include <iostream>
#include <memory>

#include "model.hpp"
#include "window.hpp"

int main() {
    auto window = std::make_unique<window_t>();
    rigid_body_t rigid_body;
    // auto [cloth, rigid_body] = model_t::make_cloth();
    auto cloth = model_t::make_cloth(rigid_body);

    while (true) {
        // update models

        /*
        given: 各質点の位置 p_i, 速度 v_i, 質量の逆数 w
               制約 (constraint) C(p_i, .., p_j) = 0 or >= 0 とか, stiffiness \in [0 .. 1] 重み付け
        
        algorithm: 各時間ステップで以下をする

        for i番目の質点 {
            v_i を更新 (外部の力の影響を反映 e.g. 重力)
            predic_p_i = p_i + v_i * dt
        }

        収束するまで繰り返す:
            各 predic_p_i が制約を満たすまで修正する
        
        for i番目の質点 {
            p_i = predic_p_i
        }
        */

        rigid_body.update(0.1f);

        window->update();
        window->draw(cloth);

        if (window->shouldClose())
            break;
    }
    return 0;
}