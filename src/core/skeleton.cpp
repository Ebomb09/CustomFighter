#include "render_instance.h"
#include "skeleton.h"

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

	return *this;
}

Skeleton& Skeleton::operator=(Skeleton&& move) {

	for(int i = 0; i < jointCount; i ++)
		joints[i] = std::move(move.joints[i]);	

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
		if(&joints[index] == &bones[i].start) {

			if(&bones[i].end != &torso[1])
				moveJoint(bones[i].end, mov);
		}
	}
}

void Skeleton::moveJoint(Vector2& joint, Vector2 mov) {
	joint += mov;

	for(int i = 0; i < boneCount; i ++) {
		if(&joint == &bones[i].start) {

			if(&bones[i].end != &torso[1])
				moveJoint(bones[i].end, mov);
		}
	}
}

void Skeleton::drawBone(Bone& bone, sf::Texture* tex, float width, bool flip) {

    if(!tex)
        return;

    width /= 2.f;

    float angle = (bone.end - bone.start).getAngle();

    Vector2 start = bone.start;
    Vector2 end = bone.end;

    sf::Vertex v[] = {
        sf::Vertex(start.translate(angle - 0.75f * PI, width), {0.f, 0.f}),     
        sf::Vertex(end.translate(angle - 0.25f * PI, width), {(float)tex->getSize().x, 0}),
        sf::Vertex(end.translate(angle + 0.25f * PI, width), {(float)tex->getSize().x, (float)tex->getSize().y}),   
        sf::Vertex(start.translate(angle + 0.75f * PI, width), {0.f, (float)tex->getSize().y}),                                             
    };

    if(flip) {
        std::swap(v[0].texCoords, v[3].texCoords);
        std::swap(v[1].texCoords, v[2].texCoords);        
    }

    sf::RenderStates states;
    states.texture = tex;

    g::video.window.draw(v, 4, sf::PrimitiveType::Quads, states);
}

