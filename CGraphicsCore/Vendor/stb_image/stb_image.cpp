#include "CGRpch.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstring>
#include <algorithm>

namespace stbUtils
{
    void FlipImageHorizontally(unsigned char* data, int width, int height, int channels)
    {
        for (int y = 0; y < height; ++y)
        {
            unsigned char* row = data + y * width * channels;
            for (int x = 0; x < width / 2; ++x)
            {
                for (int c = 0; c < channels; ++c)
                {
                    std::swap(row[x * channels + c], row[(width - 1 - x) * channels + c]);
                }
            }
        }
    }
    void RotateImage90(unsigned char* data, int width, int height, int channels, bool clockwise)
    {
        int newWidth = height;
        int newHeight = width;
        size_t newSize = newWidth * newHeight * channels;

        unsigned char* temp = new unsigned char[newSize];

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                for (int c = 0; c < channels; ++c)
                {
                    int srcIndex = (y * width + x) * channels + c;
                    int dstIndex;

                    if (clockwise)
                    {
                        dstIndex = (x * newWidth + (newWidth - 1 - y)) * channels + c;
                    }
                    else
                    {
                        dstIndex = ((newHeight - 1 - x) * newWidth + y) * channels + c;
                    }

                    temp[dstIndex] = data[srcIndex];
                }
            }
        }

        // Copy back to original buffer if same size
        if (width == newHeight && height == newWidth)
        {
            std::memcpy(data, temp, newSize);
        }
        else
        {
            // Otherwise, user must handle reallocation
            std::memcpy(data, temp, std::min(newSize, (size_t)(width * height * channels)));
        }

        std::swap(width, height);
        delete[] temp;
    }
}