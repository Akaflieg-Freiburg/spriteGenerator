/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
 *   stefan.kebekus@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>

auto main(int argc, char *argv[]) -> int
{
    //
    // Application setup
    //
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Akaflieg Freiburg"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("akaflieg_freiburg.de"));
    QCoreApplication::setApplicationName(QStringLiteral("spriteGenerator"));
    QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

    //
    // Command line parsing
    //
    QStringList positionalArguments;
    int pixelRatio {1};
    {
        QCommandLineOption pixelRatioOption(
                    QStringLiteral("pixelRatio"),
                    QStringLiteral("Pixel ratio, to be stored in the JSON file. Defaults to value 1."),
                    QStringLiteral("integer"),
                    QStringLiteral("1"));

        QCommandLineParser parser;
        parser.setApplicationDescription(
                    QStringLiteral("Generates sprite sheets for use with MapBox/MapLibre map styles.\nSee https://github.com/Akaflieg-Freiburg/spriteGenerator for more information."));
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOption(pixelRatioOption);
        parser.addPositionalArgument(
                    QStringLiteral("files"),
                    QStringLiteral("image files"));

        parser.process(app);

        bool ok;
        pixelRatio = parser.value(pixelRatioOption).toInt(&ok);
        positionalArguments = parser.positionalArguments();
        if (positionalArguments.isEmpty() && !ok)
        {
            parser.showHelp();
        }
    }

    //
    // Load all graphic files
    //
    QVector<QImage> images;
    {
        foreach(auto fileName, positionalArguments)
    {
        QImage image;
        if (!image.load(fileName))
        {
            qCritical() << QStringLiteral("Error loading image file %1. Exiting.").arg(fileName);
            exit(0);
        }
        image.setText(QStringLiteral("name"), fileName.section(QStringLiteral("."), 0, -2).section(QStringLiteral("/"), -1));

        images << image;
    }
        if (images.empty())
        {
            qCritical() << "No images loaded. Exiting.";
            exit(0);
        }
    }

    //
    // Compute number of columns and number of rows, and size of sprite sheet
    //
    auto numImages  = images.size();
    auto numColumns = qCeil( sqrt( (double)numImages ) );
    auto numRows    = (numImages+numColumns-1)/numColumns;
    int spriteSheetWidth = 0;
    int spriteSheetHeight = 0;
    {
        QVector<int> rowHeight(numRows, 0);
        QVector<int> rowWidth(numRows, 0);
        for(int i=0; i<numImages; i++)
        {
            auto row = i / numColumns;
            rowHeight[row] = qMax(rowHeight[row], images[i].height());
            rowWidth[row] = rowWidth[row] + images[i].width();
        }
        for(int i=0; i<numRows; i++)
        {
            spriteSheetWidth = qMax(spriteSheetWidth, rowWidth[i]);
            spriteSheetHeight = spriteSheetHeight + rowHeight[i];
        }
    }


    //
    // Generate sprite sheet and JSON
    //
    QImage spriteSheet(spriteSheetWidth, spriteSheetHeight, QImage::Format_ARGB32);
    QJsonObject obj;
    QPainter painter(&spriteSheet);
    int xOffset = 0;
    int yOffset = 0;
    for(int i=0; i<numImages; i++)
    {
        auto image = images[i];

        auto row = i / numColumns;
        auto col = i % numColumns;

        if (col == 0)
        {
            xOffset = 0;
        }

        painter.drawImage(xOffset, yOffset, image);
        QJsonObject jsonObj;
        jsonObj.insert(QStringLiteral("width"), images[i].width());
        jsonObj.insert(QStringLiteral("height"), images[i].height());
        jsonObj.insert(QStringLiteral("x"), xOffset);
        jsonObj.insert(QStringLiteral("y"), yOffset);
        jsonObj.insert(QStringLiteral("pixelRatio"), pixelRatio);
        obj.insert(image.text(QStringLiteral("name")), jsonObj);

        xOffset += image.width();
        if (col == numColumns-1)
        {
            yOffset += rowHeight[row];
        }

    }
    painter.end();
    spriteSheet.save(QStringLiteral("x.png"));


    QJsonDocument doc;
    doc.setObject(obj);

    QFile file(QStringLiteral("x.json"));
    file.open(QFile::WriteOnly);
    file.write(doc.toJson());

    return 0;
}
