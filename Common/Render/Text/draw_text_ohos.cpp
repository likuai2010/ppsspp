#include "ppsspp_config.h"

#include "Common/Log.h"
#include "Common/GPU/thin3d.h"
#include "Common/Data/Hash/Hash.h"
#include "Common/Render/Text/draw_text.h"

#if PPSSPP_PLATFORM(OHOS) && !defined(__LIBRETRO__)
#include "Common/Render/Text/draw_text_ohos.h"
#include "Common/Render/Text/draw_text_ohos.h"
#include <native_drawing/drawing_bitmap.h>
#include <native_drawing/drawing_brush.h>
#include <native_drawing/drawing_canvas.h>
#include <native_drawing/drawing_font.h>
#include <native_drawing/drawing_rect.h>
#include <native_drawing/drawing_text_blob.h>


using Point = std::pair<int, int>;
Point MeasureLine(std::string str, float textScale){
    OH_Drawing_Font *font = OH_Drawing_FontCreate();
    OH_Drawing_FontSetTextSize(font, textScale);
    OH_Drawing_TextBlob *textBlob =
    OH_Drawing_TextBlobCreateFromString(str.c_str(), font, TEXT_ENCODING_UTF8);
    OH_Drawing_Rect* rect = OH_Drawing_RectCreate(0, 0, 0, 0);
    OH_Drawing_TextBlobGetBounds(textBlob, rect);
    int right = OH_Drawing_RectGetRight(rect);
    int top = OH_Drawing_RectGetTop(rect);
    OH_Drawing_TextBlobDestroy(textBlob);
    OH_Drawing_FontDestroy(font);
    OH_Drawing_RectDestroy(rect);
    return { right, -top + 10  };
}
std::vector<std::string> SplitString(std::string text){
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    std::vector<std::string> lines;
    size_t start = 0;
    size_t end = text.find('\n');
    while (end != std::string::npos) {
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
        end = text.find('\n', start);
    }
    lines.push_back(text.substr(start));
    return lines;
}
Point MeasureText(std::string text, float textSize){
    std::vector<std::string> lines = SplitString(text);
    Point total{0, 0};
    for (const auto& line : lines) {
        Point sz = MeasureLine(line, textSize);
        total.first = std::max(sz.first, total.first); // x = max width
        total.second = total.second + sz.second;
    }
    total.second += 2;
      // 限制最小和最大尺寸
    if (total.first < 1) total.first = 1;
    if (total.second < 1) total.second = 1;
    if (total.first > 4096) total.first = 4096;
    if (total.second > 4096) total.second = 4096;
	// Round width up to even already here to avoid annoyances from odd-width 16-bit textures
	// which OpenGL does not like - each line must be 4-byte aligned
	total.first = (total.first + 5) & ~1;
    return total;
}
int* RenderString(Point wh, std::string str, float textSize) {
    int width = wh.first;
    int height = wh.second;
    OH_Drawing_Bitmap* cBitmap_ = OH_Drawing_BitmapCreate();
    OH_Drawing_BitmapFormat cFormat {COLOR_FORMAT_RGBA_8888, ALPHA_FORMAT_OPAQUE};
    OH_Drawing_BitmapBuild(cBitmap_, width, height, &cFormat);
    OH_Drawing_Canvas* cCanvas_ = OH_Drawing_CanvasCreate();
    OH_Drawing_CanvasBind(cCanvas_, cBitmap_);
    
    OH_Drawing_Brush *brush = OH_Drawing_BrushCreate();
    OH_Drawing_BrushSetColor(brush, 0);
    OH_Drawing_CanvasAttachBrush(cCanvas_, brush);
    OH_Drawing_Rect *rect = OH_Drawing_RectCreate(0, 0, width, height);
    OH_Drawing_CanvasDrawRect(cCanvas_, rect);
    OH_Drawing_CanvasDetachBrush(cCanvas_);
   
    float y = 1.0f;
    std::vector<std::string> lines = SplitString(str);
    for (const auto& line : lines) {
        OH_Drawing_Font *font = OH_Drawing_FontCreate();
        OH_Drawing_FontSetTextSize(font, textSize);
        OH_Drawing_TextBlob *textBlob =
        OH_Drawing_TextBlobCreateFromString(line.c_str(), font, TEXT_ENCODING_UTF8);
        OH_Drawing_Rect* rect = OH_Drawing_RectCreate(0, 0, 0, 0);
        OH_Drawing_TextBlobGetBounds(textBlob, rect);
        int top = OH_Drawing_RectGetTop(rect);
        y += -top;
        OH_Drawing_CanvasDrawTextBlob(cCanvas_, textBlob, 0, y);
        OH_Drawing_TextBlobDestroy(textBlob);
        OH_Drawing_FontDestroy(font);
        OH_Drawing_RectDestroy(rect);
    }
  	
    void *bitmapAddr = OH_Drawing_BitmapGetPixels(cBitmap_);
	if (bitmapAddr == nullptr)
		return nullptr;
    uint32_t *value = static_cast<uint32_t *>(bitmapAddr);
    int *dest = (int *)malloc(height * width * sizeof(uint32_t));
    for (int y = 0; y < width; ++y) {
        for (int x = 0; x < height; ++x) {
            int srcPos = y * height + x;
            dest[srcPos] = value[srcPos];
        }
    }
	OH_Drawing_CanvasDestroy(cCanvas_);
	OH_Drawing_BitmapDestroy(cBitmap_);
   	return dest;
}

TextDrawerOHOS::TextDrawerOHOS(Draw::DrawContext *draw) : TextDrawer(draw) {
	
	dpiScale_ = CalculateDPIScale();
	INFO_LOG(Log::G3D, "Initializing TextDrawerOHOS with DPI scale %f", dpiScale_);
}

