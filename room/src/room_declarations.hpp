#include "Render_Utils.h"

namespace models {
	Core::RenderContext shipContext;
	Core::RenderContext sphereContext;
	Core::RenderContext skyboxContext;
	Core::RenderContext doorContext;
	Core::RenderContext planeContext;
	Core::RenderContext roomContext;
	Core::RenderContext windowContext;
	Core::RenderContext testContext;
	Core::RenderContext commodeContext;
	Core::RenderContext floorContext;
	Core::RenderContext wallContext;
	Core::RenderContext tvTableContext;
	Core::RenderContext tvContext;
	Core::RenderContext sofaContext;
	Core::RenderContext tableContext;
	Core::RenderContext guitarContext;
	Core::RenderContext plantContext;
	Core::RenderContext vaseLeftContext;
	Core::RenderContext vaseRightContext;
	Core::RenderContext lampContext;
	Core::RenderContext cakeContext;
	Core::RenderContext deskContext;
	Core::RenderContext chairContext;
	Core::RenderContext paintingContext;
	Core::RenderContext kukriContext;
	Core::RenderContext fireContext;
	Core::RenderContext fireplaceContext;
	Core::RenderContext fireParticleContext;
}

namespace texture {
	GLuint cubemapTexture;

	GLuint defaultTexture;
	GLuint defaultTextureNormal;
	GLuint defaultTextureArm;

	GLuint spaceshipTexture;
	GLuint spaceshipTextureNormal;
	GLuint spaceshipTextureArm;

	GLuint commodeTexture;
	GLuint commodeTextureNormal;
	GLuint commodeTextureArm;

	GLuint floorTexture;
	GLuint floorTextureNormal;
	GLuint floorTextureArm;

	GLuint wallTexture;
	GLuint wallTextureNormal;
	GLuint wallTextureArm;

	GLuint tvTableTexture;
	GLuint tvTableTextureNormal;
	GLuint tvTableTextureArm;

	GLuint tvTexture;
	GLuint tvTextureNormal;
	GLuint tvTextureArm;

	GLuint sofaTexture;
	GLuint sofaTextureNormal;
	GLuint sofaTextureArm;

	GLuint windowTexture;
	GLuint windowTextureNormal;
	GLuint windowTextureArm;

	GLuint doorTexture;
	GLuint doorTextureNormal;
	GLuint doorTextureArm;

	GLuint tableTexture;
	GLuint tableTextureNormal;
	GLuint tableTextureArm;

	GLuint guitarTexture;
	GLuint guitarTextureNormal;
	GLuint guitarTextureArm;

	GLuint plantTexture;
	GLuint plantTextureNormal;
	GLuint plantTextureArm;

	GLuint vaseTexture;
	GLuint vaseTextureNormal;
	GLuint vaseTextureArm;

	GLuint lampTexture;
	GLuint lampTextureNormal;
	GLuint lampTextureArm;

	GLuint cakeTexture;
	GLuint cakeTextureNormal;
	GLuint cakeTextureArm;

	GLuint deskTexture;
	GLuint deskTextureNormal;
	GLuint deskTextureArm;

	GLuint chairTexture;
	GLuint chairTextureNormal;
	GLuint chairTextureArm;

	GLuint paintingTexture;
	GLuint paintingTextureNormal;
	GLuint paintingTextureArm;

	GLuint kukriTexture;
	GLuint kukriTextureNormal;
	GLuint kukriTextureArm;

	GLuint flames1Texture;
	GLuint flames2Texture;
	GLuint flames3Texture;
	GLuint flames4Texture;

	GLuint fireplaceTexture;
	GLuint fireplaceTextureNormal;
	GLuint fireplaceBackTexture;
	GLuint fireplaceBackTextureNormal;
	GLuint fireplaceBackTextureArm;
}

