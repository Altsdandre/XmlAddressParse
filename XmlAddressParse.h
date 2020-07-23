#pragma once

#include <QtWidgets>
#include <QXmlStreamReader>
#include "ui_XmlAddressParse.h"
#include <QtWebEngine>
#include <QWebEngineView>

class XmlAddressParse : public QMainWindow
{
    Q_OBJECT

public:
    XmlAddressParse(QWidget *parent = Q_NULLPTR);
    ~XmlAddressParse();

private slots:
    void openXml();
    int  findAdd();
	int  findXQuery();
	void  parseXml();
    void makeShortHousesAtPostal();

protected:
    void keyPressEvent(QKeyEvent *event);
    
private:
    QStringList formAddressList();
    void setFileNames();

private:
    QString m_inputFile;
    QWebEngineView *m_webView;
    QString m_objectName;       // "Object" or "House"
    QString m_regionCode;
    QString m_rootPath;
    QString m_housesNumbersFile;
    QString m_housesNamesFile;
    QString m_postalHousesNumbersFile;
    QString m_city;
    QSettings m_settings;
    int m_requestsToday;
    QDate m_requestsDate;


    Ui::XmlAddressParseClass ui;
};
