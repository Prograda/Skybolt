#include "ImageFactory.h"

#include <filesystem>
#include <fstream>
#include <istream>
#include <ostream>

namespace skybolt::vis {

Expected<ImagePtr> ImageFactory::readImage(const std::string& filename) const
{
	std::filesystem::path path(filename);
	if (!std::filesystem::exists(path))
	{
		return UnexpectedMessage("File does not exist:'" + filename + "'");
	}

	std::ifstream f(filename, std::ios::binary);
	if (!f.is_open() || f.fail())
	{
		return UnexpectedMessage("File could not be opened :'" + filename + "'");
	}

	return readImage(f, path.extension().string());
}

Expected<std::monostate> ImageFactory::writeImage(const Image& image, const std::string& filename) const
{
	std::filesystem::path path(filename);
	std::ofstream f(filename, std::ios::binary);
	if (!f.is_open() || f.fail())
	{
		return UnexpectedMessage("Failed to open image file '" + filename + "'");
	}

	return writeImage(image, f, path.extension().string());
}

} // namespace skybolt::vis