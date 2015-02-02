/**
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

#include "blackandwhiteeffect.h"

//#include <QDebug>

BlackAndWhiteEffect::BlackAndWhiteEffect(QObject *parent) :
    QObject(parent)
{
}

/*!
  Apply effect to the given source image using the threshold value and write result to destination image.
*/
bool BlackAndWhiteEffect::applyEffect(const QImage &srcImg, QImage &dstImg, const int &thresh)
{
    // Check if in/out images match
    if (srcImg.size() != dstImg.size() || srcImg.format() != dstImg.format())
    {
        return false;
    }

    // Parameters
    int w1 = 76; // (0.299f);
    int w2 = 149; // (0.587f);
    int w3 = 29; // (0.114f);

    int intensity;
    int threshold = thresh;

    // Process image
    uint r,g,b;
    uint *ptrSrc = (uint*)srcImg.bits();
    uint *ptrDst = (uint*)dstImg.bits();
    uint *end = ptrSrc + srcImg.width() * srcImg.height();
    while (ptrSrc != end) {
        // Extract RGB components from the source image pixel
        r = (*ptrSrc&0xff);
        g = (((*ptrSrc)>>8)&0xff);
        b = (((*ptrSrc)>>16)&0xff);

        // Gray (intensity) from RGB
        intensity = ((w1 * r) + (w2 * g) + (w3 * b)) >> 8;

        // Decide between black and white based on threshold
        if (intensity < threshold)
        {
            r = g = b = 0;
        }
        else
        {
            r = g = b = 255;
        }

        // "Mix" rgb values and save to destination image
        *ptrDst = r | (g<<8) | (b<<16) | 0xFF000000;

        // Jump to next pixel
        ptrSrc++;
        ptrDst++;
    }

    return true;
}

