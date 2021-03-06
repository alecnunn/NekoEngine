#ifndef __MODULE_FILE_SYSTEM_H__
#define __MODULE_FILE_SYSTEM_H__

#include "Module.h"

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(bool start_enabled = true);
	virtual ~ModuleFileSystem();

	bool Init(JSON_Object* jObject);

	bool CleanUp();

	bool AddPath(const char* newDir, const char* mountPoint = nullptr);

	uint OpenRead(const char* file, char** buffer, uint& size) const;
	uint OpenWrite(const char* file, const char* buffer) const;

	const char* GetBasePath() const;
	const char* GetReadPaths() const;
	const char* GetWritePath() const;

	const char* GetFileNameFromPath(const char* path) const;
};

#endif