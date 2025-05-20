#pragma once

#include <vector>
#include <string>
#include <string_view>

#include "Common/File/DirListing.h"

// To emphasize that OHOS storage mode strings are different, let's just use
// an enum.
enum class OHOS_OpenContentUriMode {
	READ = 0,  // "r"
	READ_WRITE = 1,  // "rw"
	READ_WRITE_TRUNCATE = 2,  // "rwt"
};

// Matches the constants in PpssppActivity.java.
enum class StorageError {
	SUCCESS = 0,
	UNKNOWN = -1,
	NOT_FOUND = -2,
	DISK_FULL = -3,
	ALREADY_EXISTS = -4,
};

inline StorageError StorageErrorFromInt(int ival) {
	if (ival >= 0) {
		return StorageError::SUCCESS;
	} else {
		return (StorageError)ival;
	}
}

extern std::string g_extFilesDir;
extern std::string g_externalDir;
extern std::string g_nativeLibDir;

#if PPSSPP_PLATFORM(OHOS) && !defined(__LIBRETRO__)

bool OHOS_IsContentUri(std::string_view uri);
int OHOS_OpenContentUriFd(std::string_view uri, const OHOS_OpenContentUriMode mode);
StorageError OHOS_CreateDirectory(const std::string &parentTreeUri, const std::string &dirName);
StorageError OHOS_CreateFile(const std::string &parentTreeUri, const std::string &fileName);
StorageError OHOS_MoveFile(const std::string &fileUri, const std::string &srcParentUri, const std::string &destParentUri);
StorageError OHOS_CopyFile(const std::string &fileUri, const std::string &destParentUri);

// WARNING: This is very powerful, it will delete directories recursively!
StorageError OHOS_RemoveFile(const std::string &fileUri);

StorageError OHOS_RenameFileTo(const std::string &fileUri, const std::string &newName);
bool OHOS_GetFileInfo(const std::string &fileUri, File::FileInfo *info);
bool OHOS_FileExists(const std::string &fileUri);
int64_t OHOS_ComputeRecursiveDirectorySize(const std::string &fileUri);
int64_t OHOS_GetFreeSpaceByContentUri(const std::string &uri);
int64_t OHOS_GetFreeSpaceByFilePath(const std::string &filePath);
bool OHOS_IsExternalStoragePreservedLegacy();
const char *OHOS_ErrorToString(StorageError error);

std::vector<File::FileInfo> OHOS_ListContentUri(const std::string &uri, bool *exists);


#else

// Stub out the OHOS Storage wrappers, so that we can avoid ifdefs everywhere.

// See comments for the corresponding functions above.

inline bool OHOS_IsContentUri(std::string_view uri) { return false; }
inline int OHOS_OpenContentUriFd(std::string_view uri, const OHOS_OpenContentUriMode mode) { return -1; }
inline StorageError OHOS_CreateDirectory(const std::string &parentTreeUri, const std::string &dirName) { return StorageError::UNKNOWN; }
inline StorageError OHOS_CreateFile(const std::string &parentTreeUri, const std::string &fileName) { return StorageError::UNKNOWN; }
inline StorageError OHOS_MoveFile(const std::string &fileUri, const std::string &srcParentUri, const std::string &destParentUri) { return StorageError::UNKNOWN; }
inline StorageError OHOS_CopyFile(const std::string &fileUri, const std::string &destParentUri) { return StorageError::UNKNOWN; }
inline StorageError OHOS_RemoveFile(const std::string &fileUri) { return StorageError::UNKNOWN; }
inline StorageError OHOS_RenameFileTo(const std::string &fileUri, const std::string &newName) { return StorageError::UNKNOWN; }
inline bool OHOS_GetFileInfo(const std::string &fileUri, File::FileInfo *info) { return false; }
inline bool OHOS_FileExists(const std::string &fileUri) { return false; }
inline int64_t OHOS_ComputeRecursiveDirectorySize(const std::string &fileUri) { return -1; }
inline int64_t OHOS_GetFreeSpaceByContentUri(const std::string &uri) { return -1; }
inline int64_t OHOS_GetFreeSpaceByFilePath(const std::string &filePath) { return -1; }
inline bool OHOS_IsExternalStoragePreservedLegacy() { return false; }
inline const char *OHOS_ErrorToString(StorageError error) { return ""; }
inline std::vector<File::FileInfo> OHOS_ListContentUri(const std::string &uri, bool *exists) {
	*exists = false;
	return std::vector<File::FileInfo>();
}

#endif
