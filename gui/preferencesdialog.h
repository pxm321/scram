/*
 * Copyright (C) 2017 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <memory>

#include <QDialog>
#include <QSettings>
#include <QTimer>
#include <QUndoStack>

namespace Ui {
class PreferencesDialog;
}

namespace scram {
namespace gui {

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QSettings *preferences, QUndoStack *undoStack,
                               QTimer *autoSaveTimer,
                               QWidget *parent = nullptr);
    ~PreferencesDialog();

private:
    /// Language to locale mapping in the same order as presented in the dialog.
    static const char* const m_languageToLocale[];

    std::unique_ptr<Ui::PreferencesDialog> ui;
};

} // namespace gui
} // namespace scram

#endif // PREFERENCESDIALOG_H
