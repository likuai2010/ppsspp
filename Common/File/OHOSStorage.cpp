#include <inttypes.h>

#include "Common/File/OHOSStorage.h"
#include "Common/StringUtils.h"
#include "Common/Log.h"
#include <fcntl.h>
#include <stdio.h>
#if PPSSPP_PLATFORM(OHOS)
#include "ohos/cpp/app-ohos.h"
#include <filemanagement/file_uri/oh_file_uri.h>
#endif

#if PPSSPP_PLATFORM(OHOS) && !defined(__LIBRETRO__)


#include "File/OHOSStorage.h"

bool OHOS_IsContentUri(std::string_view filename) {
	return startsWith(filename, "file://");
}

int OHOS_OpenContentUriFd(std::string fname, OHOS_OpenContentUriMode mode) {
    char * path;
    OH_FileUri_GetPathFromUri(fname.c_str(), fname.size(), &path);
    int openMode = 0;
    switch (mode) {
    	case OHOS_OpenContentUriMode::READ: openMode = O_RDONLY ; break;
    	case OHOS_OpenContentUriMode::READ_WRITE: openMode = O_RDWR ; break;
    	case OHOS_OpenContentUriMode::READ_WRITE_TRUNCATE: openMode = O_RDWR | O_CREAT | O_TRUNC ; break;
    }
	int fd = open(path, openMode | O_CLOEXEC);
    free(path);
    return fd;
}

StorageError OHOS_CreateDirectory(const std::string &rootTreeUri, const std::string &dirName) {
	return StorageError::UNKNOWN;
}

StorageError OHOS_CreateFile(const std::string &parentTreeUri, const std::string &fileName) {
	return StorageError::UNKNOWN;
}

StorageError OHOS_CopyFile(const std::string &fileUri, const std::string &destParentUri) {
	return StorageError::UNKNOWN;
}

StorageError OHOS_MoveFile(const std::string &fileUri, const std::string &srcParentUri, const std::string &destParentUri) {
    return StorageError::UNKNOWN;
}

StorageError OHOS_RemoveFile(const std::string &fileUri) {
	return StorageError::UNKNOWN;
}

StorageError OHOS_RenameFileTo(const std::string &fileUri, const std::string &newName) {
	return StorageError::UNKNOWN;
}

// NOTE: Does not set fullName - you're supposed to already know it.
static bool ParseFileInfo(const std::string &line, File::FileInfo *fileInfo) {
	std::vector<std::string> parts;
	SplitString(line, '/', parts);
	if (parts.size() != 4) {
		ERROR_LOG(Log::FileSystem, "Bad format: %s", line.c_str());
		return false;
	}
	fileInfo->name = std::string(parts[2]);
	fileInfo->isDirectory = parts[0][0] == 'D';
	fileInfo->exists = true;
	sscanf(parts[1].c_str(), "%" PRIu64, &fileInfo->size);
	fileInfo->isWritable = true;  // TODO: Should be passed as part of the string.
	// TODO: For read-only mappings, reflect that here, similarly as with isWritable.
	// Directories are normally executable (0111) which means they're traversable.
	fileInfo->access = fileInfo->isDirectory ? 0777 : 0666;

	uint64_t lastModifiedMs = 0;
	sscanf(parts[3].c_str(), "%" PRIu64, &lastModifiedMs);

	// Convert from milliseconds
	uint32_t lastModified = lastModifiedMs / 1000;

	// We don't have better information, so let's just spam lastModified into all the date/time fields.
	fileInfo->mtime = lastModified;
	fileInfo->ctime = lastModified;
	fileInfo->atime = lastModified;
	return true;
}

bool OHOS_GetFileInfo(const std::string &fileUri, File::FileInfo *fileInfo) {
	char * path;
    OH_FileUri_GetPathFromUri(fileUri.c_str(), fileUri.size(), &path);

	ParseFileInfo(path, fileInfo);
	free(path);
	return fileInfo->exists;
}

bool OHOS_FileExists(const std::string &fileUri) {
  	char * path;
    OH_FileUri_GetPathFromUri(fileUri.c_str(), fileUri.size(), &path);
	int fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd > 0) {
		free(path);
		return true;
	} else {
		return false;
	}
}

std::vector<File::FileInfo> OHOS_ListContentUri(const std::string &path, bool *exists) {
    *exists = false;
    return std::vector<File::FileInfo>();
}

int64_t OHOS_GetFreeSpaceByContentUri(const std::string &uri) {
	return false;
}

int64_t OHOS_GetFreeSpaceByFilePath(const std::string &filePath) {
	return false;
}

int64_t OHOS_ComputeRecursiveDirectorySize(const std::string &uri) {
	return false;

}

bool OHOS_IsExternalStoragePreservedLegacy() {
    return false;
}

const char *OHOS_ErrorToString(StorageError error) {
	switch (error) {
	case StorageError::SUCCESS: return "SUCCESS";
	case StorageError::UNKNOWN: return "UNKNOWN";
	case StorageError::NOT_FOUND: return "NOT_FOUND";
	case StorageError::DISK_FULL: return "DISK_FULL";
	case StorageError::ALREADY_EXISTS: return "ALREADY_EXISTS";
	default: return "(UNKNOWN)";
	}
}
#endif
