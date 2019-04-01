#pragma once

#include <QtWidgets>
#include <QXmlStreamReader>
#include "ui_XmlAddressParse.h"
#include <QtWebEngine>
#include <QWebEngineView>

struct XmlStruct {
    QString name;
    QXmlStreamAttributes attributes;
    QString text;
    QVector<XmlStruct> elements;

    XmlStruct()
    {
        clear();
    }

    void clear()
    {
        name.clear();
        attributes.clear();
        text.clear();
        elements.clear();
    }
};

class XmlAddressParse : public QMainWindow
{
    Q_OBJECT

public:
    XmlAddressParse(QWidget *parent = Q_NULLPTR);

private slots:

    void openXml();
    int  copyXml();
    int  findXml();
	int  findAdd();
	int  findXQuery();
    int  saveXmlStruct();

	void  parseXml();

    void makeShortHousesAtPostal();
    
private:
    void parseElement(QXmlStreamReader *stream, XmlStruct *currentStruct);
	QStringList foo();

private:
    QString m_inputFile;
    XmlStruct xml_struct;
    QWebEngineView *m_webView;

    Ui::XmlAddressParseClass ui;
};
