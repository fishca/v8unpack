#include "System.IOUtils.hpp"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace System {

namespace Ioutils {


void CreateDir(const String &dirname)
{
	fs::create_directory(fs::path(dirname.c_str()));
}

bool FileExists(const String &filename)
{
	return fs::exists(fs::path(filename.c_str()));
}

void DeleteFile(const String &filename)
{
	fs::remove_all(filename.c_str());
}

void RemoveDir(const String &dirname)
{
	// fs::remove_all(dirname.c_str());
}

namespace TDirectory {

void CreateDirectory(const String &dirname)
{
	CreateDir(dirname);
}

} // TDirectory


namespace TPath {

String GetFullPath(const String &filename)
{
	auto absolute_path = fs::absolute(fs::path(filename.c_str()));
	return String(absolute_path.string());
}

String GetTempPath()
{
	return fs::temp_directory_path().string();
}

#if defined(_MSC_EXTENSIONS)
void GetTempPath(int bufSize, char *buf)
{
	strncpy_s(buf, bufSize, GetTempPath().c_str(), bufSize);
}

#else
void GetTempPath(int bufSize, char *buf)
{
	strncpy(buf, GetTempPath().c_str(), bufSize);
}

#endif


} // TPath

} // Ioutils


} // System
