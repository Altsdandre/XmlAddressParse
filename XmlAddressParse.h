#pragma once

#include <QtWidgets/QMainWindow>
#include <QXmlStreamReader>
#include "ui_XmlAddressParse.h"

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
    
private:
    void parseElement(QXmlStreamReader *stream, XmlStruct *currentStruct);
	QStringList foo();

private:
    QString m_inputFile;
    XmlStruct xml_struct;

    Ui::XmlAddressParseClass ui;
};
