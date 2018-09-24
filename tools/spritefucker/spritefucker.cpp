// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2018 Kiri Jolly
//     http://expiredpopsicle.com
//     expiredpopsicle@gmail.com
//
// ---------------------------------------------------------------------------
//
//   This software is provided 'as-is', without any express or implied
//   warranty. In no event will the authors be held liable for any
//   damages arising from the use of this software.
//
//   Permission is granted to anyone to use this software for any
//   purpose, including commercial applications, and to alter it and
//   redistribute it freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must
//      not claim that you wrote the original software. If you use this
//      software in a product, an acknowledgment in the product
//      documentation would be appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must
//      not be misrepresented as being the original software.
//
//   3. This notice may not be removed or altered from any source
//      distribution.
//
// -------------------------- END HEADER -------------------------------------

#include <lilyengine/utils.h>

#include <vector>
#include <string>

int wrapValue(int i, int max)
{
    i = i % max;
    if(i < 0) {
        i += max;
    }
    return i;
}

void copyRect(
    const ExPop::Gfx::Image *src,
    ExPop::Gfx::Image *dst,
    int src_left,
    int src_top,
    int dst_left,
    int dst_top,
    int width,
    int height)
{
    for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {

            int wrapped_src_x = wrapValue(x + src_left, src->getWidth());
            int wrapped_src_y = wrapValue(y + src_top,  src->getHeight());
            int wrapped_dst_x = wrapValue(x + dst_left, dst->getWidth());
            int wrapped_dst_y = wrapValue(y + dst_top,  dst->getHeight());

            const ExPop::Gfx::Pixel *src_p = src->getPixel(wrapped_src_x, wrapped_src_y);
            ExPop::Gfx::Pixel *dst_p = dst->getPixel(wrapped_dst_x, wrapped_dst_y);

            *dst_p = *src_p;
        }
    }
}

void clearRect(
    ExPop::Gfx::Image *dst,
    int dst_left,
    int dst_top,
    int width,
    int height)
{
    for(int x = 0; x < width; x++) {
        for(int y = 0; y < height; y++) {
            int wrapped_dst_x = wrapValue(x + dst_left, dst->getWidth());
            int wrapped_dst_y = wrapValue(y + dst_top,  dst->getHeight());

            ExPop::Gfx::Pixel *dst_p = dst->getPixel(wrapped_dst_x, wrapped_dst_y);

            dst_p->value = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    std::vector<std::string> inputFiles;
    int globalRadius = -1;
    int globalBorder = -1;
    bool gridOverlay = false;

    ExPop::CommandlineParser cmdParser(argv[0]);

    cmdParser.addHandler<std::string>(
        "", [&inputFiles](const std::string &filename) {
            inputFiles.push_back(filename);
        });
    cmdParser.addVariableHandler("radius", &globalRadius);
    cmdParser.addVariableHandler("border", &globalBorder);
    cmdParser.addFlagHandler("grid", &gridOverlay);

    if(!cmdParser.handleCommandline(argc, argv)) {
        return cmdParser.getErrorFlag();
    }

    for(size_t i = 0; i < inputFiles.size(); i++) {

        ExPop::Gfx::Image *img = ExPop::Gfx::loadTGAFromFile(inputFiles[i]);
        if(!img) {
            std::cerr << "Could not load TGA: " << inputFiles[i] << std::endl;
            return 1;
        }

        if(img->getWidth() != img->getHeight()) {
            std::cerr << "Image not square: " << inputFiles[i] << std::endl;
            return 1;
        }

        std::string filename;
        std::string extension;
        ExPop::stringSplit(inputFiles[i], ".", filename, extension, true);

        int radius = globalRadius;
        if(globalRadius == -1) {
            radius = img->getWidth() / 2;
        }
        int border = globalBorder;
        if(globalBorder == -1) {
            border = img->getWidth() / 8;
        }

        if(radius < border) {
            radius = border;
        }

        ExPop::Gfx::Image *dstImg = new ExPop::Gfx::Image(img->getWidth() * 5, img->getHeight() * 3);

        // '#' shaped thing on the left. One inner tile, four outer
        // angle tiles, and four edge tiles.
        copyRect(
            img, dstImg,
            -radius, -border,
            img->getWidth() - radius, img->getHeight() - border,
            radius * 2 + img->getWidth(), border * 2);

        copyRect(
            img, dstImg,
            -radius, -border,
            img->getWidth() - radius, img->getHeight() * 2 - border,
            radius * 2 + img->getWidth(), border * 2);

        copyRect(
            img, dstImg,
            -border, -radius,
            img->getWidth() - border, img->getHeight() - radius,
            border * 2, radius * 2 + img->getHeight());

        copyRect(
            img, dstImg,
            -border, -radius,
            img->getWidth() - border + img->getWidth(), img->getHeight() - radius,
            border * 2, radius * 2 + img->getHeight());

        // 'o' on the right. Four inner angle tiles.
        copyRect(
            img, dstImg,
            0, 0,
            img->getWidth() * 3, 0,
            img->getWidth() * 2, img->getHeight() * 2);

        clearRect(dstImg,
            img->getWidth() * 3 + border, border,
            img->getWidth() * 2 - 2 * border,
            img->getHeight() * 2 - 2 * border);

        copyRect(
            img, dstImg,
            0, -border,
            img->getWidth() * 3, img->getHeight() + -border,
            img->getWidth() * 2, border * 2);

        copyRect(
            img, dstImg,
            -border, 0,
            img->getWidth() * 4 - border, 0,
            border * 2, img->getHeight() * 2);

        clearRect(dstImg,
            img->getWidth() * 3 + radius, radius,
            img->getWidth() * 2 - 2 * radius,
            img->getHeight() * 2 - 2 * radius);


        if(gridOverlay) {
            for(int y = 0; y < dstImg->getHeight(); y += img->getHeight()) {
                for(int x = 0; x < dstImg->getWidth(); x++) {
                    ExPop::Gfx::Pixel *p = dstImg->getPixel(x, y);
                    p->rgba.r = p->rgba.g = p->rgba.b = (((x&1)^(y&1))?0xff:0x0);
                    p->rgba.a = 255;
                }
            }
            for(int x = 0; x < dstImg->getWidth(); x += img->getWidth()) {
                for(int y = 0; y < dstImg->getHeight(); y++) {
                    ExPop::Gfx::Pixel *p = dstImg->getPixel(x, y);
                    p->rgba.r = p->rgba.g = p->rgba.b = (((x&1)^(y&1))?0xff:0x0);
                    p->rgba.a = 255;
                }
            }
        }


        saveTGAToFile(dstImg, filename + ".template." + extension);

        delete dstImg;
        delete img;
    }

    return 0;
}