void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void initDeclarations()
{
	loadModelToContext("./models/sphere/sphere.obj", models::sphereContext);
	loadModelToContext("./models/spaceship/spaceship.obj", models::shipContext);
	loadModelToContext("./models/cube/cube.obj", models::skyboxContext);
	loadModelToContext("./models/door/door.obj", models::doorContext);
	loadModelToContext("./models/plane/plane.obj", models::planeContext);
	loadModelToContext("./models/room/room.obj", models::roomContext);
	loadModelToContext("./models/window/window.obj", models::windowContext);
	loadModelToContext("./models/test/test.obj", models::testContext);
	loadModelToContext("./models/commode/commode.obj", models::commodeContext);
	loadModelToContext("./models/tv_table/tv_table.obj", models::tvTableContext);
	loadModelToContext("./models/tv/tv.obj", models::tvContext);
	loadModelToContext("./models/sofa/sofa.obj", models::sofaContext);
	loadModelToContext("./models/table/table.obj", models::tableContext);
	loadModelToContext("./models/guitar/guitar.obj", models::guitarContext);
	loadModelToContext("./models/plant/plant.obj", models::plantContext);
	loadModelToContext("./models/vase/vase_left.obj", models::vaseLeftContext);
	loadModelToContext("./models/vase/vase_right.obj", models::vaseRightContext);
	loadModelToContext("./models/lamp/lamp.obj", models::lampContext);
	loadModelToContext("./models/cake/cake.obj", models::cakeContext);
	loadModelToContext("./models/desk/desk.obj", models::deskContext);
	loadModelToContext("./models/chair/chair.obj", models::chairContext);
	loadModelToContext("./models/painting/painting.obj", models::paintingContext);
	loadModelToContext("./models/kukri/kukri.obj", models::kukriContext);
	loadModelToContext("./models/fire/fire.obj", models::fireContext);
	loadModelToContext("./models/fireplace/fireplace.obj", models::fireplaceContext);
	loadModelToContext("./models/fire_particle/fire_particle.obj", models::fireParticleContext);

	texture::defaultTexture = Core::LoadTexture("textures/default/default.png");
	texture::defaultTextureNormal = Core::LoadTexture("textures/default/default_normalmap.png");
	texture::defaultTextureArm = Core::LoadTexture("textures/default/default_arm.png");

	texture::spaceshipTexture = Core::LoadTexture("textures/spaceship/spaceship.jpg");
	texture::spaceshipTextureNormal = Core::LoadTexture("textures/spaceship/spaceship_normalmap.png");
	texture::spaceshipTextureArm = Core::LoadTexture("textures/spaceship/spaceship_arm.png");

	texture::commodeTexture = Core::LoadTexture("textures/commode/commode.png");
	texture::commodeTextureNormal = Core::LoadTexture("textures/commode/commode_normalmap.png");
	texture::commodeTextureArm = Core::LoadTexture("textures/commode/commode_arm.png");

	texture::floorTexture = Core::LoadTexture("textures/floor/floor.png");
	texture::floorTextureNormal = Core::LoadTexture("textures/floor/floor_normalmap.png");
	texture::floorTextureArm = Core::LoadTexture("textures/floor/floor_arm.png");

	texture::wallTexture = Core::LoadTexture("textures/wall/wall.png");
	texture::wallTextureNormal = Core::LoadTexture("textures/wall/wall_normalmap.png");
	texture::wallTextureArm = Core::LoadTexture("textures/wall/wall_arm.png");

	texture::tvTableTexture = Core::LoadTexture("textures/tv_table/tv_table.png");
	texture::tvTableTextureNormal = Core::LoadTexture("textures/tv_table/tv_table_normalmap.png");
	texture::tvTableTextureArm = Core::LoadTexture("textures/tv_table/tv_table_arm.png");

	texture::tvTexture = Core::LoadTexture("textures/tv/tv_off.png");
	texture::tvTextureNormal = Core::LoadTexture("textures/tv/tv_normalmap.png");
	texture::tvTextureArm = Core::LoadTexture("textures/tv/tv_arm.png");

	texture::sofaTexture = Core::LoadTexture("textures/sofa/sofa.png");
	texture::sofaTextureNormal = Core::LoadTexture("textures/sofa/sofa_normalmap.png");
	texture::sofaTextureArm = Core::LoadTexture("textures/sofa/sofa_arm.png");

	texture::windowTexture = Core::LoadTexture("textures/window/window.png");
	texture::windowTextureNormal = Core::LoadTexture("textures/window/window_normalmap.png");
	texture::windowTextureArm = Core::LoadTexture("textures/window/window_arm.png");

	texture::doorTexture = Core::LoadTexture("textures/door/door.jpg");
	texture::doorTextureNormal = Core::LoadTexture("textures/door/door_normalmap.png");
	texture::doorTextureArm = Core::LoadTexture("textures/door/door_arm.png");

	texture::tableTexture = Core::LoadTexture("textures/table/table.png");
	texture::tableTextureNormal = Core::LoadTexture("textures/table/table_normalmap.png");
	texture::tableTextureArm = Core::LoadTexture("textures/table/table_arm.png");

	texture::guitarTexture = Core::LoadTexture("textures/guitar/guitar.png");
	texture::guitarTextureNormal = Core::LoadTexture("textures/guitar/guitar_normalmap.png");
	texture::guitarTextureArm = Core::LoadTexture("textures/guitar/guitar_arm.png");

	texture::plantTexture = Core::LoadTexture("textures/plant/plant.png");
	texture::plantTextureNormal = Core::LoadTexture("textures/plant/plant_normalmap.png");
	texture::plantTextureArm = Core::LoadTexture("textures/plant/plant_arm.png");

	texture::vaseTexture = Core::LoadTexture("textures/vase/vase.png");
	texture::vaseTextureNormal = Core::LoadTexture("textures/vase/vase_normalmap.png");
	texture::vaseTextureArm = Core::LoadTexture("textures/vase/vase_arm.png");

	texture::lampTexture = Core::LoadTexture("textures/lamp/lamp.png");
	texture::lampTextureNormal = Core::LoadTexture("textures/lamp/lamp_normalmap.png");
	texture::lampTextureArm = Core::LoadTexture("textures/lamp/lamp_arm.png");

	texture::cakeTexture = Core::LoadTexture("textures/cake/cake.png");
	texture::cakeTextureNormal = Core::LoadTexture("textures/cake/cake_normalmap.png");
	texture::cakeTextureArm = Core::LoadTexture("textures/cake/cake_arm.png");

	texture::deskTexture = Core::LoadTexture("textures/desk/desk.png");
	texture::deskTextureNormal = Core::LoadTexture("textures/desk/desk_normalmap.png");
	texture::deskTextureArm = Core::LoadTexture("textures/desk/desk_arm.png");

	texture::chairTexture = Core::LoadTexture("textures/chair/chair.png");
	texture::chairTextureNormal = Core::LoadTexture("textures/chair/chair_normalmap.png");
	texture::chairTextureArm = Core::LoadTexture("textures/chair/chair_arm.png");

	texture::paintingTexture = Core::LoadTexture("textures/painting/painting.png");
	texture::paintingTextureNormal = Core::LoadTexture("textures/painting/painting_normalmap.png");
	texture::paintingTextureArm = Core::LoadTexture("textures/painting/painting_arm.png");

	texture::kukriTexture = Core::LoadTexture("textures/kukri/kukri.png");
	texture::kukriTextureNormal = Core::LoadTexture("textures/kukri/kukri_normalmap.png");
	texture::kukriTextureArm = Core::LoadTexture("textures/kukri/kukri_arm.png");

	texture::flames1Texture = Core::LoadTexture("textures/flames/flames1.png");
	texture::flames2Texture = Core::LoadTexture("textures/flames/flames2.png");
	texture::flames3Texture = Core::LoadTexture("textures/flames/flames3.png");
	texture::flames4Texture = Core::LoadTexture("textures/flames/flames4.png");

	texture::fireplaceTexture = Core::LoadTexture("textures/fireplace/fireplace.png");
	texture::fireplaceTextureNormal = Core::LoadTexture("textures/fireplace/fireplace_normalmap.png");
	texture::fireplaceBackTexture = Core::LoadTexture("textures/fireplace/fireplace_back.png");
	texture::fireplaceBackTextureNormal = Core::LoadTexture("textures/fireplace/fireplace_back_normalmap.png");
	texture::fireplaceBackTextureArm = Core::LoadTexture("textures/fireplace/fireplace_back_arm.png");
}