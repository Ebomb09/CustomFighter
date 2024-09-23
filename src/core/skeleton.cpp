#include "skeleton.h"
#include "save.h"
#include "video.h"

using std::vector;

Skeleton::Skeleton() :
	// Unique bones
	neck		{torso[0], head},
	spine		{torso[0], torso[1]},

	// Left & Right equivalent bones
	collar		{{torso[0], shoulder[0]}, 	{torso[0], shoulder[1]}},
	upperarm	{{shoulder[0], elbow[0]}, 	{shoulder[1], elbow[1]}},
	forearm		{{elbow[0], wrist[0]}, 		{elbow[1], wrist[1]}},
	pelvis		{{torso[1], hip[0]}, 		{torso[1], hip[1]}},
	thigh		{{hip[0], knee[0]}, 		{hip[1], knee[1]}},
	calf		{{knee[0], heel[0]}, 		{knee[1], heel[1]}},
    hand        {{wrist[0], fingers[0]},    {wrist[1], fingers[1]}},
    foot        {{heel[0], toes[0]},        {heel[1], toes[1]}}
{
	setDefault();
}

Skeleton::Skeleton(const Skeleton& copy) : Skeleton() {
	*this = copy;
}

Skeleton::Skeleton(Skeleton&& move) : Skeleton() {
	*this = move;
}

Skeleton& Skeleton::operator=(const Skeleton& copy) {

	for(int i = 0; i < jointCount; i ++)
		joints[i] = copy.joints[i];	

    for(int i = 0; i < SkeletonDrawOrder::Total; i ++) 
        order[i] = copy.order[i];

	return *this;
}

Skeleton& Skeleton::operator=(Skeleton&& move) {

	for(int i = 0; i < jointCount; i ++)
		joints[i] = std::move(move.joints[i]);

    for(int i = 0; i < SkeletonDrawOrder::Total; i ++) 
        order[i] = std::move(move.order[i]);

	return *this;
}

void Skeleton::setDefault() {
	heel[0] = {15, 0};
	heel[1] = {-15, 0};
    toes[0] = {20, 0};
    toes[1] = {-20, 0};
	knee[0] = {17, 15};
	knee[1] = {-17, 15};
	hip[0] = {15, 30};
	hip[1] = {-15, 30};
	torso[1] = {0, 30};	
	torso[0] = {0, 60};	
	shoulder[0] = {10, 60};	
	shoulder[1] = {-10, 60};
	elbow[0] = {20, 55};
	elbow[1] = {-20, 55};
	wrist[0] = {8, 45};	
	wrist[1] = {-8, 45};	
    fingers[0] = {13, 45};
    fingers[1] = {-13, 45};
	head = {0, 70};	
}

Bone& Skeleton::getBone(int index) {
	return bones[index];
}

Vector2& Skeleton::getJoint(int index) {
	return joints[index];
}

void Skeleton::moveJoint(int index, Vector2 mov) {
	joints[index] += mov;

	for(int i = 0; i < boneCount; i ++) {

		if(&joints[index] == &bones[i].start && &bones[i].end != &torso[1]) 
			moveJoint(bones[i].end, mov);
	}
}

void Skeleton::moveJoint(Vector2& joint, Vector2 mov) {
	joint += mov;

	for(int i = 0; i < boneCount; i ++) {

		if(&joint == &bones[i].start && &bones[i].end != &torso[1]) 
			moveJoint(bones[i].end, mov);
	}
}

void Skeleton::rotateJoint(int index, float rotate) {

    for(int i = 0; i < boneCount; i ++) {

        if(&joints[index] == &bones[i].start && &bones[i].end != &torso[1]) 
            rotateJoint(bones[i].end, bones[i].start, rotate);
    }
}

void Skeleton::rotateJoint(Vector2& joint, Vector2& origin, float rotate) {
    joint = joint.rotate(rotate, origin);

    for(int i = 0; i < boneCount; i ++) {

        if(&joint == &bones[i].start && &bones[i].end != &torso[1]) 
            rotateJoint(bones[i].end, origin, rotate);
    }    
}

