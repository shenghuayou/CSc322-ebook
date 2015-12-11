#include "superwidget.h"
#include "superuser.h"
#include <QSqlQuery>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include "qtablepushbutton.h"
#include <QSpinBox>
#include "documentsdb.h"
#include <QDebug>
#include <QLabel>

enum {UID, TITLE, USER, REQCRED, APPROVE, DECLINE, COUNTER, COUNTERVAL};

SuperWidget::SuperWidget(SuperUser* user, QWidget* parent) : QWidget(parent)
{
    m_user = user;

    createWidgets();
    createLayouts();
    createActions();
    populateTable();
}

void SuperWidget::createWidgets() {
    m_pending = new QTableWidget();
    m_pending->setColumnCount(COUNTERVAL+1);
    m_pending->setHorizontalHeaderLabels(QStringList() << "UID" << "Title" << "User" << "Cred. Request" << "" << "" << "" << "Counter Value");
    m_pending->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_pending->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void SuperWidget::createLayouts() {
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->addWidget(m_pending);

    setLayout(m_mainLayout);
}

void SuperWidget::createActions() {
}

void SuperWidget::populateTable() {
    clearTable();

    QSqlQuery pending = m_user->getSupersPendingDocuments();
    while(pending.next()) {
        int index = m_pending->rowCount();
        m_pending->insertRow(index);

        // retrieve doc info
        QString uid(pending.value(MainDB::UID).toString());
        QString title(pending.value(MainDB::TITLE).toString());
        QString user(pending.value(MainDB::POSTEDBY).toString());
        QString reqCreds(pending.value(MainDB::ASKINGPRICE).toString());

        QTablePushButton*   approveButton = new QTablePushButton(tr("Accept"), index, APPROVE, this);
        QTablePushButton*   declineButton = new QTablePushButton(tr("Decline"), index, DECLINE, this);
        QTablePushButton*   counterButton = new QTablePushButton(tr("Counter"), index, COUNTER, this);
        QSpinBox*           counterValue = new QSpinBox();

        connect(approveButton, SIGNAL(sendLoc(int,int)),
                this, SLOT(s_buttonClicked(int,int)));
        connect(declineButton, SIGNAL(sendLoc(int,int)),
                this, SLOT(s_buttonClicked(int,int)));
        connect(counterButton, SIGNAL(sendLoc(int,int)),
                this, SLOT(s_buttonClicked(int,int)));


        // insert items to row
        m_pending->setItem(index, UID, new QTableWidgetItem(uid));
        m_pending->setItem(index, TITLE, new QTableWidgetItem(title));
        m_pending->setItem(index, USER, new QTableWidgetItem(user));
        m_pending->setItem(index, REQCRED, new QTableWidgetItem(reqCreds));

        m_pending->setCellWidget(index, APPROVE, approveButton);
        m_pending->setCellWidget(index, DECLINE, declineButton);
        m_pending->setCellWidget(index, COUNTER, counterButton);
        m_pending->setCellWidget(index, COUNTERVAL, counterValue);
    }
}

void SuperWidget::accept(int row)
{
    qDebug() << "Accepted row " << row;
    //afterwards, RU still has to confirm

    int uid = m_pending->item(row, UID)->text().toInt();
    m_giveCredit = m_pending->item(row, REQCRED)->text().toInt();

    RegisteredUser* user = new RegisteredUser(m_pending->item(row, USER)->text());
    user->changeCreditsBy(m_giveCredit);
    delete user;

    //upload the book to the table
    m_user->acceptDocumentWithUID(uid);

    // Get username
    // create registered user
    // change credits by

    // get username
    // update database info for username key credits

}

void SuperWidget::decline(int row)
{
    qDebug() << "Declined row " << row;
    /*
    //use m_user->deleteBookWithUID(int uid) to delete the book
    qDebug() << "Declined row " << row;
    m_title=m_pending->item(row,0)->text();
    m_username=m_pending->item(row,1)->text();
    DocumentsDB *db=new DocumentsDB();
    int m_id=db->getbookID(m_title,m_username,1,0);
    m_user->deleteBookWithUID(m_id);
    */

}

void SuperWidget::counter(int row)
{
    /*
    QSqlQuery q;
    //delete row where u_id == id
    if(q.exec("UPDATE doc_info SET is_deleted = 1 WHERE u_id = "+QString::number(id)))
        qDebug()<< "Document has been deleted.";
    else{
        qDebug()<<"deleteDocumentWitUID: "+QString::number(id)+ " failed";
        qDebug()<<q.lastError();
    }
    */

    QSpinBox* box = dynamic_cast<QSpinBox*>(m_pending->cellWidget(row, COUNTERVAL));
    qDebug() << "Countered row " << row << " for " << box->value();

    int uid = m_pending->item(row, UID)->text().toInt();
    int counterVal = box->value();

    DocumentsDB* db = new DocumentsDB();
    db->modifyCounterForUID(uid, counterVal);


}

void SuperWidget::s_buttonClicked(int row, int col) {
    qDebug() << "Row: " << row << " Col: " << col;

    /*
    m_title=m_pending->item(row,0)->text();
    m_username=m_pending->item(row,1)->text();
    DocumentsDB *db=new DocumentsDB();
    int m_id=db->getbookID(m_title,m_username,1,0);
    m_user->declineDocumentWithUID(m_id,5);
    */

    if(col == APPROVE) {
        accept(row);
    }else if(col == DECLINE) {
        decline(row);
    }else if(col == COUNTER) {
        counter(row);
    }
    clearTable();
    populateTable();
}

void SuperWidget::clearTable()
{
    while (m_pending->rowCount() > 0)
    {
        m_pending->removeRow(0);
    }
}
