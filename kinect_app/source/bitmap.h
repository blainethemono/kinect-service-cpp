#ifndef __kinect_app_bitmap_h__
#define __kinect_app_bitmap_h__

extern "C"{
#include <freeimage/freeimage.h>
}

#include <Windows.h>
#include <NuiApi.h>

#include <assert.h>

namespace image
{
//lookups for color tinting based on player index
const int g_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
const int g_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
const int g_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

namespace image_types
{
	enum
	{
		Bitmap,
		Gif
	};
	typedef size_t Type;
}

struct Resolution
{
	Resolution(NUI_IMAGE_RESOLUTION resolution)
	{
		::NuiImageResolutionToSize(resolution, width, height);
	}

	DWORD width;
	DWORD height;
};

class Bitmap
{
public:
	explicit Bitmap(NUI_IMAGE_RESOLUTION resolution)
		: m_resolution(resolution)
		, m_rgbWk(m_resolution.width * m_resolution.height)
#ifdef _DEBUG
		, m_converted(0)
#endif
	{
		m_biHdr.biSize = sizeof(BITMAPINFOHEADER);
		m_biHdr.biBitCount = 24;
		m_biHdr.biPlanes = 1;
		m_biHdr.biCompression = BI_RGB;
		m_biHdr.biWidth = m_resolution.width;
		m_biHdr.biHeight = m_resolution.height;
		m_biHdr.biSizeImage = ((((m_biHdr.biWidth * m_biHdr.biBitCount) + 31) & ~31) >> 3) * m_biHdr.biHeight;
		m_biHdr.biXPelsPerMeter = 0;
		m_biHdr.biYPelsPerMeter = 0;
		m_biHdr.biClrUsed = 0;
		m_biHdr.biClrImportant = 0;

		FreeImage_Initialise(); // @todo: Can be called multiple times or not?
	}

	~Bitmap()
	{
		FreeImage_DeInitialise();
	}

	void Write(const NUI_IMAGE_FRAME& frame)
	{
		::Sleep(20);
		return;

		INuiFrameTexture* texture = frame.pFrameTexture;

		NUI_LOCKED_RECT lockedRect;
		HRESULT hr = texture->LockRect(0, &lockedRect, NULL, 0);

		if (lockedRect.Pitch)
		{
			DWORD frameWidth, frameHeight;
        
			::NuiImageResolutionToSize(frame.eResolution, frameWidth, frameHeight);
        
			USHORT* bufferRun = (USHORT*)lockedRect.pBits;
	
			const USHORT* bufferEnd = bufferRun + (frameWidth * frameHeight);
			
			for (size_t y = 0; y < m_resolution.height; ++y)
			{
				for (size_t x = 0; x < m_resolution.width; ++x)
				{
					FillTriple(GetTriple(x, y), *bufferRun);
				
					++bufferRun;
				}
			}
		}
		texture->UnlockRect(0);
	}

	void Save2File(const char* fileName, image_types::Type type)
	{
		switch (type)
		{
		case image_types::Bitmap:
			{
				if (FILE* pFile = fopen(fileName, "wb"))
				{
					BITMAPFILEHEADER bmfh;
					int bitsOffset = sizeof(BITMAPFILEHEADER) + m_biHdr.biSize; 
					LONG imageSize = m_rgbWk.size() * sizeof(RGBTRIPLE);
					LONG fileSize = bitsOffset + imageSize;
					bmfh.bfType = 'B'+('M'<<8);			
					bmfh.bfOffBits = bitsOffset;		
					bmfh.bfSize = fileSize;				
					bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
		
					UINT writtenFileHeaderSize = fwrite(&bmfh, 1, sizeof(BITMAPFILEHEADER), pFile);
					UINT writtenInfoHeaderSize = fwrite(&m_biHdr, 1, sizeof(BITMAPINFOHEADER), pFile);
					UINT writtenDIBDataSize    = fwrite(Data(), sizeof(RGBTRIPLE), m_rgbWk.size(), pFile);
		
					fclose(pFile);
				}
				else
				{
					// @todo:
				}
				break;
			}
		case image_types::Gif:
			{	
				BITMAPFILEHEADER bmfh;
				const int bitsOffset = sizeof(BITMAPFILEHEADER) + m_biHdr.biSize; 
				const LONG imageSize = m_rgbWk.size() * sizeof(RGBTRIPLE);
				const LONG fileSize = bitsOffset + imageSize;
				bmfh.bfType = 'B'+('M'<<8);			
				bmfh.bfOffBits = bitsOffset;		
				bmfh.bfSize = fileSize;				
				bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

				std::vector<BYTE> tmp(fileSize);
				size_t offset(0);

				memcpy(&tmp[offset], &bmfh, sizeof(BITMAPFILEHEADER));
				offset += sizeof(BITMAPFILEHEADER);

				memcpy(&tmp[offset], &m_biHdr, sizeof(BITMAPINFOHEADER));
				offset += sizeof(BITMAPINFOHEADER);
				
				memcpy(&tmp[offset], Data(), imageSize);

				FIMEMORY* in = FreeImage_OpenMemory(&tmp[0], tmp.size());

				if (FIBITMAP* bitmap = FreeImage_LoadFromMemory(FIF_BMP, in))
				{
					if (FIBITMAP* converted = FreeImage_ConvertTo8Bits(bitmap))
					{
						FIMEMORY* out = FreeImage_OpenMemory();

						const BOOL saved = FreeImage_Save(FIF_GIF, converted, fileName);
						if (!saved)
						{
							// @todo:
						}

						FreeImage_CloseMemory(out);

						FreeImage_Unload(converted);
					}

					FreeImage_Unload(bitmap);
				}

				FreeImage_CloseMemory(in);

				break;
			}
		}
	}