TextDrawerOHOS::~TextDrawerOHOS() {
	// Not sure why we can't do this but it crashes. Likely some deeper threading issue.
	// At worst we leak one ref...
	// env_->DeleteGlobalRef(cls_textRenderer);
	ClearCache();
	fontMap_.clear();  // size is precomputed using dpiScale_.
}

bool TextDrawerOHOS::IsReady() const {
	return true;
}

uint32_t TextDrawerOHOS::SetFont(const char *fontName, int size, int flags) {
	// We will only use the default font but just for consistency let's still involve
	// the font name.
	uint32_t fontHash = fontName ? hash::Adler32((const uint8_t *)fontName, strlen(fontName)) : 1337;
	fontHash ^= size;
	fontHash ^= flags << 10;

	auto iter = fontMap_.find(fontHash);
	if (iter != fontMap_.end()) {
		fontHash_ = fontHash;
		return fontHash;
	}

	// Just chose a factor that looks good, don't know what unit size is in anyway.
	OHOSFontEntry entry{};
	entry.size = ((float)size * 1.4f) / dpiScale_;
	fontMap_[fontHash] = entry;
	fontHash_ = fontHash;
	return fontHash;
}

void TextDrawerOHOS::SetFont(uint32_t fontHandle) {
	uint32_t fontHash = fontHandle;
	auto iter = fontMap_.find(fontHash);
	if (iter != fontMap_.end()) {
		fontHash_ = fontHandle;
	} else {
		ERROR_LOG(Log::G3D, "Invalid font handle %08x", fontHandle);
	}
}

void TextDrawerOHOS::MeasureStringInternal(std::string_view str, float *w, float *h) {
	float scaledSize = 14;
	auto iter = fontMap_.find(fontHash_);
	if (iter != fontMap_.end()) {
		scaledSize = iter->second.size;
	} else {
		ERROR_LOG(Log::G3D, "Missing font");
	}
	   std::string text(str);
	Point p = MeasureText(text,scaledSize);
	*w = (float)(p.first);
	*h = (float)(p.second & 0xFFFF);
	// WARN_LOG(Log::G3D, "Measure Modified: '%.*s' size: %fx%f", (int)text.length(), text.data(), *w, *h);
}

bool TextDrawerOHOS::DrawStringBitmap(std::vector<uint8_t> &bitmapData, TextStringEntry &entry, Draw::DataFormat texFormat, std::string_view str, int align, bool fullColor) {
	if (str.empty()) {
		bitmapData.clear();
		return false;
	}

	float size = 0.0f;
	auto iter = fontMap_.find(fontHash_);
	if (iter != fontMap_.end()) {
		size = iter->second.size;
	} else {
		ERROR_LOG(Log::G3D, "Missing font");
	}


	std::string text(str);
	Point textSize = MeasureText(text, size);
	int imageWidth = (short)(textSize.first);
	int imageHeight = (short)(textSize.second & 0xFFFF);
	if (imageWidth <= 0)
		imageWidth = 1;
	if (imageHeight <= 0)
		imageHeight = 1;
	int* pixels = RenderString(textSize, text, size);
	if (pixels == nullptr)
		return false;
	entry.texture = nullptr;
	entry.bmWidth = imageWidth;
	entry.width = imageWidth;
	entry.bmHeight = imageHeight;
	entry.height = imageHeight;
	entry.lastUsedFrame = frameCount_;

	if (texFormat == Draw::DataFormat::B4G4R4A4_UNORM_PACK16 || texFormat == Draw::DataFormat::R4G4B4A4_UNORM_PACK16) {
		bitmapData.resize(entry.bmWidth * entry.bmHeight * sizeof(uint16_t));
		uint16_t *bitmapData16 = (uint16_t *)&bitmapData[0];
		for (int y = 0; y < entry.bmHeight; y++) {
			for (int x = 0; x < entry.bmWidth; x++) {
				uint32_t v = pixels[imageWidth * y + x];
				v = 0xFFF0 | ((v >> 28) & 0xF);  // Grab the upper bits from the alpha channel, and put directly in the 16-bit alpha channel.
				bitmapData16[entry.bmWidth * y + x] = (uint16_t)v;
			}
		}
	} else if (texFormat == Draw::DataFormat::R8_UNORM) {
		bitmapData.resize(entry.bmWidth * entry.bmHeight);
		for (int y = 0; y < entry.bmHeight; y++) {
			for (int x = 0; x < entry.bmWidth; x++) {
				uint32_t v = pixels[imageWidth * y + x];
				bitmapData[entry.bmWidth * y + x] = (uint8_t)(v >> 24);
			}
		}
	} else if (texFormat == Draw::DataFormat::R8G8B8A8_UNORM) {
		bitmapData.resize(entry.bmWidth * entry.bmHeight * sizeof(uint32_t));
		uint32_t *bitmapData32 = (uint32_t *)&bitmapData[0];
		for (int y = 0; y < entry.bmHeight; y++) {
			for (int x = 0; x < entry.bmWidth; x++) {
				uint32_t v = pixels[imageWidth * y + x];
				// Swap R and B, for some reason.
				v = (v & 0xFF00FF00) | ((v >> 16) & 0xFF) | ((v << 16) & 0xFF0000);
				bitmapData32[entry.bmWidth * y + x] = v;
			}
		}
	} else {
		_assert_msg_(false, "Bad TextDrawer format");
	}
	free(pixels);
	return true;
}

void TextDrawerOHOS::ClearFonts() {
	fontMap_.clear();  // size is precomputed using dpiScale_.
}

#endif
