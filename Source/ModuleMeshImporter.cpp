#include "ModuleMeshImporter.h"
#include "ModuleRenderer3D.h"

#include "Application.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/libx86/assimp-vc140-mt.lib")

#include <string>

void myCallback(const char* msg, char* userData)
{
	CONSOLE_LOG("%s", msg);
}

ModuleMeshImporter::ModuleMeshImporter(bool start_enabled) : Module(start_enabled)
{
	name = "MeshImporter";
}

ModuleMeshImporter::~ModuleMeshImporter() {}

bool ModuleMeshImporter::Init(JSON_Object* jObject)
{
	struct aiLogStream stream;
	stream.callback = myCallback;
	aiAttachLogStream(&stream);

	return true;
}

bool ModuleMeshImporter::CleanUp()
{
	aiDetachAllLogStreams();
	return true;
}

bool ModuleMeshImporter::LoadMeshesFromFile(const char* path) const
{
	bool ret = false;

	App->renderer3D->ClearTextures();
	App->renderer3D->ClearMeshes();
	
	uint postProcessingFlags = 0;
	postProcessingFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
	//postProcessingFlags |= aiPostProcessSteps::aiProcess_Triangulate;
	//postProcessingFlags |= aiPostProcessSteps::aiProcess_GenSmoothNormals;
	//postProcessingFlags |= aiPostProcessSteps::aiProcess_JoinIdenticalVertices;

	const aiScene* scene = aiImportFile(path, postProcessingFlags);

	if (scene != nullptr)
	{
		InitMeshesFromScene(scene, path);
		aiReleaseImport(scene);

		ret = true;
	}
	else
		CONSOLE_LOG("Error loading scene %s", path);

	return ret;
}

bool ModuleMeshImporter::LoadMeshesFromMemory(const char* buffer, unsigned int& bufferSize, const char* path) const
{
	bool ret = false;

	App->renderer3D->ClearTextures();
	App->renderer3D->ClearMeshes();

	uint postProcessingFlags = 0;
	postProcessingFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
	//postProcessingFlags |= aiPostProcessSteps::aiProcess_Triangulate;
	//postProcessingFlags |= aiPostProcessSteps::aiProcess_GenSmoothNormals;
	//postProcessingFlags |= aiPostProcessSteps::aiProcess_JoinIdenticalVertices;

	const aiScene* scene = aiImportFileFromMemory(buffer, bufferSize, postProcessingFlags, nullptr);

	if (scene != nullptr)
	{
		InitMeshesFromScene(scene, path);
		aiReleaseImport(scene);

		ret = true;
	}

	return ret;
}

bool ModuleMeshImporter::LoadMeshesWithPHYSFS(const char* path) const
{
	bool ret = false;

	char* buffer;
	uint size;

	if (App->filesystem->OpenRead(path, &buffer, size))
	{
		ret = LoadMeshesFromMemory(buffer, size, path);

		if (!ret)
			CONSOLE_LOG("Error loading scene %s", path);

		delete[] buffer;
	}

	return ret;
}