	void Convert(image_types::Type type, std::vector<BYTE>& image)
	{
		switch (type)
		{
		case image_types::Gif:
			{	
				BITMAPFILEHEADER bmfh;
				const int bitsOffset = sizeof(BITMAPFILEHEADER) + m_biHdr.biSize; 
				const LONG imageSize = m_rgbWk.size() * sizeof(RGBTRIPLE);
				const LONG fileSize = bitsOffset + imageSize;
				bmfh.bfType = 'B'+('M'<<8);			
				bmfh.bfOffBits = bitsOffset;		
				bmfh.bfSize = fileSize;				
				bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

				std::vector<BYTE> tmp(fileSize);
				size_t offset(0);

				memcpy(&tmp[offset], &bmfh, sizeof(BITMAPFILEHEADER));
				offset += sizeof(BITMAPFILEHEADER);

				memcpy(&tmp[offset], &m_biHdr, sizeof(BITMAPINFOHEADER));
				offset += sizeof(BITMAPINFOHEADER);
				
				memcpy(&tmp[offset], Data(), imageSize);

				FIMEMORY* in = FreeImage_OpenMemory(&tmp[0], tmp.size());

				if (FIBITMAP* bitmap = FreeImage_LoadFromMemory(FIF_BMP, in))
				{
					FIMEMORY* out = FreeImage_OpenMemory();

					if (FIBITMAP* converted = FreeImage_ConvertTo8Bits(bitmap))
					{
						if (BOOL ret = FreeImage_SaveToMemory(FIF_GIF, converted, out))
						{
							if (FreeImage_SeekMemory(out, 0, SEEK_END))
							{
								image.resize(FreeImage_TellMemory(out));

								FreeImage_SeekMemory(out, 0, SEEK_SET);

								FreeImage_ReadMemory(&image[0], image.size(), 1, out);
							}
						}

						FreeImage_Unload(converted);
					}

					FreeImage_CloseMemory(out);
						
					FreeImage_Unload(bitmap);
				}

				FreeImage_CloseMemory(in);

				break;
			}
		default:
			break;
			// @todo: error
		}

#ifdef _DEBUG
		++m_converted;
#endif
	}

private:
	RGBTRIPLE& GetTriple(size_t x, size_t y)
	{
		assert(x < m_resolution.width);
		assert(y < m_resolution.height);

		return m_rgbWk[(m_resolution.height - y - 1) * m_resolution.width + x];
	}
	
	static void FillTriple(RGBTRIPLE& triple, USHORT s)
	{
		const USHORT realDepth = ::NuiDepthPixelToDepth(s);
		const USHORT player    = ::NuiDepthPixelToPlayerIndex(s);

		const BYTE intensity = (BYTE)~(realDepth >> 4);

		triple.rgbtRed   = intensity >> g_IntensityShiftByPlayerR[player];
		triple.rgbtGreen = intensity >> g_IntensityShiftByPlayerG[player];
		triple.rgbtBlue  = intensity >> g_IntensityShiftByPlayerB[player];
	}

	BYTE* Data()
	{
		return reinterpret_cast<BYTE*>(&m_rgbWk[0]);
	}

private:
	const Resolution m_resolution;
	std::vector<RGBTRIPLE> m_rgbWk;
	BITMAPINFOHEADER m_biHdr;

#ifdef _DEBUG
	size_t m_converted;
#endif
};
}

#endif //__kinect_app_bitmap_h__