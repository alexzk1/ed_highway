#pragma once

#include <QComboBox>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <vector>

class StyleSheetsLoader
{
  public:
    StyleSheetsLoader();
    void UpdateComboBoxToLoadStyleSheets(QPointer<QComboBox> comboBox);
    static void AttachHandlerToComboBox(QPointer<QComboBox> comboBox);

    static void UseBiggerFont();

    /// @brief Support for GlobalComboBoxStorables. It can be used as items_supplier_t.
    void operator()(QStringList &userText, QVariantList &userData) const;
    static void LoadStyleSheet(const QString &path);

  private:
    struct TInfo
    {
        QString nameForUser;
        QString pathToLoad;
    };
    using list_t = std::vector<TInfo>;

    void AppendCompiledInStyleSheets();
    void Sort();

    list_t styleSheetsAvail{};
};