void Skeleton::drawBone(Renderer* renderer, std::vector<Clothing> list, int part, Bone& bone, float width, bool flip) {
    width /= 2.f;

    // Shortcuts
    Vector2 start = bone.start;
    Vector2 mid = bone.start + (bone.end - bone.start)/2.f;
    Vector2 end = bone.end;

    // Real world coordinates
    Vector2 pt[4];
    float ptAngle = (end - start).getAngle();

    pt[0] = start.translate(ptAngle + 0.75f * PI, width);
    pt[1] = end.translate(ptAngle + 0.25f * PI, width);
    pt[2] = end.translate(ptAngle - 0.25f * PI, width);
    pt[3] = start.translate(ptAngle - 0.75f * PI, width);

    for(int i = 0; i < list.size(); i ++) {
        sf::Texture* ogTex = list[0].part[part];
        sf::Texture* tex = list[i].part[part];

        if(!tex || !ogTex)
            continue;

        // Drawing texture coordinates
        Vector2 texPt[4] = {
            {0, 0},
            {(float)tex->getSize().x, 0},
            {(float)tex->getSize().x, (float)tex->getSize().y},
            {0.f, (float)tex->getSize().y}
        };

        Vector2 scale = {
            (float)tex->getSize().x / (float)ogTex->getSize().x,
            (float)tex->getSize().y / (float)ogTex->getSize().y
        };

        // Screen coordinates
        sf::Vertex vert[4];

        // Want vs Have Angle
        float rotate = PI/2 - (ptAngle + PI/2);

        for(int j = 0; j < 4; j ++) {

            // Rotate skeleton, and scale accordingly, then rerotate into proper position
            Vector2 pos = pt[j].rotate(rotate, mid);
            pos = mid + (pos - mid) * scale;
            pos = pos.rotate(-rotate, mid);

            vert[j].position = renderer->toScreen(pos);
            vert[j].texCoords = texPt[j];
            vert[j].color = list[i].blend;
        }                                    

        if(flip) {
            std::swap(vert[0].texCoords, vert[3].texCoords);
            std::swap(vert[1].texCoords, vert[2].texCoords);        
        }

        sf::RenderStates states;
        states.texture = tex;
        
        renderer->draw(vert, 4, sf::PrimitiveType::Quads, states);        
    }
}

void Skeleton::drawTorso(Renderer* renderer, std::vector<Clothing> list) {
    Vector2 pt[3][3];

    if(!torsoTopFlipped()) {
        pt[0][0] = shoulder[1];
        pt[0][2] = shoulder[0];

    }else{
        pt[0][0] = shoulder[0];        
        pt[0][2] = shoulder[1];
    }

    pt[0][1] = torso[0];
    pt[1][1] = (torso[1] - torso[0])/2.f + torso[0];
    pt[2][1] = torso[1];

    if(!torsoBottomFlipped()) {
        pt[2][0] = hip[1];
        pt[2][2] = hip[0];

    }else{
        pt[2][0] = hip[0];
        pt[2][2] = hip[1];
    }

    pt[1][0] = (pt[2][0] - pt[0][0])/2.f + pt[0][0];
    pt[1][2] = (pt[2][2] - pt[0][2])/2.f + pt[0][2];

    for(int i = 0; i < list.size(); i ++) {
        sf::Texture* ogTex = torsoTopFlipped() ? list[0].torsoBack : list[0].torsoFront;
        sf::Texture* tex = torsoTopFlipped() ? list[i].torsoBack : list[i].torsoFront;

        if(!tex || !ogTex)
            continue;

        Vector2 texPt[3][3] = {
            {
                {0.f, 0.f},
                {tex->getSize().x/2.f, 0.f},
                {tex->getSize().x/1.f, 0.f}                
            },
            {
                {0.f, tex->getSize().y/2.f},
                {tex->getSize().x/2.f, tex->getSize().y/2.f},
                {tex->getSize().x/1.f, tex->getSize().y/2.f}   
            },
            {
                {0.f, tex->getSize().y/1.f},
                {tex->getSize().x/2.f, tex->getSize().y/1.f},
                {tex->getSize().x/1.f, tex->getSize().y/1.f}  
            }
        };

        Vector2 scale = {
            (float)tex->getSize().x / (float)ogTex->getSize().x,
            (float)tex->getSize().y / (float)ogTex->getSize().y
        };

        sf::Vertex vert[4];

        sf::RenderStates states;
        states.texture = tex;

        // For each 4 quadrants
        vector<sf::Vector2i> indices = {
            {0, 0}, {0, 1}, {1, 1}, {1, 0},
            {0, 1}, {0, 2}, {1, 2}, {1, 1},
            {1, 0}, {1, 1}, {2, 1}, {2, 0},
            {1, 1}, {1, 2}, {2, 2}, {2, 1}                
        };

        // Want vs Have Angle
        float rotate = PI/2 - (pt[0][1] - pt[2][1]).getAngle();

        for(int u = 0; u < indices.size(); u += 4) {

            for(int v = 0; v < 4; v ++) {
                int x = indices[u+v].x;
                int y = indices[u+v].y;

                // Rotate skeleton, and scale accordingly, then rerotate into proper position
                Vector2 pos = pt[x][y].rotate(rotate, pt[1][1]);
                pos = pt[1][1] + (pos - pt[1][1]) * scale;
                pos = pos.rotate(-rotate, pt[1][1]);

                vert[v].position = renderer->toScreen(pos);
                vert[v].texCoords = texPt[x][y];   
                vert[v].color = list[i].blend;
            }
            renderer->draw(vert, 4, sf::PrimitiveType::Quads, states);
        }
    }
}