void Skeleton::drawTorso(sf::Texture* texFront, sf::Texture* texBack) {

    if(!texFront || !texBack)
        return;

    Vector2 mid[2];
    Vector2 midTorso = (torso[1] - torso[0])/2 + torso[0];

    // Normal
    bool flipTop = torsoTopFlipped();
    bool flipBot = torsoBottomFlipped();

    if(!flipTop && !flipBot) {
        mid[0] = (hip[0] - shoulder[0])/2 + shoulder[0];
        mid[1] = (hip[1] - shoulder[1])/2 + shoulder[1];        

    // Top flipped
    }else if(flipTop && !flipBot) {
        mid[0] = (hip[0] - shoulder[1])/2 + shoulder[1];
        mid[1] = (hip[1] - shoulder[0])/2 + shoulder[0];   

    // Bottom flipped
    }else if(!flipTop && flipBot) {
        mid[0] = (hip[1] - shoulder[0])/2 + shoulder[0];
        mid[1] = (hip[0] - shoulder[1])/2 + shoulder[1];  

    // Both flipped
    }else {
        mid[0] = (hip[1] - shoulder[1])/2 + shoulder[1];
        mid[1] = (hip[0] - shoulder[0])/2 + shoulder[0];       
    }

    // Upper half
    {
        sf::Texture* tex;
        sf::Vertex v[8];

        if(shoulder[0].x > shoulder[1].x){
            tex = texFront;

            v[0] = sf::Vertex(torso[0],      	Vector2(tex->getSize().x/2, 0));
            v[1] = sf::Vertex(shoulder[0],   	Vector2(tex->getSize().x, 0));
            v[2] = sf::Vertex(mid[0],           Vector2(tex->getSize().x, tex->getSize().y/2));
            v[3] = sf::Vertex(midTorso,         Vector2(tex->getSize().x/2, tex->getSize().y/2));

            v[4] = sf::Vertex(torso[0],      	Vector2(tex->getSize().x/2, 0));
            v[5] = sf::Vertex(midTorso,         Vector2(tex->getSize().x/2, tex->getSize().y/2));
            v[6] = sf::Vertex(mid[1],           Vector2(0, tex->getSize().y/2));         
            v[7] = sf::Vertex(shoulder[1],		Vector2(0, 0));

        }else{
            tex = texBack;

            v[0] = sf::Vertex(torso[0],      	Vector2(tex->getSize().x/2, 0));
            v[1] = sf::Vertex(midTorso,         Vector2(tex->getSize().x/2, tex->getSize().y/2));
            v[2] = sf::Vertex(mid[1],           Vector2(0, tex->getSize().y/2));
            v[3] = sf::Vertex(shoulder[0],   	Vector2(0, 0));

            v[4] = sf::Vertex(torso[0],      	Vector2(tex->getSize().x/2, 0));
            v[5] = sf::Vertex(shoulder[1],   	Vector2(tex->getSize().x, 0));
            v[6] = sf::Vertex(mid[0],			Vector2(tex->getSize().x, tex->getSize().y/2));                                  
            v[7] = sf::Vertex(midTorso,			Vector2(tex->getSize().x/2, tex->getSize().y/2));   
        }
        
        sf::RenderStates states;
        states.texture = tex;

        g::video.window.draw(v, 8, sf::PrimitiveType::Quads, states);
    }

    // Lower half
    {
        sf::Texture* tex;
        sf::Vertex v[8];

        if(hip[0].x > hip[1].x){
            tex = texFront;

            v[0] = sf::Vertex(midTorso,     Vector2(tex->getSize().x/2, tex->getSize().y/2));
            v[1] = sf::Vertex(mid[0],       Vector2(tex->getSize().x, tex->getSize().y/2));
            v[2] = sf::Vertex(hip[0],       Vector2(tex->getSize().x, tex->getSize().y));
            v[3] = sf::Vertex(torso[1],     Vector2(tex->getSize().x/2, tex->getSize().y));

            v[4] = sf::Vertex(midTorso,     Vector2(tex->getSize().x/2, tex->getSize().y/2));
            v[5] = sf::Vertex(torso[1],     Vector2(tex->getSize().x/2, tex->getSize().y));
            v[6] = sf::Vertex(hip[1],       Vector2(0, tex->getSize().y));
            v[7] = sf::Vertex(mid[1],       Vector2(0, tex->getSize().y/2));              

        }else{
            tex = texBack;

            v[0] = sf::Vertex(midTorso,     Vector2(tex->getSize().x/2, tex->getSize().y/2));
            v[1] = sf::Vertex(torso[1],     Vector2(tex->getSize().x/2, tex->getSize().y));
            v[2] = sf::Vertex(hip[0],       Vector2(tex->getSize().x, tex->getSize().y));
            v[3] = sf::Vertex(mid[1],       Vector2(tex->getSize().x, tex->getSize().y/2));

            v[4] = sf::Vertex(midTorso,     Vector2(tex->getSize().x/2, tex->getSize().y/2));
            v[5] = sf::Vertex(mid[0],       Vector2(0, tex->getSize().y/2));
            v[6] = sf::Vertex(hip[1],       Vector2(0, tex->getSize().y));
            v[7] = sf::Vertex(torso[1],     Vector2(tex->getSize().x/2, tex->getSize().y));              
        }
        
        sf::RenderStates states;
        states.texture = tex;

        g::video.window.draw(v, 8, sf::PrimitiveType::Quads, states);
    }
} 

void Skeleton::drawNeck(sf::Texture* tex) {
    float width = ((shoulder[0] - torso[0]).getDistance() + (shoulder[1] - torso[0]).getDistance()) / 3.f;
    drawBone(neck, tex, width);
}

