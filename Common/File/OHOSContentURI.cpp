#include "Common/File/OHOSContentURI.h"

bool OHOSContentURI::Parse(std::string_view path) {
	const char *prefix = "file://";
	if (!startsWith(path, prefix)) {
		return false;
	}

	std::string_view components = path.substr(strlen(prefix));

    if(startsWith(components, "docs/storage/Users/currentUser")){
        provider = "docs";
        root = UriDecode("storage/Users/currentUser");
        file = components.substr(strlen("docs/storage/Users/currentUser/"));
    } else {
        std::vector<std::string_view> parts;
	    SplitString(components, '/', parts);
        provider = parts[0];
        root = "data/storage/el2/base";
        file = components.substr(strlen("data/storage/el2/base/"));
    }
    return true;
}

OHOSContentURI OHOSContentURI::WithRootFilePath(const std::string &filePath) {
	if (root.empty()) {
		ERROR_LOG(Log::System, "WithRootFilePath cannot be used with single file URIs.");
		return *this;
	}

	OHOSContentURI uri = *this;
	uri.file = uri.root;
	if (!filePath.empty()) {
		uri.file += "/" + filePath;
	}
	return uri;
}

OHOSContentURI OHOSContentURI::WithComponent(std::string_view filePath) {
	OHOSContentURI uri = *this;
	if (uri.file.empty()) {
		// Not sure what to do.
		return uri;
	}
	if (uri.file.back() == ':') {
		// Special case handling for Document URIs: Treat the ':' as a directory separator too (but preserved in the filename).
		uri.file.append(filePath);
	} else {
		uri.file.push_back('/');
		uri.file.append(filePath);
	}
	return uri;
}

OHOSContentURI OHOSContentURI::WithExtraExtension(std::string_view extension) {
	OHOSContentURI uri = *this;
	uri.file.append(extension);
	return uri;
}

OHOSContentURI OHOSContentURI::WithReplacedExtension(const std::string &oldExtension, const std::string &newExtension) const {
	_dbg_assert_(!oldExtension.empty() && oldExtension[0] == '.');
	_dbg_assert_(!newExtension.empty() && newExtension[0] == '.');
	OHOSContentURI uri = *this;
	if (endsWithNoCase(file, oldExtension)) {
		uri.file = file.substr(0, file.size() - oldExtension.size()) + newExtension;
	}
	return uri;
}

OHOSContentURI OHOSContentURI::WithReplacedExtension(const std::string &newExtension) const {
	_dbg_assert_(!newExtension.empty() && newExtension[0] == '.');
	OHOSContentURI uri = *this;
	if (file.empty()) {
		return uri;
	}
	std::string extension = GetFileExtension();
	uri.file = file.substr(0, file.size() - extension.size()) + newExtension;
	return uri;
}

bool OHOSContentURI::CanNavigateUp() const {
	if (IsTreeURI()) {
		return file.size() > root.size();
	} else {
		return file.find(':') != std::string::npos && file.back() != ':';
	}
}

// Only goes downwards in hierarchies. No ".." will ever be generated.
bool OHOSContentURI::ComputePathTo(const OHOSContentURI &other, std::string &path) const {
	size_t offset = FilePath().size() + 1;
	const auto &otherFilePath = other.FilePath();
	if (offset >= otherFilePath.size()) {
		ERROR_LOG(Log::System, "Bad call to PathTo. '%s' -> '%s'", FilePath().c_str(), other.FilePath().c_str());
		return false;
	}

	path = other.FilePath().substr(FilePath().size() + 1);
	return true;
}

std::string OHOSContentURI::GetFileExtension() const {
	size_t pos = file.rfind('.');
	if (pos == std::string::npos) {
		return "";
	}
	size_t slash_pos = file.rfind('/');
	if (slash_pos != std::string::npos && slash_pos > pos) {
		// Don't want to detect "df/file" from "/as.df/file"
		return "";
	}
	std::string ext = file.substr(pos);
	for (size_t i = 0; i < ext.size(); i++) {
		ext[i] = tolower(ext[i]);
	}
	return ext;
}

std::string OHOSContentURI::GetLastPart() const {
	if (file.empty()) {
		// Can't do anything anyway.
		return std::string();
	}

	if (!CanNavigateUp()) {
		size_t colon = file.rfind(':');
		if (colon == std::string::npos) {
			return std::string();
		}
		if (file.back() == ':') {
			return file;
		}
		return file.substr(colon + 1);
	}

	size_t slash = file.rfind('/');
	if (slash == std::string::npos) {
		// ok, look for the final colon. If it's the last char, we would have been caught above in !CanNavigateUp.
		size_t colon = file.rfind(':');
		if (colon == std::string::npos) {
			return std::string();
		}
		return file.substr(colon + 1);
	}

	std::string part = file.substr(slash + 1);
	return part;
}

bool OHOSContentURI::NavigateUp() {
	if (!CanNavigateUp()) {
		return false;
	}

	size_t slash = file.rfind('/');
	if (slash == std::string::npos) {
		// ok, look for the final colon.
		size_t colon = file.rfind(':');
		if (colon == std::string::npos) {
			return false;
		}
		file = file.substr(0, colon + 1);  // Note: we include the colon in these paths.
		return true;
	}

	file = file.substr(0, slash);
	return true;
}


std::string OHOSContentURI::ToString() const {
	if (file.empty()) {
		// Tree URI
		return StringFromFormat("file://%s/%s", provider.c_str(), UriEncode(root).c_str());
	} else if (root.empty()) {
		// Single file URI
		return StringFromFormat("file://%s/%s", provider.c_str(), UriEncode(file).c_str());
	} else {
		// File URI from Tree
		return StringFromFormat("file://%s/%s/%s", provider.c_str(), UriEncode(root).c_str(), UriEncode(file).c_str());
	}
}
