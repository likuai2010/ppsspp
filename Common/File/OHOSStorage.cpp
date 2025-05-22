#include <inttypes.h>

#include "Common/File/OHOSStorage.h"
#include "Common/StringUtils.h"
#include "Common/Log.h"
#include <fcntl.h>
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
	int fd =  open(path, openMode | O_CLOEXEC);
    free(path);
    return fd;
}

StorageError OHOS_CreateDirectory(const std::string &rootTreeUri, const std::string &dirName) {
	return StorageError::UNKNOWN;
// 	auto env = getEnv();
// 	jstring paramRoot = env->NewStringUTF(rootTreeUri.c_str());
// 	jstring paramDirName = env->NewStringUTF(dirName.c_str());
// 	return StorageErrorFromInt(env->CallIntMethod(g_nativeActivity, contentUriCreateDirectory, paramRoot, paramDirName));
}

StorageError OHOS_CreateFile(const std::string &parentTreeUri, const std::string &fileName) {
	return StorageError::UNKNOWN;
// 	auto env = getEnv();
// 	jstring paramRoot = env->NewStringUTF(parentTreeUri.c_str());
// 	jstring paramFileName = env->NewStringUTF(fileName.c_str());
// 	return StorageErrorFromInt(env->CallIntMethod(g_nativeActivity, contentUriCreateFile, paramRoot, paramFileName));
}

StorageError OHOS_CopyFile(const std::string &fileUri, const std::string &destParentUri) {
	return StorageError::UNKNOWN;
// 	auto env = getEnv();
// 	jstring paramFileName = env->NewStringUTF(fileUri.c_str());
// 	jstring paramDestParentUri = env->NewStringUTF(destParentUri.c_str());
// 	return StorageErrorFromInt(env->CallIntMethod(g_nativeActivity, contentUriCopyFile, paramFileName, paramDestParentUri));
}

StorageError OHOS_MoveFile(const std::string &fileUri, const std::string &srcParentUri, const std::string &destParentUri) {
    return StorageError::UNKNOWN;
// 	auto env = getEnv();
// 	jstring paramFileName = env->NewStringUTF(fileUri.c_str());
// 	jstring paramSrcParentUri = env->NewStringUTF(srcParentUri.c_str());
// 	jstring paramDestParentUri = env->NewStringUTF(destParentUri.c_str());
// 	return StorageErrorFromInt(env->CallIntMethod(g_nativeActivity, contentUriMoveFile, paramFileName, paramSrcParentUri, paramDestParentUri));
}

StorageError OHOS_RemoveFile(const std::string &fileUri) {
	return StorageError::UNKNOWN;
// 	auto env = getEnv();
// 	jstring paramFileName = env->NewStringUTF(fileUri.c_str());
// 	return StorageErrorFromInt(env->CallIntMethod(g_nativeActivity, contentUriRemoveFile, paramFileName));
}

StorageError OHOS_RenameFileTo(const std::string &fileUri, const std::string &newName) {
	return StorageError::UNKNOWN;
// 	auto env = getEnv();
// 	jstring paramFileUri = env->NewStringUTF(fileUri.c_str());
// 	jstring paramNewName = env->NewStringUTF(newName.c_str());
// 	return StorageErrorFromInt(env->CallIntMethod(g_nativeActivity, contentUriRenameFileTo, paramFileUri, paramNewName));
}

// NOTE: Does not set fullName - you're supposed to already know it.
static bool ParseFileInfo(const std::string &line, File::FileInfo *fileInfo) {
	std::vector<std::string> parts;
	SplitString(line, '|', parts);
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
	return false;
// 	auto env = getEnv();
// 	jstring paramFileUri = env->NewStringUTF(fileUri.c_str());
//
// 	jstring str = (jstring)env->CallObjectMethod(g_nativeActivity, contentUriGetFileInfo, paramFileUri);
// 	if (!str) {
// 		return false;
// 	}
// 	const char *charArray = env->GetStringUTFChars(str, 0);
// 	bool retval = ParseFileInfo(std::string(charArray), fileInfo);
// 	fileInfo->fullName = Path(fileUri);
//
// 	env->DeleteLocalRef(str);
// 	return retval && fileInfo->exists;
}

