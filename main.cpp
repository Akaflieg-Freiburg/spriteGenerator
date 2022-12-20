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
#include <QDirIterator>
#include <QImage>
#include <QPainter>
#include <QJsonObject>
#include <QJsonDocument>

auto main(int argc, char *argv[]) -> int
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Akaflieg Freiburg"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("akaflieg_freiburg.de"));
    QCoreApplication::setApplicationName(QStringLiteral("spriteGenerator"));

    // Command line parsing
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Generates sprite sheets for use with MapBox maps"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("[directory]"), QCoreApplication::translate("main", "Directory with SVG files."));
    parser.process(app);
    auto positionalArguments = parser.positionalArguments();

    // Find all relevant SVG files
    QVector<QImage> images;
    QVector<QString> names;
    foreach(auto directory, positionalArguments)
    {
        QDirIterator fileIterator(directory);
        while (fileIterator.hasNext())
        {
            fileIterator.next();
            if (fileIterator.fileName().endsWith("svg", Qt::CaseInsensitive))
            {
                qDebug() << "Loading SVG file" << fileIterator.filePath();
                images << QImage(fileIterator.filePath());
                names << fileIterator.fileName().section(".", 0, -2);
            }
        }
    }

    // Compute number of columns
    auto numImages  = images.length();
    auto numColumns = qCeil( sqrt(numImages) );
    auto numRows    = (numImages+numColumns-1)/numColumns;
    qDebug() << QString("Arranging %1 items in %2 rows and %3 columns.").arg(numImages).arg(numRows).arg(numColumns);

    // Compute Size of sprite sheet
    QVector<int> rowHeight(numRows, 0);
    QVector<int> rowWidth(numRows, 0);
    for(int i=0; i<numImages; i++)
    {
        auto row = i / numColumns;
        rowHeight[row] = qMax(rowHeight[row], images[i].height());
        rowWidth[row] = rowWidth[row] + images[i].width();
    }
    int spriteSheetWidth = 0;
    int spriteSheetHeight = 0;
    for(int i=0; i<numRows; i++)
    {
        spriteSheetWidth = qMax(spriteSheetWidth, rowWidth[i]);
        spriteSheetHeight = spriteSheetHeight + rowHeight[i];
    }
    qDebug() << QString("Generating sprite sheet with (%1,%2)").arg(spriteSheetHeight).arg(spriteSheetWidth);

    // Generate sprite sheet
    QImage spriteSheet(spriteSheetWidth, spriteSheetHeight, QImage::Format_ARGB32);
    QPainter painter(&spriteSheet);
    int xOffset = 0;
    int yOffset = 0;
    QJsonObject obj;
    for(int i=0; i<numImages; i++)
    {
        auto row = i / numColumns;
        auto col = i % numColumns;

        if (col == 0)
        {
            xOffset = 0;
        }

        painter.drawImage(xOffset, yOffset, images[i]);
        QJsonObject jsonObj;
        jsonObj.insert(QStringLiteral("width"), images[i].width());
        jsonObj.insert(QStringLiteral("height"), images[i].height());
        jsonObj.insert(QStringLiteral("x"), xOffset);
        jsonObj.insert(QStringLiteral("y"), yOffset);
        jsonObj.insert(QStringLiteral("pixelRatio"), 1);
        obj.insert(names[i], jsonObj);

        xOffset += images[i].width();
        if (col == numColumns-1)
        {
            yOffset += rowHeight[row];
        }

    }
    painter.end();
    spriteSheet.save("x.png");


    QJsonDocument doc;
    doc.setObject(obj);

    QFile file("x.json");
    file.open(QFile::WriteOnly);
    file.write(doc.toJson());

    return 0;
}
