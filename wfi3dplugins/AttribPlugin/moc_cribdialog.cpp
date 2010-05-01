/****************************************************************************
** CRIBDialog meta object code from reading C++ file 'cribdialog.h'
**
** Created: Thu Jun 14 16:37:27 2001
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "cribdialog.h"
#include <qmetaobject.h>
#include <qapplication.h>



const char *CRIBDialog::className() const
{
    return "CRIBDialog";
}

QMetaObject *CRIBDialog::metaObj = 0;

void CRIBDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("CRIBDialog","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString CRIBDialog::tr(const char* s)
{
    return qApp->translate( "CRIBDialog", s, 0 );
}

QString CRIBDialog::tr(const char* s, const char * c)
{
    return qApp->translate( "CRIBDialog", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* CRIBDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void (CRIBDialog::*m1_t0)();
    typedef void (QObject::*om1_t0)();
    typedef void (CRIBDialog::*m1_t1)();
    typedef void (QObject::*om1_t1)();
    typedef void (CRIBDialog::*m1_t2)();
    typedef void (QObject::*om1_t2)();
    typedef void (CRIBDialog::*m1_t3)();
    typedef void (QObject::*om1_t3)();
    typedef void (CRIBDialog::*m1_t4)();
    typedef void (QObject::*om1_t4)();
    typedef void (CRIBDialog::*m1_t5)();
    typedef void (QObject::*om1_t5)();
    m1_t0 v1_0 = &CRIBDialog::slotHelp;
    om1_t0 ov1_0 = (om1_t0)v1_0;
    m1_t1 v1_1 = &CRIBDialog::slotRendRibBrowse;
    om1_t1 ov1_1 = (om1_t1)v1_1;
    m1_t2 v1_2 = &CRIBDialog::slotEdit;
    om1_t2 ov1_2 = (om1_t2)v1_2;
    m1_t3 v1_3 = &CRIBDialog::slotSave;
    om1_t3 ov1_3 = (om1_t3)v1_3;
    m1_t4 v1_4 = &CRIBDialog::slotSavePathBrowse;
    om1_t4 ov1_4 = (om1_t4)v1_4;
    m1_t5 v1_5 = &CRIBDialog::slotShaderPathBrowse;
    om1_t5 ov1_5 = (om1_t5)v1_5;
    QMetaData *slot_tbl = QMetaObject::new_metadata(6);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(6);
    slot_tbl[0].name = "slotHelp()";
    slot_tbl[0].ptr = (QMember)ov1_0;
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "slotRendRibBrowse()";
    slot_tbl[1].ptr = (QMember)ov1_1;
    slot_tbl_access[1] = QMetaData::Public;
    slot_tbl[2].name = "slotEdit()";
    slot_tbl[2].ptr = (QMember)ov1_2;
    slot_tbl_access[2] = QMetaData::Public;
    slot_tbl[3].name = "slotSave()";
    slot_tbl[3].ptr = (QMember)ov1_3;
    slot_tbl_access[3] = QMetaData::Public;
    slot_tbl[4].name = "slotSavePathBrowse()";
    slot_tbl[4].ptr = (QMember)ov1_4;
    slot_tbl_access[4] = QMetaData::Public;
    slot_tbl[5].name = "slotShaderPathBrowse()";
    slot_tbl[5].ptr = (QMember)ov1_5;
    slot_tbl_access[5] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"CRIBDialog", "QDialog",
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    metaObj->set_slot_access( slot_tbl_access );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    return metaObj;
}