void Skeleton::drawNeck(Renderer* renderer, std::vector<Clothing> list) {
    float width = ((shoulder[0] - torso[0]).getDistance() + (shoulder[1] - torso[0]).getDistance()) / 3.f;
    drawBone(renderer, list, Clothing::Neck, neck, width);
}

void Skeleton::drawHead(Renderer* renderer, std::vector<Clothing> list, float headAngle) {
    float height = (torso[0] - torso[1]).getDistance();   

    Vector2 psuedoHeadTop;

    if(headAngle >= -PI / 2 && headAngle < PI / 2.f)
        psuedoHeadTop = head.translate(headAngle + PI / 2, height / 4);
    else
        psuedoHeadTop = head.translate(headAngle - PI / 2, height / 4);

    Bone psuedoBone {head, psuedoHeadTop};

    drawBone(renderer, list, Clothing::Head, psuedoBone, height / 2.f, !(headAngle >= -PI / 2 && headAngle < PI / 2.f));  
}

void Skeleton::drawUpperArm(Renderer* renderer, std::vector<Clothing> list, int side) {
    float width = (torso[1] - torso[0]).getDistance() / 5.f;

    Vector2 psuedoShoulder = shoulder[side].translate((hip[side] - shoulder[side]).getAngle(), width / 2);
    Vector2 psuedoElbow = psuedoShoulder + (elbow[side] - shoulder[side]);

    Bone psuedoBone {psuedoShoulder, psuedoElbow};

    drawBone(renderer, list, Clothing::UpperArm, psuedoBone, width, torsoTopFlipped());
}

void Skeleton::drawForeArm(Renderer* renderer, std::vector<Clothing> list, int side) {
    float width = (torso[1] - torso[0]).getDistance() / 5.f;

    Vector2 psuedoShoulder = shoulder[side].translate((hip[side] - shoulder[side]).getAngle(), width / 2);
    Vector2 psuedoElbow = psuedoShoulder + (elbow[side] - shoulder[side]);
    Vector2 psuedoWrist = psuedoElbow + (wrist[side] - elbow[side]);

    Bone psuedoBone {psuedoElbow, psuedoWrist};

    drawBone(renderer, list, Clothing::ForeArm, psuedoBone, width, torsoTopFlipped());   
}

void Skeleton::drawHand(Renderer* renderer, std::vector<Clothing> list, int side) {
    float width = (torso[1] - torso[0]).getDistance() / 5.f;

    Vector2 psuedoShoulder = shoulder[side].translate((hip[side] - shoulder[side]).getAngle(), width / 2);
    Vector2 psuedoElbow = psuedoShoulder + (elbow[side] - shoulder[side]);
    Vector2 psuedoWrist = psuedoElbow + (wrist[side] - elbow[side]);
    Vector2 psuedoHand = psuedoWrist + (fingers[side] - wrist[side]);

    Bone psuedoBone {psuedoWrist, psuedoHand};

    // Determine back or front of hand showing
    int index = Clothing::HandFront;

    if(side == 0 && hand[side].end.x < hand[side].start.x) 
        index = Clothing::HandBack;
    else if(side == 1 && hand[side].end.x > hand[side].start.x) 
        index = Clothing::HandBack;           

    drawBone(renderer, list, index, psuedoBone, width, hand[side].end.x < hand[side].start.x);       
}

void Skeleton::drawThigh(Renderer* renderer, std::vector<Clothing> list, int side) {
    float width = (hip[side] - torso[1]).getDistance();

    Vector2 psuedoHip = torso[1] + (hip[side] - torso[1]) / 2.f;
    Vector2 psuedoKnee = psuedoHip + (knee[side] - hip[side]);

    Bone psuedoBone {psuedoHip, psuedoKnee};
    drawBone(renderer, list, Clothing::Thigh, psuedoBone, width, torsoBottomFlipped()); 
}

