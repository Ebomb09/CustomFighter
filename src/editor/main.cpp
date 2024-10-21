#include "editor.h"

#include "core/save.h"
#include "core/input_interpreter.h"
#include "core/video.h"
#include "core/math.h"

#include <vector>
#include <string>
#include <iostream>

#ifdef __WIN32__
    #include <windows.h>
#endif

using std::vector, std::string;

static bool inRange(int i, int a, int b) {
    return (i <= std::max(a, b) && i >= std::min(a, b));
}

int main() {

    #ifdef __WIN32__
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    #endif

    Editor editor;

    g::video.setTitle("Skeletal Animation Editor");
    g::video.setSize(g::save.resolution);
    g::video.setDisplayMode(g::save.displayMode);
    g::video.setVSync(g::save.vsync);
    g::video.reload();

    g::video.camera.x = g::video.getSize().x / -2.f;
    g::video.camera.y = g::video.getSize().y / 2.f;

    while (g::video.isOpen()) {
        g::input.pollEvents();

        g::video.clear();

        // Draw editor grid
        editor.update();
        
        if(g::input.held(MOUSE_INDEX, sf::Mouse::Button::Middle)) {
            g::video.camera.x += -g::input.mouseMove.x * g::video.getCameraScale().x;
            g::video.camera.y += g::input.mouseMove.y * g::video.getCameraScale().y;
        }

        if(g::input.mouseScroll < 0) {
            g::video.camera.w *= 2;
            g::video.camera.h *= 2;
        }

        if(g::input.mouseScroll > 0) {                     
            g::video.camera.w /= 2.;
            g::video.camera.h /= 2.;
        }

        ImGui::SetNextWindowPos({0, (float)g::video.getSize().y - 200});
        ImGui::SetNextWindowSize({(float)g::video.getSize().x, 200});
        ImGui::Begin("TimeLine");

        ImGui::BeginChild("KeyFrames", {0.0f, 96.f}, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_HorizontalScrollbar);
        for(int i = 0; i < editor.anim.getKeyFrameCount(); i ++) {

            if(i != 0)
                ImGui::SameLine();

            bool highlighted = editor.keyFrame == i || (editor.settings.mode == Editor::Mode::Frames) && editor.selected != -1 && inRange(i, editor.keyFrame, editor.selected);

            if(highlighted){
                ImGui::PushStyleColor(ImGuiCol_Button, ImColor(102, 255, 102).Value);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(204, 255, 204).Value);
                ImGui::PushStyleColor(ImGuiCol_Text, ImColor(0, 102, 0).Value);
            }

            bool clicked = ImGui::Button(std::to_string(i).c_str(), ImVec2(16 * editor.anim.getKeyFrame(i).duration, 64));

            if(highlighted) 
                ImGui::PopStyleColor(3);                  
            
            if(clicked) {

                if(ImGui::GetIO().KeyShift && editor.settings.mode == Editor::Mode::Frames) {
                    editor.selected = i;

                }else {
                    editor.setKeyFrame(i);
                }
            }
        }
        ImGui::EndChild();
  
        // Delete current key frame and pick the previous
        if(ImGui::Button("-Remove") && editor.keyFrame >= 0) {
            int count = 1;

            // Get the range of removal
            if(editor.settings.mode == Editor::Mode::Frames && editor.selected != -1) {
                count = 1 + std::max(editor.keyFrame, editor.selected) - std::min(editor.keyFrame, editor.selected);
                editor.keyFrame = std::min(editor.keyFrame, editor.selected);
                editor.selected = -1;
            }

            // The keyframes to the right
            for(int i = 0; i < count; i ++)
                editor.anim.removeKeyFrame(editor.keyFrame);
            
            editor.setKeyFrame(editor.keyFrame-1);
        }

        ImGui::SameLine();
        if(ImGui::Button("+Add")) {
            Frame src;

            // If selecting a frame copy it over
            if(editor.getKeyFrame())
                src = *editor.getKeyFrame();

            editor.anim.insertKeyFrame(editor.keyFrame + 1, src); 
            editor.keyFrame ++;         
        }

        if(!ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            editor.setKeyFrame(editor.keyFrame-1);

        if(!ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            editor.setKeyFrame(editor.keyFrame+1);

        if(editor.keyFrame >= 0) {
            
            if((ImGui::ArrowButton("LeftFrame", ImGuiDir_Left) && editor.keyFrame > 0)
            || (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_LeftArrow))) {
                editor.anim.swapKeyFrame(editor.keyFrame, editor.keyFrame-1);
                editor.setKeyFrame(editor.keyFrame-1);
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(64);
            ImGui::InputInt("##Duration", &editor.anim.getKeyFrame(editor.keyFrame).duration, 0);   

            ImGui::SameLine();
            if((ImGui::ArrowButton("RightFrame", ImGuiDir_Right) && editor.keyFrame < editor.anim.getKeyFrameCount()-1)
            || (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_RightArrow))) {
                editor.anim.swapKeyFrame(editor.keyFrame, editor.keyFrame+1);
                editor.setKeyFrame(editor.keyFrame+1);
            }                        
        }
        ImGui::End();

        if(ImGui::BeginMainMenuBar()) {

            if(ImGui::BeginMenu("File")) {

                if(editor.fileName != "") {
                    ImGui::MenuItem(editor.fileName.c_str(), NULL, false, false);
                    ImGui::Separator();
                }

                if(ImGui::MenuItem("Open")) {
                    nfdchar_t* outPath;
                    nfdfilteritem_t filters = {"editor.animation", "move"};

                    nfdresult_t result = NFD_OpenDialog(&outPath, &filters, 1, NULL);

                    if(result == NFD_OKAY) {
                        editor.fileName = outPath;
                        editor.anim.loadFromFile(outPath);
                        editor.setKeyFrame((editor.anim.getKeyFrameCount() == 0) ? -1 : 0);
                        NFD_FreePath(outPath);
                    }
                }

                if(ImGui::MenuItem("Import")) {
                    nfdchar_t* outPath;
                    nfdfilteritem_t filters = {"freemocap.json", "json"};

                    nfdresult_t result = NFD_OpenDialog(&outPath, &filters, 1, NULL);

                    if(result == NFD_OKAY) {
                        editor.fileName = "";
                        editor.anim.importFromFreeMoCap(outPath);
                        editor.setKeyFrame((editor.anim.getKeyFrameCount() == 0) ? -1 : 0);
                        NFD_FreePath(outPath);
                    }
                }

                if(ImGui::MenuItem("Save")) {

                    if(editor.fileName != "") {
                        editor.anim.saveToFile(editor.fileName);

                    }else {
                        nfdchar_t* outPath;
                        nfdfilteritem_t filters = {"editor.animation", "move"};
                        nfdchar_t defaultName[] = "*.move";

                        nfdresult_t result = NFD_SaveDialog(&outPath, &filters, 1, NULL, defaultName);

                        if(result == NFD_OKAY) {
                            editor.fileName = outPath;
                            editor.anim.saveToFile(outPath);
                            NFD_FreePath(outPath);
                        }                        
                    }
                }

                if(ImGui::MenuItem("Save As...")) {
                    nfdchar_t* outPath;
                    nfdfilteritem_t filters = {"editor.animation", "move"};
                    nfdchar_t defaultName[] = "*.move";

                    nfdresult_t result = NFD_SaveDialog(&outPath, &filters, 1, NULL, defaultName);

                    if(result == NFD_OKAY) {
                        editor.fileName = outPath;
                        editor.anim.saveToFile(outPath);
                        NFD_FreePath(outPath);
                    }                          
                }

                ImGui::Separator();

                if(ImGui::MenuItem("Exit")) {
                    g::video.close();
                }

                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Draw")) {
                ImGui::Checkbox("Grid", &editor.settings.drawGrid);
                ImGui::Checkbox("Skeleton", &editor.settings.drawSkeleton);        
                ImGui::Checkbox("Model", &editor.settings.drawModel);       
                ImGui::Checkbox("HitBox", &editor.settings.drawHitBox);
                ImGui::Checkbox("HurtBox", &editor.settings.drawHurtBox);   
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Camera")) {
                ImGui::InputFloat("X", &g::video.camera.x);
                ImGui::InputFloat("Y", &g::video.camera.y);
                ImGui::InputFloat("W", &g::video.camera.w);
                ImGui::InputFloat("H", &g::video.camera.h);     
                ImGui::EndMenu();                       
            }  

            if(ImGui::BeginMenu("Playback")) {

                if(ImGui::Checkbox("Play", &editor.settings.playback)) {
                    editor.resetPlayer();
                }
                ImGui::SliderInt("Playback Speed", &editor.settings.playbackSpeed, 1, 10, "Every %d frame(s)");
                ImGui::EndMenu();
            }

            // Editing mode
            vector<string> modes {"Frames", "Joints", "HitBoxes", "HurtBoxes"};

            if(ImGui::BeginCombo("##Editor Mode", modes[editor.settings.mode].c_str(), ImGuiComboFlags_WidthFitPreview)) {

                for(int i = 0; i < modes.size(); i ++) {

                    if(ImGui::Selectable(modes[i].c_str(), i == editor.settings.mode)) {
                        editor.settings.mode = i;
                        editor.selectDefault();
                    }
                }
                ImGui::EndCombo();
            }

            if(ImGui::BeginMenu("Stats")) {
                ImGui::Text("Frames: %d", editor.anim.getFrameCount());
                ImGui::Text("Startup: %d", editor.anim.getStartup());
                ImGui::Text("On Hit: %d", editor.anim.getOnHit());
                ImGui::Text("On Block: %d", editor.anim.getOnBlock());
                ImGui::Text("Damage: %d", editor.anim.getDamage());
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGui::SetNextWindowPos({(float)g::video.getSize().x - 300, 0});
        ImGui::SetNextWindowSize({300, (float)g::video.getSize().y - 200});
        ImGui::Begin("Properties");

        if(editor.getKeyFrame()) {  
            Frame& keyFrame = *editor.getKeyFrame();

            if(ImGui::CollapsingHeader("Animation Category...")) {

                if(ImGui::BeginCombo("Category", MoveCategory::String[editor.anim.category].c_str())) {

                    for(int i = 0; i < MoveCategory::Total; i ++) {

                        if(ImGui::Selectable(MoveCategory::String[i].c_str(), i == editor.anim.category))
                            editor.anim.category = i;
                    }

                    ImGui::EndCombo();
                }
            }

            if(ImGui::CollapsingHeader("Animation From...")) {

                for(int i = 0; i < MoveCategory::Total; i ++)
                    ImGui::Checkbox((MoveCategory::String[i] + "##FROM").c_str(), &editor.anim.from[i]);

                if(ImGui::BeginCombo("Custom From", editor.anim.customFrom.c_str())) {
                    vector<string> list = g::save.getAnimationsList();
                    list.insert(list.begin(), "##_NO_CUSTOM_CANCEL_");

                    for(auto& anim : list) {

                        if(ImGui::Selectable(anim.c_str(), editor.anim.customFrom == anim))
                            editor.anim.customFrom = anim;
                    }

                    if(editor.anim.customFrom == "##_NO_CUSTOM_CANCEL_")
                        editor.anim.customFrom = "";

                    ImGui::EndCombo();
                }
            }

            if(ImGui::CollapsingHeader("Skeleton Draw Order")) {
                
                // Skeleton reoderable draw part selection
                for(int i = 0; i < SkeletonDrawOrder::Total; i ++) {
                    ImGui::Text("%s", (std::to_string(i) + ".").c_str());
                    ImGui::SameLine();
                    ImGui::Selectable(SkeletonDrawOrder::String[keyFrame.pose.order[i]].c_str());

                    if(ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
                        ImVec2 drag = ImGui::GetMouseDragDelta();

                        if(drag.y < 0 && i > 0) {
                            std::swap(keyFrame.pose.order[i], keyFrame.pose.order[i-1]);

                        }else if(drag.y > 0 && i < SkeletonDrawOrder::Total - 1){
                            std::swap(keyFrame.pose.order[i], keyFrame.pose.order[i+1]);
                        }
                        ImGui::ResetMouseDragDelta();
                    }
                }
            }

            if(ImGui::CollapsingHeader("Key Frame")) {
                ImGui::InputFloat("Impulse X", &keyFrame.impulse.x);
                ImGui::InputFloat("Impulse Y", &keyFrame.impulse.y);
                ImGui::Checkbox("Cancel", &keyFrame.cancel);

                if(ImGui::BeginCombo("Sound Effect", keyFrame.sound.c_str())) {
                    vector<string> list = g::save.getSoundList();
                    list.insert(list.begin(), "##__NO_SOUND__");

                    for(auto& sound : list) {

                        if(ImGui::Selectable(sound.c_str(), keyFrame.sound == sound))
                            keyFrame.sound = sound;
                    }

                    if(keyFrame.sound == "##__NO_SOUND__")
                        keyFrame.sound = "";

                    ImGui::EndCombo();
                }

                ImGui::Checkbox("Grab", &keyFrame.isGrab);

                if(keyFrame.isGrab) {

                    if(ImGui::BeginCombo("Grab Break", GrabBreak::String[keyFrame.grabBreak].c_str())) {

                        for(int i = 0; i < GrabBreak::Total; i ++) {

                            if(ImGui::Selectable(GrabBreak::String[i].c_str(), i == keyFrame.grabBreak))
                                keyFrame.grabBreak = i;
                        }

                        ImGui::EndCombo();
                    }
                }           
            }

            if(ImGui::CollapsingHeader("Selection")) { 

                if(editor.settings.mode == Editor::Mode::Joints) {

                    if(ImGui::BeginListBox("Joints")){

                        for(int i = 0; i < editor.getJoints().size(); i ++) {

                            if(ImGui::Selectable(("joint [" + std::to_string(i) + "]").c_str(), i == editor.selected)) 
                                editor.setSelected(i);
                        }
                        ImGui::EndListBox();                        
                    }

                    if(editor.getJoint()) {
                        ImGui::SeparatorText("Attributes");
                        Vector2& joint = *editor.getJoints()[editor.selected];
                        ImGui::InputFloat("X", &editor.getJoint()->x);
                        ImGui::InputFloat("Y", &editor.getJoint()->y);                    
                    }   
                } 

                if(editor.settings.mode == Editor::Mode::HitBoxes && editor.getHitBoxes()) {
                    vector<HitBox>* hitBoxes = editor.getHitBoxes();

                    if(ImGui::BeginListBox("HitBoxes")){

                        for(int i = 0; i < hitBoxes->size(); i ++) {

                            if(ImGui::Selectable(("hitBox [" + std::to_string(i) + "]").c_str(), i == editor.selected)) 
                                editor.setSelected(i);
                        }
                        ImGui::EndListBox();
                    }

                    if(editor.getHitBox()) {
                        ImGui::SeparatorText("Attributes");
                        ImGui::InputFloat("X", &editor.getHitBox()->x);
                        ImGui::InputFloat("Y", &editor.getHitBox()->y);
                        ImGui::InputFloat("W", &editor.getHitBox()->w);
                        ImGui::InputFloat("H", &editor.getHitBox()->h);

                        if(ImGui::BeginCombo("Type", HitType::String[editor.getHitBox()->type].c_str())) {

                            for(int i = 0; i < HitType::Total; i ++) {

                                if(ImGui::Selectable(HitType::String[i].c_str(), i == editor.getHitBox()->type)) 
                                    editor.getHitBox()->type = i;
                            }
                            ImGui::EndCombo();
                        }

                        ImGui::InputInt("Damage", &editor.getHitBox()->damage);
                        ImGui::InputInt("Hit Stun", &editor.getHitBox()->hitStun);
                        ImGui::InputInt("Block Stun", &editor.getHitBox()->blockStun);
                        ImGui::Checkbox("Knockdown", &editor.getHitBox()->knockdown);
                        ImGui::InputFloat("Force X", &editor.getHitBox()->force.x);     
                        ImGui::InputFloat("Force Y", &editor.getHitBox()->force.y);                                      
                    }
                } 

                if(editor.settings.mode == Editor::Mode::HurtBoxes && editor.getHurtBoxes()) {
                    vector<HurtBox>* hurtBoxes = editor.getHurtBoxes();

                    if(ImGui::BeginListBox("Hurt Boxes")) {
                        for(int i = 0; i < hurtBoxes->size(); i ++) {

                            if(ImGui::Selectable(("hurtBox [" + std::to_string(i) + "]").c_str(), i == editor.selected)) 
                                editor.setSelected(i);
                        }
                        ImGui::EndListBox();
                    }

                    if(editor.getHurtBox()) {
                        ImGui::SeparatorText("Attributes");
                        ImGui::InputFloat("X", &editor.getHurtBox()->x);
                        ImGui::InputFloat("Y", &editor.getHurtBox()->y);
                        ImGui::InputFloat("W", &editor.getHurtBox()->w);
                        ImGui::InputFloat("H", &editor.getHurtBox()->h);
                        ImGui::InputInt("Armour", &editor.getHurtBox()->armour);
                    }
                } 
            }     
        }
        ImGui::End();

        g::video.display();
    }
    return 0;
}