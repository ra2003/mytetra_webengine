/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MODELMENU_H
#define MODELMENU_H

#include <QtCore/QAbstractItemModel>
#include <QtWidgets/QMenu>

#if QT_VERSION == 0x050600
#include <QObject>
#include <wobjectdefs.h>
#endif

// QT_BEGIN_NAMESPACE

namespace web {
// A QMenu that is dynamically populated from a QAbstractItemModel
class ModelMenu : public QMenu {
#if QT_VERSION == 0x050600
	W_OBJECT(ModelMenu)
#else
	Q_OBJECT
#endif
	signals:
	void activated(const QModelIndex &index)
#if QT_VERSION == 0x050600
	    W_SIGNAL(activated, (const QModelIndex &), index)  //
#else
	    ;
#endif
	    void hovered_signal(const QString &text)
#if QT_VERSION == 0x050600
	        W_SIGNAL(hovered_signal, (const QString &), text)  //
#else
	    ;
#endif

	    public : ModelMenu(QWidget *parent = 0);

	void setModel(QAbstractItemModel *model);
	QAbstractItemModel *model() const;

	void setMaxRows(int max);
	int maxRows() const;

	void setFirstSeparator(int offset);
	int firstSeparator() const;

	void setRootIndex(const QModelIndex &index);
	QModelIndex rootIndex() const;

	void setHoverRole(int role);
	int hoverRole() const;

	void setSeparatorRole(int role);
	int separatorRole() const;

	QAction *makeAction(const QIcon &icon, const QString &text, QObject *parent);

	protected:
	// add any actions before the tree, return true if any actions are added.
	virtual bool prePopulated();
	// add any actions after the tree
	virtual void postPopulated();
	// put all of the children of parent into menu up to max
	void createMenu(const QModelIndex &parent, int max, QMenu *parentMenu = 0, QMenu *menu = 0);

	private slots:
	void aboutToShow();
	void triggered(QAction *action);
	void hovered(QAction *action);

	private:
	QAction *makeAction(const QModelIndex &index);
	int _maxrows;
	int _firstseparator;
	int _maxwidth;
	int _hoverrole;
	int _separatorrole;
	QAbstractItemModel *_model;
	QPersistentModelIndex _root;
};
}

// QT_END_NAMESPACE

#endif  // MODELMENU_H