void Skeleton::drawCalf(Renderer* renderer, std::vector<Clothing> list, int side) {
    float width = (hip[side] - torso[1]).getDistance();

    Vector2 psuedoHip = torso[1] + (hip[side] - torso[1]) / 2.f;
    Vector2 psuedoKnee = psuedoHip + (knee[side] - hip[side]);
    Vector2 psuedoHeel = psuedoKnee + (heel[side] - knee[side]);

    Bone psuedoBone {psuedoKnee, psuedoHeel};
    drawBone(renderer, list, Clothing::Calf, psuedoBone, width, torsoBottomFlipped());   
}

void Skeleton::drawFoot(Renderer* renderer, std::vector<Clothing> list, int side) {
    float width = (hip[side] - torso[1]).getDistance();

    Vector2 psuedoHip = torso[1] + (hip[side] - torso[1]) / 2.f;
    Vector2 psuedoKnee = psuedoHip + (knee[side] - hip[side]);
    Vector2 psuedoHeel = psuedoKnee + (heel[side] - knee[side]);
    Vector2 psuedoToes = psuedoHeel + (toes[side] - heel[side]);

    Bone psuedoBone {psuedoHeel, psuedoToes};

    float rotate = PI/2 - (knee[side] - heel[side]).getAngle();
    Vector2 norm = toes[side].rotate(rotate, heel[side]);

    drawBone(renderer, list, Clothing::Foot, psuedoBone, width, norm.x < heel[side].x);    
}

void Skeleton::draw(vector<Clothing> list, float headAngle) {
    draw(&g::video, list, headAngle);
}

static Renderer renderer(TargetMode::Texture);

void Skeleton::draw(Renderer* final, vector<Clothing> list, float headAngle, bool unused) {

    if(!final)
        return;

    sf::Shader* outline = outline = g::save.getShader("outline.fs");

    renderer.setSize(final->getSize());

    if(!renderer.reload())
        return;

    renderer.camera = final->camera;

    for(int i = 0; i < SkeletonDrawOrder::Total; i ++) {
        renderer.clear(sf::Color::Transparent);

        switch(order[i]) {

            case SkeletonDrawOrder::LegLeft:
                drawCalf(&renderer, list, 1);
                drawThigh(&renderer, list, 1);
                drawFoot(&renderer, list, 1); 
                break; 

            case SkeletonDrawOrder::LegRight:
                drawCalf(&renderer, list, 0);
                drawThigh(&renderer, list, 0);  
                drawFoot(&renderer, list, 0);
                break; 

            case SkeletonDrawOrder::Body:

                if(!torsoTopFlipped()) {
                    drawTorso(&renderer, list);
                    drawNeck(&renderer, list);
                    drawHead(&renderer, list, headAngle);

                }else {
                    drawNeck(&renderer, list);
                    drawHead(&renderer, list, headAngle);
                    drawTorso(&renderer, list);
                }
                break;

            case SkeletonDrawOrder::Head:
                continue;
                break;           

            case SkeletonDrawOrder::ArmLeft:
                drawUpperArm(&renderer, list, 0);
                drawForeArm(&renderer, list, 0);
                drawHand(&renderer, list, 0);
                break;

            case SkeletonDrawOrder::ArmRight:
                drawUpperArm(&renderer, list, 1);
                drawForeArm(&renderer, list, 1);
                drawHand(&renderer, list, 1);
                break;
        }

        renderer.display();

        // Apply the renderer skeleton to the final target
        sf::Vertex vert[4] {
            sf::Vertex(Vector2(0, 0), Vector2(0, 1)),
            sf::Vertex(Vector2(final->getSize().x, 0), Vector2(1, 1)),
            sf::Vertex(Vector2(final->getSize().x, final->getSize().y), Vector2(1, 0)),
            sf::Vertex(Vector2(0, final->getSize().y), Vector2(0, 0))
        };

        sf::RenderStates states;

        if(outline) {
            outline->setUniform("texture", *renderer.getTexture());         
            states.shader = outline;
        }

        final->draw(vert, 4, sf::PrimitiveType::Quads, states);        
    }
}

bool Skeleton::torsoTopFlipped() {
    Skeleton copy = *this;
    float rotate = PI/2 - (torso[0] - torso[1]).getAngle();

    for(int i = 0; i < jointCount; i ++) {
        copy.joints[i] = copy.joints[i].rotate(rotate, torso[1]);
    }

    return copy.shoulder[0].x < copy.shoulder[1].x;
}

bool Skeleton::torsoBottomFlipped() {
    Skeleton copy = *this;
    float rotate = PI/2 - (torso[0] - torso[1]).getAngle();

    for(int i = 0; i < jointCount; i ++) {
        copy.joints[i] = copy.joints[i].rotate(rotate, torso[1]);
    }

    return copy.hip[0].x < copy.hip[1].x;
}