void ModuleMeshImporter::InitMeshesFromScene(const aiScene* scene, const char* path) const
{
	// Meshes
	for (uint i = 0; i < scene->mNumMeshes; ++i)
	{
		Mesh* mesh = new Mesh();

		mesh->name = new char[strlen(scene->mMeshes[i]->mName.C_Str()) + 1];
		strcpy_s((char*)mesh->name, strlen(scene->mMeshes[i]->mName.C_Str()) + 1, scene->mMeshes[i]->mName.C_Str());

		// Unique vertices
		mesh->verticesSize = scene->mMeshes[i]->mNumVertices;
		mesh->vertices = new float[mesh->verticesSize * 3];
		memcpy(mesh->vertices, scene->mMeshes[i]->mVertices, sizeof(float) * mesh->verticesSize * 3);
		CONSOLE_LOG("New mesh with %d vertices", mesh->verticesSize);

		// Indices
		if (scene->mMeshes[i]->HasFaces())
		{
			mesh->indicesSize = scene->mMeshes[i]->mNumFaces * 3;
			mesh->indices = new uint[mesh->indicesSize];
			
			for (uint j = 0; j < scene->mMeshes[i]->mNumFaces; ++j)
			{
				if (scene->mMeshes[i]->mFaces[j].mNumIndices != 3)
				{
					CONSOLE_LOG("WARNING, geometry face with != 3 indices!");
				}
				else
				{
					memcpy(&mesh->indices[j * 3], scene->mMeshes[i]->mFaces[j].mIndices, 3 * sizeof(uint));
				}
			}
		}

		// Normals
		/*
		if (scene->mMeshes[i]->HasNormals())
		{
			mesh->normals = new float[mesh->verticesSize * 3];
			memcpy(mesh->normals, scene->mMeshes[i]->mNormals, sizeof(float) * mesh->verticesSize * 3);
			CONSOLE_LOG("Mesh vertices normals loaded");
		}
		*/
	
		// Texture coords
		if (scene->mMeshes[i]->HasTextureCoords(0))
		{
			mesh->textureCoords = new float[scene->mMeshes[i]->mNumVertices * 2];

			for (uint j = 0; j < scene->mMeshes[i]->mNumVertices; ++j)
			{
				memcpy(&mesh->textureCoords[j * 2], &scene->mMeshes[i]->mTextureCoords[0][j].x, sizeof(float));
				memcpy(&mesh->textureCoords[(j * 2) + 1], &scene->mMeshes[i]->mTextureCoords[0][j].y, sizeof(float));
			}
			CONSOLE_LOG("Mesh tex coords at channel 0 loaded");
		}

		// Transform
		if (scene->mRootNode != nullptr)
		{
			aiVector3D position;
			aiVector3D scale;
			aiQuaternion rotation;

			scene->mRootNode->mTransformation.Decompose(scale, rotation, position);
			mesh->position = { position.x, position.y, position.z };
			mesh->scale = { scale.x, scale.y, scale.z };
			mesh->rotation = { rotation.x, rotation.y, rotation.z, rotation.w };
		}

		mesh->Init();
		App->renderer3D->AddMesh(mesh);
	}

	// Material
	if (scene->mMaterials[0] != nullptr && path != nullptr)
	{
		aiString textureName;
		scene->mMaterials[0]->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);
		std::string realTextureName = textureName.data;
		realTextureName = realTextureName.substr((realTextureName.find_last_of("\\") + 1), strlen(textureName.data));
		std::string fbxPathString = path;
		std::string texturePath = fbxPathString.substr(0, fbxPathString.find_last_of("\\") + 1) + realTextureName.data();
		if (!App->tex->LoadImageFromFile(texturePath.data()))
		{
			std::string texturePath = "Assets\\Textures\\Models\\";
			texturePath += realTextureName.data();
			if (!App->tex->LoadImageFromFile(texturePath.data()))
			{
				std::string texturePath = "Assets\\Textures\\";
				texturePath += realTextureName.data();
				if (!App->tex->LoadImageFromFile(texturePath.data()))
				{
					std::string texturePath = "Game\\";
					texturePath += realTextureName.data();
					if (!App->tex->LoadImageFromFile(texturePath.data()))
					{
						CONSOLE_LOG("Impossible to load texture: %s", realTextureName.data());
					}
					else
					{
						CONSOLE_LOG("Loaded correctly texture: %s", realTextureName.data());
					}
				}
				else
				{
					CONSOLE_LOG("Loaded correctly texture: %s", realTextureName.data());
				}
			}
			else
			{
				CONSOLE_LOG("Loaded correctly texture: %s", realTextureName.data());
			}
		}
	}

	App->renderer3D->SetCheckTexture(false);
	App->renderer3D->SetMultitexturing(false);
	App->renderer3D->SetCurrentTextureUnits(1);

	// Geometry
	App->renderer3D->SetGeometryActive(true);
	App->renderer3D->SetGeometryName(App->filesystem->GetFileNameFromPath(path));
	App->renderer3D->CreateGeometryBoundingBox();
	App->renderer3D->LookAtGeometry();
}