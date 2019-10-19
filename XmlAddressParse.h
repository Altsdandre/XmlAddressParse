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

private slots:
    void openXml();
    int  findAdd();
	int  findXQuery();
	void  parseXml();
    void makeShortHousesAtPostal();
    
private:
    QStringList foo();

private:
    QString m_inputFile;
    QWebEngineView *m_webView;
    QString m_objectName;       // "Object" or "House"
    QString m_regionCode;

    Ui::XmlAddressParseClass ui;
};
