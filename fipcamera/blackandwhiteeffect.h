/**
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

#ifndef BLACKANDWHITEEFFECT_H
#define BLACKANDWHITEEFFECT_H

#include <QObject>
#include <QImage>

/*
 * A simple black and white effect.
 */

class BlackAndWhiteEffect : public QObject
{
    Q_OBJECT
public:
    explicit BlackAndWhiteEffect(QObject *parent = 0);

    bool applyEffect(const QImage &srcImg, QImage &dstImg, const int &thresh);

Q_SIGNALS:
    
public Q_SLOTS:

};

#endif // BLACKANDWHITEEFFECT_H
