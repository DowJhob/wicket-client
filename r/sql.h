#ifndef SQL_H
#define SQL_H

#include <QObject>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QDebug>

class sql
{
public:
    QSqlDatabase db;
    QString    database_addr_server = "10.1.7.10";
    int db_port = 1433;
    QString db_name = "TC_PTA_Check";
    QString db_user = "TC";
    QString db_pass = "tc12452904";

    sql()
    {

    }
    void _register(QString MAC, QString IP)
    {
        logger("<span style=""background-color:#6060b0;""> register MAC - " + MAC + "</span>" );
        if ( !db_open("register MAC") )
            return;
        QSqlQuery query(db);
        QString xml = "<gateIP>"+IP+"</gateIP><gateMAC>"+MAC+"</gateMAC>";
        query.prepare("{ call dbo.usp__RegisterGate( :x ) }");
        query.bindValue(":x", xml, QSql::In );
        if (!query.exec())
            //logger("register - last error - " + query.lastError().text() );
        qDebug() << "register - last error - " + query.lastError().text() ;
    }
    bool check_ticket(QString GateIP, QString barcode, QString direction)
    {
        if ( !db_open("Database error - ") )
            return false;
        QString result(49, ' ');
        QString desc(127, ' ');

        QSqlQuery query(db);
        QString xml = "<gateIP>"+GateIP+"</gateIP><direction>"+direction+"</direction><barcode>"+barcode+"</barcode>";
        query.prepare("{ call dbo.usp__CheckTicketFromGate( :x , :result , :desc ) }");
        query.bindValue(":x", xml, QSql::In );
        query.bindValue(":result", result, QSql::Out);
        query.bindValue(":desc", desc, QSql::Out);
        if (!query.exec())
            logger("check_ticket last error - " + query.lastError().text() );
        result = query.boundValue(":result").toByteArray();
        desc  = query.boundValue(":desc").toByteArray();
        qDebug() << "check_ticket " + barcode + " result - " + result + " :desc - " + desc ;
        if ( result.indexOf("Allow") >= 0 )
            return true;
        return false;
    }
    bool confirm_pass(QString GateIP, QString barcode, QString direction)
    {
        if ( !db_open("confirm_pass error - ") )
            return false;
        QString result(49, ' ');
        QString desc(127, ' ');

        QSqlQuery query(db);
        QString xml = "<gateIP>"+GateIP+"</gateIP><direction>"+direction+"</direction><barcode>"+barcode+"</barcode>";
        query.prepare("{ call dbo.usp__ConfirmPass_light( :x , :result , :desc ) }");
        query.bindValue(":x", xml, QSql::In );
        query.bindValue(":result", result, QSql::InOut);
        query.bindValue(":desc", desc, QSql::InOut);
        if (!query.exec())
            logger("confirm_pass check_ticket last error - " + query.lastError().text() );

        result = query.boundValue(":result").toByteArray();
        desc  = query.boundValue(":desc").toByteArray();

        logger("confirm_pass result - " + result + " :desc - " + desc );

        if ( result.indexOf("Allow") >= 0 )
            return true;
        return false;
    }
    void create_db_connect(QString db_name, int db_port, QString db_user, QString db_pass)
    {
        db = QSqlDatabase::addDatabase("QODBC3");
        db.setDatabaseName( "DRIVER=unixODBC1"
                            "; Server=" + database_addr_server + "; Database=" + db_name + ";CharSet=UTF16; " );
        db.setUserName(db_user);
        db.setPassword(db_pass);
        db.setHostName(database_addr_server);
        db.setPort(db_port);
        db.setConnectOptions("SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_WRITE;SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC4;SQL_ATTR_CONNECTION_TIMEOUT=20;SQL_ATTR_LOGIN_TIMEOUT=20;");
    }
    bool db_open(QString target_error)
    {
        if ( !db.open() )
        {
            logger(target_error + " last error - " + db.lastError().text() );
            return false;
        }
        logger(target_error + " db open:");
        return true;
    }

    void logger(QString g)
    {
qDebug() << g;
    }
};

#endif // SQL_H
