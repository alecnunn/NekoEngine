#include "Globals.h"
#include "Application.h"
#include "ModuleFileSystem.h"

#include "physfs/include/physfs.h"
#include "SDL/include/SDL.h"

#pragma comment(lib, "physfs/libx86/physfs.lib")

ModuleFileSystem::ModuleFileSystem(bool start_enabled) : Module(start_enabled)
{
	name = "FileSystem";

	PHYSFS_init(nullptr);

	AddPath(".");
	AddPath("./Assets/", "Assets");
}

ModuleFileSystem::~ModuleFileSystem() {}

bool ModuleFileSystem::Init(JSON_Object* jObject)
{
	bool ret = true;

	CONSOLE_LOG("Loading File System");

	if (PHYSFS_setWriteDir(".") == 0)
		CONSOLE_LOG("File System error while creating write dir: %s\n", PHYSFS_getLastError());

	return ret;
}

bool ModuleFileSystem::CleanUp()
{
	bool ret = true;

	CONSOLE_LOG("Freeing File System subsystem");
	PHYSFS_deinit();

	return ret;
}

bool ModuleFileSystem::AddPath(const char* newDir, const char* mountPoint)
{
	bool ret = false;

	if (PHYSFS_mount(newDir, mountPoint, 1) != 0)
		ret = true;
	else
		CONSOLE_LOG("File System error while adding a path or zip (%s): %s\n", newDir, PHYSFS_getLastError());
		
	return ret;
}

uint ModuleFileSystem::OpenRead(const char* file, char** buffer, uint& size) const
{
	uint ret = 0;
	if (PHYSFS_exists(file))
	{
		PHYSFS_file* fsFile = PHYSFS_openRead(file);

		if (fsFile != nullptr)
		{
			size = PHYSFS_fileLength(fsFile);

			if (size > 0)
			{
				*buffer = new char[(uint)size];
				PHYSFS_sint64 readed = PHYSFS_readBytes(fsFile, *buffer, size);

				if (readed == size)
					ret = (uint)readed;
				else
				{
					CONSOLE_LOG("File System error while reading from file %s: %s\n", file, PHYSFS_getLastError());
					RELEASE(buffer);
				}
			}

			if (PHYSFS_close(fsFile) == 0)
				CONSOLE_LOG("File System error while closing file %s: %s\n", file, PHYSFS_getLastError());
		}
		else
			CONSOLE_LOG("File System error while opening file %s: %s\n", file, PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG("File System could not find the file");

	return ret;
}

uint ModuleFileSystem::OpenWrite(const char* file, const char* buffer) const
{
	uint ret = 0;

	PHYSFS_file* fsFile = PHYSFS_openWrite(file);

	if (fsFile != nullptr)
	{
		PHYSFS_sint64 size = strlen(buffer);

		PHYSFS_sint64 written = PHYSFS_writeBytes(fsFile, (const void*)buffer, size);

		if (written == size)
			ret = (uint)written;
		else
			CONSOLE_LOG("File System error while writing to file %s: %s\n", file, PHYSFS_getLastError());

		if (PHYSFS_close(fsFile) == 0)
			CONSOLE_LOG("File System error while closing file %s: %s\n", file, PHYSFS_getLastError());
	}
	else
		CONSOLE_LOG("File System error while opening file %s: %s\n", file, PHYSFS_getLastError());

	return ret;
}

const char* ModuleFileSystem::GetBasePath() const 
{
	return PHYSFS_getBaseDir();
}

const char* ModuleFileSystem::GetReadPaths() const
{
	static char paths[BUF_SIZE];
	paths[0] = '\0'; // null-terminated byte string

	static char tmp_string[BUF_SIZE];

	char** path;
	for (path = PHYSFS_getSearchPath(); *path != nullptr; ++path)
	{
		sprintf_s(tmp_string, BUF_SIZE, "%s\n", *path);
		strcat_s(paths, BUF_SIZE, tmp_string);
	}

	return paths;
}

const char* ModuleFileSystem::GetWritePath() const 
{
	return PHYSFS_getWriteDir();
}

const char* ModuleFileSystem::GetFileNameFromPath(const char* path) const
{
	std::string newPath = path;
	std::string name = newPath;
	name = name.substr(name.find_last_of("\\") + 1, name.size());
	name = name.substr(name.find_last_of("//") + 1, name.size());
	name = name.substr(0, name.find_last_of("."));

	const char* result = new char[name.size() + 1];
	strcpy_s((char*)result, name.size() + 1, name.data());

	return result;
}