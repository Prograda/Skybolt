#include "HeightMapHelpers.h"
#include "SkyboltVis/Image/Image.h"

namespace skybolt::vis {

bool isHeightMapDataFormat(const Image& image)
{
	return image.getFormat() == Image::Format::R16;
}

} // namespace skybolt::vis