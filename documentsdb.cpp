#include "documentsdb.h"
#include <QString>
#include <QDebug>
#include <QtSql>


//returns new average rating of book
float getNewAverageRating(float newRating, float avgRating, int totalNumOfRatings);

DocumentsDB::DocumentsDB() : MainDB()
{
}

QSqlQuery DocumentsDB::getDocInfoForUID(int id){
    QSqlQuery query;
    //if command runs
    if(!query.exec("SELECT * FROM doc_info WHERE u_id = "+QString::number(id))) {
        qDebug() << "DOCSDB: doc info query for id " << id << " failed.";
    }
    return query;
}

QSqlQuery DocumentsDB::getDocsUploadedByUser(QString username){
    QSqlQuery query;
    //there is no check to see if this command is run
    //I did it this way so it is easier to iterate through using : while (query.next())
    query.exec("SELECT * FROM doc_info WHERE posted_by = '"+username+"' AND is_deleted = 0");
    return query;
}

QSqlQuery DocumentsDB::getAllDocs(){
    QSqlQuery q;
    q.exec("SELECT * FROM doc_info WHERE approved = 3 AND is_deleted = 0");
    return q;
}


//FIX ME: DON't AUTOAPPROVE
void DocumentsDB::addDocument(QString title, QString posted_by, int genre, QString summary, QString askingPrice){
    QSqlQuery query;
    //add document into doc_info DB with some initial values
    if (query.exec("INSERT INTO doc_info(title,posted_by,genre, upload_date, rating, num_of_ratings, views, num_of_complaints, approved, asking_price, counter_offer, summary, is_deleted) "
                   "VALUES ('"+title+"','"+posted_by+"',"+QString::number(genre)+",CURRENT_TIMESTAMP, 0, 0, 0, 0, 3,"+askingPrice+",null," + "'" + summary + "',0)"))
        qDebug()<<"Document added";
    else qDebug() <<"DOCSDB: " << query.lastError();
}

void DocumentsDB::approveDocumentWithUID(int id){
    QSqlQuery q;
    //update approved to true
    if(q.exec("UPDATE doc_info SET approved = 1 WHERE u_id = "+QString::number(id)))
        qDebug()<< "Document has been approved.";
    else qDebug()<<q.lastError();
}

void DocumentsDB::deleteDocumentWithUID(int id){
    QSqlQuery q;
    //delete row where u_id == id
    if(q.exec("UPDATE doc_info SET is_deleted = 1 WHERE u_id = "+QString::number(id)))
        qDebug()<< "Document has been deleted.";
    else qDebug()<<q.lastError();
}

void DocumentsDB::addComplaintToDocumentWithUID(int id){
    QSqlQuery q = this->getDocInfoForUID(id);
    //if doc was found
    if(q.first()){
        //get current number of complaints
        int currentComplaints = q.value(8).toInt();
        //add complaint and update database value
        if(q.exec("UPDATE doc_info SET num_of_complaints = "+QString::number(currentComplaints+1)+" WHERE u_id = "+QString::number(id)))
            qDebug()<< "Complaint recorded.";
        else qDebug()<<q.lastError();
    }
}


void DocumentsDB::addViewToDocWithUID(int id){
    QSqlQuery q = this->getDocInfoForUID(id);
    //if doc exists
    if (q.first()){
        //get current number of views
        int currentViews = q.value(7).toInt();
        //add complaint and update database value
        if(q.exec("UPDATE doc_info SET views = "+QString::number(currentViews+1)+" WHERE u_id = "+QString::number(id)))
            qDebug()<< "Views updated.";
        else qDebug()<<q.lastError();
    }
}


void DocumentsDB::addRatingToDocWithUID(int id, float newRating){
    QSqlQuery q = this->getDocInfoForUID(id);
    //if q exists
    if (q.first()){
        //get current avg rating and total num of ratings
        float avgRating = q.value(5).toFloat();
        int totalNumOfRatings = q.value(6).toInt();

        //update to new rating
        if(q.exec("UPDATE doc_info SET rating = "+QString::number(getNewAverageRating(newRating, avgRating, totalNumOfRatings))+" WHERE u_id = "+QString::number(id)))
            qDebug()<< "Rating updated.";
        else qDebug()<<q.lastError();

        //update total number of ratings made
        if(q.exec("UPDATE doc_info SET num_of_ratings = "+QString::number(totalNumOfRatings+1)+" WHERE u_id = "+QString::number(id)))
            qDebug()<< "Total number of ratings updated.";
        else qDebug()<<q.lastError();
    }
}

//return the new average rating
float getNewAverageRating(float newRating, float avgRating, int totalNumOfRatings){
    float currentTotal = avgRating * totalNumOfRatings + newRating;
    return currentTotal / (totalNumOfRatings +1);
}

//change this if we move the "books" folder somewhere else
QString DocumentsDB::getPathToDocWithUID(int id){
    return "/books/"+QString::number(id)+".txt";
}

//returns -1 if error
int DocumentsDB::getNumDocs() {
    QSqlQuery q;
    if(q.exec("SELECT COUNT(*) FROM doc_info WHERE approved = 3 AND is_deleted = 0")){
        return q.first() ? q.value(0).toInt() : -1;
    }
    else {
        qDebug()<<q.lastError();
        return -1;
    }
}

//returns 0 if nothing in table
int DocumentsDB::getLastInsertRowUID(){
    QSqlQuery q;
    if(q.exec("SELECT last_insert_rowid() AS rowid FROM doc_info")){
        return q.first() ? q.value(0).toInt() : 0;
    }
   else {
        qDebug() << q.lastError();
        return 0;
    }
}

//Get the number of summaries for all books
int DocumentsDB::getnumSumm(QString book_Name)
{
    QSqlQuery q;
    QString char_quot = "'";
    if(q.exec("select count(summary) from doc_info where approved = 3 and is_deleted = 0 and title = "+char_quot+book_Name+char_quot))
    {
        return q.first() ? q.value(0).toInt() : -1;
    }
    else
    {
        qDebug() << q.lastError();
        return -1;
    }
}

QString DocumentsDB::getSummary(QString book_Name)
{
    QSqlQuery query;
    QString char_quot = "'";
    if(query.exec("select summary from doc_info where title = "+char_quot+book_Name+char_quot))
    {
        QString summary;
        while(query.next())
        {
            summary = query.value(0).toString();
        }
        return summary;
    }
    else
    {
        QString blank = "";
        qDebug() << query.lastError();
        return blank;
    }
}