void Skeleton::drawHead(sf::Texture* tex, float headAngle) {
    float height = (torso[0] - torso[1]).getDistance();   

    Vector2 psuedoHeadTop;

    if(headAngle > PI / 2.f || headAngle < PI / -2.f)
        psuedoHeadTop = head.translate(headAngle + PI / 2, height / 4);
    else
        psuedoHeadTop = head.translate(headAngle - PI / 2, height / 4);

    Bone psuedoBone {head, psuedoHeadTop};

    drawBone(psuedoBone, tex, height / 2.f, (headAngle > PI / 2.f || headAngle < PI / -2.f));
}

void Skeleton::drawUpperArm(int side, sf::Texture* tex) {
    float width = (torso[1] - torso[0]).getDistance() / 5.f;
    drawBone(upperarm[side], tex, width, torsoTopFlipped());
}

void Skeleton::drawForeArm(int side, sf::Texture* tex) {
    float width = (torso[1] - torso[0]).getDistance() / 5.f;    
    drawBone(forearm[side], tex, width, torsoTopFlipped());
}

void Skeleton::drawHand(int side, sf::Texture* tex) {
    float width = (torso[1] - torso[0]).getDistance() / 5.f;
    float angle = (fingers[side] - wrist[side]).getAngle();
    drawBone(hand[side], tex, width, (angle > PI / 2.f || angle < PI / -2.f));
}

void Skeleton::drawThigh(int side, sf::Texture* tex) {
    float width = (hip[side] - torso[1]).getDistance();

    Vector2 psuedoHip = torso[1] + (hip[side] - torso[1]) / 2.f;
    Vector2 psuedoKnee = psuedoHip + (knee[side] - hip[side]);

    Bone psuedoBone {psuedoHip, psuedoKnee};
    drawBone(psuedoBone, tex, width, torsoBottomFlipped());
}

void Skeleton::drawCalf(int side, sf::Texture* tex) {
    float width = (hip[side] - torso[1]).getDistance();

    Vector2 psuedoHip = torso[1] + (hip[side] - torso[1]) / 2.f;
    Vector2 psuedoKnee = psuedoHip + (knee[side] - hip[side]);
    Vector2 psuedoHeel = psuedoKnee + (heel[side] - knee[side]);

    Bone psuedoBone {psuedoKnee, psuedoHeel};
    drawBone(psuedoBone, tex, width, torsoBottomFlipped());
}

void Skeleton::drawFoot(int side, sf::Texture* tex) {
    float width = (hip[side] - torso[1]).getDistance();
    float angle = (toes[side] - heel[side]).getAngle();

    Vector2 psuedoHip = torso[1] + (hip[side] - torso[1]) / 2.f;
    Vector2 psuedoKnee = psuedoHip + (knee[side] - hip[side]);
    Vector2 psuedoHeel = psuedoKnee + (heel[side] - knee[side]);
    Vector2 psuedoToes = psuedoHeel + (toes[side] - heel[side]);

    Bone psuedoBone {psuedoHeel, psuedoToes};

    drawBone(psuedoBone, tex, width, (angle > PI / 2.f || angle < PI / -2.f));
}

void Skeleton::draw(std::vector<Clothing*> clothes, float headAngle) {

    for(auto& it : clothes) {
        drawNeck(it->neck);
        drawHead(it->head, headAngle);
    }

    for(auto& it : clothes)
        drawTorso(it->torsoFront, it->torsoBack);

    for(int i = 0; i < 2; i ++){

        for(auto& it : clothes)
            drawUpperArm(i, it->upperArm);     

        for(auto& it : clothes) {
            drawForeArm(i, it->foreArm);
            drawHand(i, it->hand); 
        }

        for(auto& it : clothes)
            drawThigh(i, it->thigh);

        for(auto& it : clothes) {
            drawCalf(i, it->calf);        
            drawFoot(i, it->foot); 
        }
    }
}

bool Skeleton::torsoTopFlipped() {
    return shoulder[0].x < shoulder[1].x;
}

bool Skeleton::torsoBottomFlipped() {
    return hip[0].x < hip[1].x;
}