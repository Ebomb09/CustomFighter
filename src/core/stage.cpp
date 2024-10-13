#include "stage.h"
#include "save.h"

#include <algorithm>
#include <fstream>
#include <json.hpp>

void Stage::draw(Renderer* renderer) {

    if(!renderer)
        renderer = &g::video;

    // Distance from left to right for stage bounds
    float percent = (renderer->camera.x - StageBounds.x) / (StageBounds.w - renderer->camera.w);

    for(auto& layer : layers) {

        if(!layer.tex)
            continue;

        // Real size of layer
        float width = (StageBounds.w * layer.scale);
        
        // Create scrolling rectangle
        Rectangle bounds = {
            StageBounds.x + (StageBounds.w - width) * percent,
            StageBounds.y,
            width,
            StageBounds.h
        };

        // Expand to fit a larger texture
        if(layer.tex->getSize().x > bounds.w) {
            float diff = layer.tex->getSize().x - bounds.w;
            bounds.x -= diff / 2.f;
            bounds.w += diff;
        }

        sf::RectangleShape sh = renderer->toScreen(bounds);
        sh.setTexture(layer.tex);

        renderer->draw(sh);
    }
}

/*
void drawStage(int index) {
	sf::Texture* tex = g::save.getStage(index);

	sf::Vertex v[4] = {
		sf::Vertex(Vector2(StageBounds.x, StageBounds.y), 									Vector2(0, 0)),
		sf::Vertex(Vector2(StageBounds.x + StageBounds.w, StageBounds.y), 					Vector2(tex->getSize().x, 0)),
		sf::Vertex(Vector2(StageBounds.x + StageBounds.w, StageBounds.y - StageBounds.h),	Vector2(tex->getSize().x, tex->getSize().y)),	
		sf::Vertex(Vector2(StageBounds.x, StageBounds.y - StageBounds.h), 					Vector2(0, tex->getSize().y)),
	};

	for(int i = 0; i < 4; i ++) 
		v[i].position = g::video.toScreen(v[i].position);

	sf::RenderStates states;
	states.texture = tex;

	g::video.draw(v, 4, sf::PrimitiveType::Quads, states);
}
*/

static bool compLayer(const Stage::Layer& l1, const Stage::Layer& l2) {
    return l1.scale < l2.scale;
}

bool Stage::loadFromFile(std::filesystem::path path) {

    std::fstream file(path/"stage.json", std::fstream::in);

    if(file.good()) {
        nlohmann::json def;

        try {
            def = nlohmann::json::parse(file);
            file.close();

        }catch(nlohmann::json::exception) {
            file.close();
            return false;
        }

        for(auto& layer : def["layers"]) {

            if(layer["scale"].is_number() && layer["texture"].is_string()){
                Stage::Layer add;
                add.scale = layer["scale"];
                add.tex = g::save.getTexture(path/layer["texture"]);
                layers.push_back(add);
            }
        }

        // Sort by scale
        std::sort(layers.begin(), layers.end(), compLayer);
        return true;
    }

    file.close();
    return false;
}