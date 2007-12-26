/***************************************************************************
                         effectlistdialog.cpp  -  description
                            -------------------
   begin                : Sun Feb 9 2003
   copyright            : (C) 2003 by Jason Wood
   email                : jasonwood@blueyonder.co.uk
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qtooltip.h>
#include <qdir.h>

#include <kdebug.h>
#include <klocale.h>
#include <klistbox.h>
#include <ktextedit.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include "effectdrag.h"
#include "effectparamdesc.h"
#include "effectlistdialog.h"

#include "krender.h"

namespace Gui {

    EffectListDialog::EffectListDialog(const QPtrList < EffectDesc >
	&effectList, QWidget * parent, const char *name):QWidget(parent,
	name) {
	QFont dialogFont = font();
	dialogFont.setPointSize(dialogFont.pointSize() - 1);
	setFont(dialogFont);

	m_effectList.setAutoDelete(false);
	QVBoxLayout *viewLayout = new QVBoxLayout( this );
        viewLayout->setAutoAdd( TRUE );
	KIconLoader loader;
	
	m_effectListBox = new EffectList_UI(this);

	m_effectListBox->button_delete->setIconSet(QIconSet(loader.loadIcon("editdelete",
		    KIcon::Toolbar)));

	QToolTip::add(m_effectListBox->button_delete, i18n("Delete custom effect"));

	setEffectList(effectList);

	connect(m_effectListBox->effect_list, SIGNAL(doubleClicked (QListBoxItem *, const QPoint &)), this, SLOT(slotAddEffect(QListBoxItem *)));

	connect(m_effectListBox->effect_list, SIGNAL(selectionChanged ()), this, SLOT(slotEffectSelected()));

	connect(m_effectListBox->button_group, SIGNAL(clicked (int)), this, SLOT(generateLayout()));

	connect(m_effectListBox->button_delete, SIGNAL(clicked()), this, SLOT(slotDeleteEffect()));



	
    } 
    
    EffectListDialog::~EffectListDialog() {
	delete m_effectListBox;
    }

/** Generates the layout for this widget. */
    void EffectListDialog::generateLayout() {
	m_effectListBox->effect_list->clear();
	EFFECTTYPE type = VIDEOEFFECT;
	m_effectListBox->button_delete->setEnabled(false);
	if (m_effectListBox->audio_button->isChecked()) type = AUDIOEFFECT;
	else if (m_effectListBox->custom_button->isChecked()) {
	    m_effectListBox->button_delete->setEnabled(true);
	    type = CUSTOMEFFECT;
	}

	QPtrListIterator < EffectDesc > itt(m_effectList);
	while (itt.current()) {
	    QString effectName = itt.current()->name();
	    if (effectName.startsWith("_")) {
		//Effect is a group
		effectName = effectName.section("_", 1,1) + i18n("[group]");
	    }
	    if (itt.current()->type() == type) {
		if (m_effectListBox->effect_list->findItem(effectName)) 
		    kdWarning()<<"//  DUPLICATE EFFECT ENTRY: "<<effectName<<endl;
		else m_effectListBox->effect_list->insertItem(effectName);
	    }
	    ++itt;
	}
    }

/** Delete a custom effect. */
    void EffectListDialog::slotDeleteEffect() {
	QString direc = locateLocal("appdata", "effects/");

	bool isGroup = false;
	QString itemToDelete;
	if (m_effectListBox->effect_list->currentText().endsWith(i18n("[group]"))) {
	    isGroup = true;
	    itemToDelete = m_effectListBox->effect_list->currentText().section("[", 0, 0);
	}
	else itemToDelete = m_effectListBox->effect_list->currentText();
	QDomDocument doc;
	QString docName;
        QStringList::Iterator it;
        QStringList fileList;

	QDir directory(direc);
	fileList = directory.entryList( QDir::Files );
	for ( it = fileList.begin() ; it != fileList.end() ; ++it ){
	    QString itemName = KURL(direc + *it).path();
	    QFile file(itemName);
	    doc.setContent(&file, false);
	    file.close();
	    QDomElement documentElement = doc.documentElement();
	    if (isGroup) docName = doc.elementsByTagName("effectgroup").item(0).toElement().attribute("name");
	    else docName = doc.elementsByTagName("name").item(0).toElement().text();
	    if (docName == itemToDelete) {
		kdWarning()<<"///// / / / FOUND ITEM TO DELETE: "<<itemName<<endl;
		if (KMessageBox::questionYesNo(this, i18n("<qt>Delete custom effect <b>%1</b> saved in:<br />%2</qt>").arg(itemToDelete).arg(itemName)) == KMessageBox::Yes) 
		    file.remove(itemName);
		break;
	    }
	}
	emit refreshEffects();
    }


/** Set the effect list displayed by this dialog. */
    void EffectListDialog::setEffectList(const QPtrList < EffectDesc >
	&effectList) {
	m_effectList = effectList;
	generateLayout();
    }

    void EffectListDialog::slotAddEffect(QListBoxItem * item) {
	if (item->text().endsWith(i18n("[group]"))) {
	    // A group is selected, add all its effects
	    QString groupName = item->text().section("[", 0, 0);
	    kdWarning()<<"//////  ADDING GRUOP: "<<groupName<<endl;
	    QPtrListIterator < EffectDesc > itt(m_effectList);

	    while (itt.current()) {
	    	if (itt.current()->name().startsWith("_" + groupName + "_")) {
		    emit effectSelected(itt.current()->name(), groupName);
	    	}
	        ++itt;
	    }

	}
	else emit effectSelected(item->text());
    }

    void EffectListDialog::slotEffectSelected() {
	EffectDesc *desc = findDescription(m_effectListBox->effect_list->currentText());
	if (desc == 0) {
	    // Selected effect is a group
	    m_effectListBox->effect_text->setText(i18n("<b>Effect group</b>"));
	    return;
	}
	QString description;
	QString fullDescription = desc->description();
	if (!fullDescription.isEmpty()) {
	    description.append("<b>Description:</b><br />");
	    description.append(i18n(fullDescription));
	}
	QString author = desc->author();
	if (!author.isEmpty()) {
	    description.append("<b>Author:</b><br />");
	    description.append(author);
	}

	description.append("<b>Parameters:</b><br />");
	int num = desc->numParameters();
	int ix;
	for (ix = 0; ix < num; ix++) {
	    if ( ix > 0 ) description.append(", ");
	    description.append(i18n(desc->parameter(ix)->description()));
	}
	m_effectListBox->effect_text->setText(description);
    }

/** returns a drag object which is used for drag operations. */
    QDragObject *EffectListDialog::dragObject() {
	QListBoxItem *selected = m_effectListBox->effect_list->selectedItem();

	kdWarning() << "Returning appropriate dragObejct" << endl;

	EffectDesc *desc = findDescription(selected->text());

	if (!desc) {
	    kdWarning() << "no selected item in effect list" << endl;
	    return 0;
	}

	Effect *effect = desc->createEffect();
	return new EffectDrag(effect, this, "drag object");
    }

    EffectDesc *EffectListDialog::findDescription(const QString & name) {
	EffectDesc *desc = 0;

	QPtrListIterator < EffectDesc > itt(m_effectList);

	while (itt.current()) {
	    if (itt.current()->name() == name) {
		desc = itt.current();
		break;
	    }
	    ++itt;
	}

	return desc;
    }

}				// namespace Gui