bool OHOS_FileExists(const std::string &fileUri) {
	return false;
// 	auto env = getEnv();
// 	jstring paramFileUri = env->NewStringUTF(fileUri.c_str());
// 	bool exists = env->CallBooleanMethod(g_nativeActivity, contentUriFileExists, paramFileUri);
// 	return exists;
}

std::vector<File::FileInfo> OHOS_ListContentUri(const std::string &path, bool *exists) {
    *exists = false;
    return std::vector<File::FileInfo>();
// 	auto env = getEnv();
// 	*exists = true;
//
// 	double start = time_now_d();
//
// 	jstring param = env->NewStringUTF(path.c_str());
// 	jobject retval = env->CallObjectMethod(g_nativeActivity, listContentUriDir, param);
//
// 	jobjectArray fileList = (jobjectArray)retval;
// 	std::vector<File::FileInfo> items;
// 	int size = env->GetArrayLength(fileList);
// 	for (int i = 0; i < size; i++) {
// 		jstring str = (jstring)env->GetObjectArrayElement(fileList, i);
// 		const char *charArray = env->GetStringUTFChars(str, 0);
// 		if (charArray) {  // paranoia
// 			std::string line = charArray;
// 			File::FileInfo info;
// 			if (line == "X") {
// 				// Indicates an exception thrown, path doesn't exist.
// 				*exists = false;
// 			} else if (ParseFileInfo(line, &info)) {
// 				// We can just reconstruct the URI.
// 				info.fullName = Path(path) / info.name;
// 				items.push_back(info);
// 			}
// 		}
// 		env->ReleaseStringUTFChars(str, charArray);
// 		env->DeleteLocalRef(str);
// 	}
// 	env->DeleteLocalRef(fileList);
//
// 	double elapsed = time_now_d() - start;
// 	double threshold = 0.1;
// 	if (elapsed >= threshold) {
// 		INFO_LOG(Log::FileSystem, "Listing directory on content URI '%s' took %0.3f s (%d files, log threshold = %0.3f)", path.c_str(), elapsed, (int)items.size(), threshold);
// 	}
// 	return items;
}

int64_t OHOS_GetFreeSpaceByContentUri(const std::string &uri) {
	return false;
//
// 	jstring param = env->NewStringUTF(uri.c_str());
// 	return env->CallLongMethod(g_nativeActivity, contentUriGetFreeStorageSpace, param);
}

int64_t OHOS_GetFreeSpaceByFilePath(const std::string &filePath) {
	return false;
// 	auto env = getEnv();
//
// 	jstring param = env->NewStringUTF(filePath.c_str());
// 	return env->CallLongMethod(g_nativeActivity, filePathGetFreeStorageSpace, param);
}

int64_t OHOS_ComputeRecursiveDirectorySize(const std::string &uri) {
	return false;
// 	auto env = getEnv();
//
// 	jstring param = env->NewStringUTF(uri.c_str());
//
// 	double start = time_now_d();
// 	int64_t size = env->CallLongMethod(g_nativeActivity, computeRecursiveDirectorySize, param);
// 	double elapsed = time_now_d() - start;
//
// 	INFO_LOG(IO, "ComputeRecursiveDirectorySize(%s) in %0.3f s", uri.c_str(), elapsed);
// 	return size;
}

bool OHOS_IsExternalStoragePreservedLegacy() {
    return false;
// 	if (!g_nativeActivity) {
//		
// 	}
// 	auto env = getEnv();
// 	return env->CallBooleanMethod(g_nativeActivity, isExternalStoragePreservedLegacy);
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

#else

// These strings should never appear except on OHOS.
// Very hacky.
std::string g_extFilesDir = "(IF YOU SEE THIS THERE'S A BUG)";
std::string g_externalDir = "(IF YOU SEE THIS THERE'S A BUG (2))";
std::string g_nativeLibDir = "(IF YOU SEE THIS THERE'S A BUG (3))";

#endif
