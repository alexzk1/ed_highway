#include "stylesheetsloader.h" // IWYU pragma: keep

#include "qglobal.h"
#include "qstringliteral.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <QObject>
#include <QScreen>

#include <algorithm>

namespace {

constexpr auto kStylesCompiledPath = ":/style_sheets";
constexpr auto kNotSelectedSheet = "Default";

constexpr auto kBiggerFontMul = 1.5f;

QString FontSizeStyleString(double multiplier = kBiggerFontMul)
{
    const QFont font = qApp->font(); // Текущий шрифт приложения
    const double pointSize = font.pointSizeF();

    if (pointSize <= 0.0)
        return QString(); // Лучше ничего не делать, чем ошибку

    const QScreen *screen = QGuiApplication::primaryScreen();
    const double dpi = screen ? screen->logicalDotsPerInch() : 96.0; // Fallback

    const double pixels = pointSize * multiplier * dpi / 72.0;
    return QString("font-size: %1px;").arg(pixels, 0, 'f', 1); // Один знак после точки
}
} // namespace

StyleSheetsLoader::StyleSheetsLoader()
{
    AppendCompiledInStyleSheets();
    Sort();
}

void StyleSheetsLoader::UpdateComboBoxToLoadStyleSheets(QPointer<QComboBox> comboBox)
{
    if (!comboBox)
    {
        return;
    }

    comboBox->clear();
    comboBox->addItem(QObject::tr(kNotSelectedSheet), ""); // Empty one
    for (const auto &[nameForUser, path] : styleSheetsAvail)
    {
        comboBox->addItem(nameForUser, path);
    }
    AttachHandlerToComboBox(comboBox);
}

void StyleSheetsLoader::AttachHandlerToComboBox(QPointer<QComboBox> comboBox)
{
    if (!comboBox)
    {
        return;
    }
    QObject::connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged),
                     [comboBox](int index) {
                         if (comboBox)
                         {
                             LoadStyleSheet(comboBox->itemData(index).toString());
                         }
                     });
}

void StyleSheetsLoader::UseBiggerFont()
{
    // qApp->setStyleSheet(QString("QWidget{%1}\n").arg(FontSizeStyleString(kBiggerFontMul))
    //                       + qApp->styleSheet());
    auto font = qApp->font();
    font.setPointSizeF(qApp->font().pointSizeF() * kBiggerFontMul);
    qApp->setFont(font);
}

void StyleSheetsLoader::operator()(QStringList &userText, QVariantList &userData) const
{
    userText << QObject::tr(kNotSelectedSheet);
    userData << "";

    for (const auto &[nameForUser, path] : styleSheetsAvail)
    {
        userText << nameForUser;
        userData << path;
    }
}

void StyleSheetsLoader::AppendCompiledInStyleSheets()
{
    // Read resource, list files, add to list.
    const QDir dir(kStylesCompiledPath);
    if (!dir.exists())
    {
        qDebug() << "Directory does not exist: " << dir.path();
        return;
    }

    // Note, sub-folders are not supported.
    const QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files);
    for (const auto &fileInfo : fileInfoList)
    {
        styleSheetsAvail.emplace_back(
          TInfo{fileInfo.baseName(), dir.path() + "/" + fileInfo.fileName()});
    }
}

void StyleSheetsLoader::Sort()
{
    std::sort(styleSheetsAvail.begin(), styleSheetsAvail.end(), [](const TInfo &a, const TInfo &b) {
        return a.nameForUser < b.nameForUser;
    });
}

void StyleSheetsLoader::LoadStyleSheet(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (!path.isEmpty())
        {
            qApp->setStyleSheet(path);
            return;
        }
        qApp->setStyleSheet("");
        return;
    }
    const QByteArray styleSheetData = file.readAll();
    file.close();

    qApp->setStyleSheet(QString::fromUtf8(styleSheetData));
}
