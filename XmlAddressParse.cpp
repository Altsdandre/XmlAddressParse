#include "XmlAddressParse.h"
#include <qvalidator.h>
#include <qfile.h>

#include <QXmlStreamWriter>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QXmlQuery>
#include <QXmlSerializer>
#include <QSet>
#include <QMap>

struct check_struct {
    QString name;
    QString action;
    QString temp_delay;

    check_struct()
    {
        clear();
    }

    void clear()
    {
        name.clear();
        action.clear();
        temp_delay.clear();
    }
};

XmlAddressParse::XmlAddressParse(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    QRegExp rx("\\d+");

    //QString temp_html;
    //    //QFile linksFile(QDir::currentPath().append("/Include/Program_Files/tenderMainPage2.txt"));
    //    QFile file("ya_page.html");
    //    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    //        return;

    //    QByteArray ar = file.readAll();
        //temp_html = ar.tos
    //    QWebEnginePage *page = new QWebEnginePage(this);
    //     page->setHtml(ar);

    m_webView = new QWebEngineView(this);
    ui.mapScroll->setWidget(m_webView);
    //m_webView->load(QUrl("http://www.yandex.ru"));
    m_webView->load(QUrl::fromLocalFile("/Users/dmitriy/Projects/QtProjects/XmlAddressParse/XmlAddressParse/ya_page.html"));

    QRegExpValidator *validator = new QRegExpValidator(rx);
    ui.copyEdit->setValidator(validator);

    m_inputFile = ui.openEdit->text();

    connect(ui.openButton, SIGNAL(clicked()), this, SLOT(openXml()));
    connect(ui.copyButton, SIGNAL(clicked()), this, SLOT(copyXml()));
    connect(ui.findButton, SIGNAL(clicked()), this, SLOT(findXml()));
    connect(ui.saveStructButton, SIGNAL(clicked()), this, SLOT(saveXmlStruct()));
	//connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(findAdd()));
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(findXQuery()));

	connect(ui.findHousesButton, SIGNAL(clicked()), this, SLOT(parseXml()));

    xml_struct.clear();

}

void XmlAddressParse::openXml()
{
    m_inputFile = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("data (*.xml)"));
    ui.openEdit->setText(m_inputFile);
}

int XmlAddressParse::copyXml()
{
    //QFile* xmlFile = new QFile("check_diff.xml");
    QFile* xmlFile = new QFile(m_inputFile);
    if (!xmlFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
        return -1;
    }

    QXmlStreamReader xml(xmlFile);

    QFile* xmlOutFile = new QFile("Out.xml");
    if (!xmlOutFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
        return -1;
    }

    QXmlStreamWriter xmlOut(xmlOutFile);
    xmlOut.setAutoFormatting(true);

    int i = 0;
    bool stop = false;

    while (!xml.atEnd() && !xml.hasError() && i < ui.copyEdit->text().toInt() && !stop)
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        ++i;
        if (xml.error() != QXmlStreamReader::NoError) {
            qDebug() << xml.error() << "   " << xml.errorString();
        }

        switch (token)
        {
        case QXmlStreamReader::NoToken:
            break;
        case QXmlStreamReader::Invalid:
            break;
        case QXmlStreamReader::StartDocument:
            xmlOut.writeStartDocument();
            break;
        case QXmlStreamReader::EndDocument:
            xmlOut.writeEndDocument();
            break;
        case QXmlStreamReader::StartElement:
            if (xml.name() == ui.elementEdit->text() &&
                xml.attributes().hasAttribute(ui.attributeEdit->text()) &&
                xml.attributes().value(ui.attributeEdit->text()) == ui.valueEdit->text())
            {
                stop = true;
                break;
            }
            xmlOut.writeStartElement(xml.name().toString());
            xmlOut.writeAttributes(xml.attributes());
            break;
        case QXmlStreamReader::EndElement:
            xmlOut.writeEndElement();
            break;
        case QXmlStreamReader::Characters:
            xmlOut.writeCharacters(xml.text().toString());
            break;
        case QXmlStreamReader::Comment:
            break;
        case QXmlStreamReader::DTD:
            break;
        case QXmlStreamReader::EntityReference:
            break;
        case QXmlStreamReader::ProcessingInstruction:
            break;
        default:
            break;
        }
    }

    ui.copyEdit->setText(QString::number(i));
    xmlFile->close();
    xmlOutFile->close();

    ui.resultsBrowser->append("Copy Done\n");

    return 0;
}

int XmlAddressParse::findXml()
{
    //QFile* xmlFile = new QFile("check_diff.xml");
    QFile* xmlFile = new QFile(m_inputFile);
    if (!xmlFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
        return -1;
    }

    QXmlStreamReader xml(xmlFile);

    bool stop = false;
    check_struct whole_struct[100];
    int k = 0;

    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (xml.error() != QXmlStreamReader::NoError) {
            qDebug() << xml.error() << "   " << xml.errorString();
        }
        switch (token)
        {
        case QXmlStreamReader::NoToken:
            break;
        case QXmlStreamReader::Invalid:
            break;
        case QXmlStreamReader::StartDocument:  
            break;
        case QXmlStreamReader::EndDocument:
            break;
        case QXmlStreamReader::StartElement:
            if (xml.name() == ui.highEdit->text())
            {
                xml.readNext();
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == ui.highEdit->text()))
                {
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == ui.mid1Edit->text())
                    {
                        xml.readNext();
                        if (xml.tokenType() == QXmlStreamReader::Characters)
                        {
                            whole_struct[k].name = xml.text().toString();
                        }
                    }
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == ui.mid2Edit->text())
                    {
                        xml.readNext();
                        if (xml.tokenType() == QXmlStreamReader::Characters)
                        {
                            whole_struct[k].action = xml.text().toString();
                        }
                    }
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == ui.mid3Edit->text())
                    {
                        xml.readNext();
                        if (xml.tokenType() == QXmlStreamReader::Characters)
                        {
                            whole_struct[k].temp_delay = xml.text().toString();
                        }
                    }
                    xml.readNext();
                }
                if (whole_struct[k].temp_delay != ui.valEdit->text())
                {
                    whole_struct[k].clear();
                    --k;
                }
                ++k;
            }
            break;
        case QXmlStreamReader::EndElement:
            break;
        case QXmlStreamReader::Characters:
            break;
        case QXmlStreamReader::Comment:
            break;
        case QXmlStreamReader::DTD:
            break;
        case QXmlStreamReader::EntityReference:
            break;
        case QXmlStreamReader::ProcessingInstruction:
            break;
        default:
            break;
        }
        
    }
    for (int j = 0; j < k; ++j)
    {
        ui.resultsBrowser->append(whole_struct[j].name + "  " + whole_struct[j].action + "  " + whole_struct[j].temp_delay);
    }

    xmlFile->close();

    ui.resultsBrowser->append("\n");

    return 0;
}
/* Рекурсивный парсинг файла и сохранение структуры документа в XmlStruct */
void XmlAddressParse::parseElement(QXmlStreamReader *stream, XmlStruct *currentStruct)
{
    currentStruct->name = stream->name().toString();
    currentStruct->attributes = stream->attributes();
    int currentNum = 0;
    stream->readNext();
    while (!(stream->tokenType() == QXmlStreamReader::EndElement && stream->name() == currentStruct->name))
    {
        if (stream->tokenType() == QXmlStreamReader::Characters)
        {
            currentStruct->text = stream->text().toString();
            stream->readNext();
            continue;
        }
        else if (stream->tokenType() == QXmlStreamReader::StartElement)
        {
            currentStruct->elements.push_back(XmlStruct());
            parseElement(stream, &currentStruct->elements[currentNum++]);
        }
        stream->readNext();
    }
}

int XmlAddressParse::saveXmlStruct()
{
    QFile* xmlFile = new QFile(m_inputFile);
    if (!xmlFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
        return -1;
    }

    QXmlStreamReader *xml = new QXmlStreamReader(xmlFile);
    while (!xml->atEnd() && !xml->hasError())
    {
        QXmlStreamReader::TokenType token = xml->readNext();
        if (token == QXmlStreamReader::StartElement)
        {
            parseElement(xml, &xml_struct);
        }
    }
    xmlFile->close();
    ui.resultsBrowser->append("Struct Save\n");

    return 0;
}

int XmlAddressParse::findAdd()
{
	//QFile* xmlFile = new QFile("check_diff.xml");
	QFile* xmlFile = new QFile(m_inputFile);
	if (!xmlFile->open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
		return -1;
	}

	QXmlStreamReader xml(xmlFile);

	QFile* xmlOutFile = new QFile("Out.xml");
	if (!xmlOutFile->open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
		return -1;
	}

	QXmlStreamWriter xmlOut(xmlOutFile);
	xmlOut.setAutoFormatting(true);

	bool stop = false;
	check_struct whole_struct[100];
	int k = 0;
	bool has = false;
	unsigned int counter = 0;

	while (!xml.atEnd() && !xml.hasError())
	{
		QXmlStreamReader::TokenType token = xml.readNext();
		if (xml.error() != QXmlStreamReader::NoError) {
			qDebug() << xml.error() << "   " << xml.errorString();
		}
		switch (token)
		{
		case QXmlStreamReader::StartDocument:
			xmlOut.writeStartDocument();
			break;
		case QXmlStreamReader::EndDocument:
			xmlOut.writeEndDocument();
			break;
		case QXmlStreamReader::StartElement:
			//if (xml.name() == "Object")
			//{
			//	QXmlStreamAttributes attr = xml.attributes();
			//	if (xml.attributes().value("REGIONCODE") == "77")
			//	{
			//		xmlOut.writeStartElement(xml.name().toString());
			//		xmlOut.writeAttributes(xml.attributes());
			//		has = true;
			//	}
			//	else
			//	{
			//		has = false;
			//	}
			//}

			if (xml.name() == "House")
			{
				QXmlStreamAttributes attr = xml.attributes();
				if (xml.attributes().value("REGIONCODE") == "77")
				{
					xmlOut.writeStartElement(xml.name().toString());
					xmlOut.writeAttributes(xml.attributes());
					has = true;
					++counter;
				}
				else
				{
					has = false;
				}
				
				if (counter >= 300000)
				{
					xmlOutFile->close();
					xmlOutFile = new QFile("Out2.xml");
				}

			}

			break;
		case QXmlStreamReader::EndElement:
			if (has)
			{
				xmlOut.writeEndElement();
			}
			break;
		case QXmlStreamReader::Characters:
			xmlOut.writeCharacters(xml.text().toString());
			break;
		default:
			break;
		}

	}

	xmlFile->close();
	xmlOutFile->close();
	ui.resultsBrowser->append("Done\n");

	return 0;
}

int XmlAddressParse::findXQuery()
{
	QFile* moscowFile = new QFile("Moscow.xml");
	if (!moscowFile->open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
		return -1;
	}

	QXmlStreamReader xml(moscowFile);
	QSet<QString> objects_id;
	//заполнение QSet. парсинг Moscow.xml.
	while (!xml.atEnd() && !xml.hasError())
	{
		if (xml.readNext() == QXmlStreamReader::StartElement)
		{
			if (xml.name() == "Object")
			{
				if (xml.attributes().hasAttribute("AOGUID"))
				{
					objects_id.insert(xml.attributes().value("AOGUID").toString());
				}
			}
		}
		if (xml.error() != QXmlStreamReader::NoError)
		{
			qDebug() << xml.error() << "   " << xml.errorString();
		}

	}
	ui.resultsBrowser->append("Parse Moscow.xml done");

	//Создание xml с домами для Москвы
	QFile* xmlOutFile = new QFile("houses.xml");
	if (!xmlOutFile->open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
		return -1;
	}

	QXmlStreamWriter xmlOut(xmlOutFile);
	xmlOut.setAutoFormatting(true);

	QFile* xmlFile = new QFile(m_inputFile);
	if (!xmlFile->open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
		return -1;
	}

	xml.setDevice(xmlFile);
	bool has = false;

	while (!xml.atEnd() && !xml.hasError())
	{
		QXmlStreamReader::TokenType token = xml.readNext();
		if (xml.error() != QXmlStreamReader::NoError) {
			qDebug() << xml.error() << "   " << xml.errorString();
		}
		switch (token)
		{
		case QXmlStreamReader::StartDocument:
			xmlOut.writeStartDocument();
			break;
		case QXmlStreamReader::EndDocument:
			xmlOut.writeEndDocument();
			break;
		case QXmlStreamReader::StartElement:
			if (xml.name() == "House")
			{
				if (xml.attributes().hasAttribute("AOGUID")) {
					if (objects_id.contains(xml.attributes().value("AOGUID").toString()))
					{
						xmlOut.writeStartElement(xml.name().toString());
						xmlOut.writeAttributes(xml.attributes());
						has = true;
					}
					else
					{
						has = false;
					}
				}
			}
			break;
		case QXmlStreamReader::EndElement:
			if (has)
			{
				xmlOut.writeEndElement();
			}
			break;
		case QXmlStreamReader::Characters:
			xmlOut.writeCharacters(xml.text().toString());
			break;
		default:
			break;
		}
	}

	//while (!xml.atEnd() && !xml.hasError())
	//{
	//	if (xml.readNext() == QXmlStreamReader::StartElement)
	//	{
	//		if (xml.name() == "House")
	//		{
	//			if (xml.attributes().value("HOUSENUM") == "7")
	//			{
	//				//strOutput += xml.attributes().value("HOUSEID");
	//				//strOutput += ";";
	//			}
	//		}
	//	}
	//	if (xml.error() != QXmlStreamReader::NoError)
	//	{
	//		qDebug() << xml.error() << "   " << xml.errorString();
	//	}
	//
	//}
	/*-----------------------------------------------------------------*/

	xmlFile->close();
	xmlOutFile->close();

	//QStringList strlist = strOutput.split(";");
	//for (QString str : strlist) {
	//	ui.resultsBrowser->append(str);
	//}
	ui.resultsBrowser->append("Done");

    return 0;
}

//void xquery()
//{
	//QFile* xmlFile = new QFile("Out.xml");
	//if (!xmlFile->open(QIODevice::ReadOnly))
	//{
	//	QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
	//	return -1;
	//}
	//
	/*----------------------- XQery -----------------------------------*/
	//QFile* xmlOutFile = new QFile("Out2.xml");
	//if (!xmlOutFile->open(QIODevice::WriteOnly))
	//{
	//	QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
	//	return -1;
	//}
	//
	//QFile* xqFile = new QFile("input_house.xq");
	//if (!xqFile->open(QIODevice::ReadOnly))
	//{
	//	QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
	//	return -1;
	//}
	//QString strQuery = xqFile->readAll();
	//
	//QXmlQuery query;
	//query.bindVariable("inputDocument", xmlFile);
	//query.setQuery(strQuery);
	//query.setQuery("doc('Out.xml')/Houses/House[@HOUSENUM = \"7\"]");
	//query.setQuery("doc('Out.xml')/Houses");
	//query.setQuery("doc('avtransport.xml')");
	//if (!query.isValid()) {
	//	qDebug() << "Query is invalid!";
	//	return 0;
	//}
	//
	//QXmlSerializer serializer(query, xmlOutFile);
	//if (!query.evaluateTo(&serializer)) {
	//	qDebug() << "Can't evaluate the query!";
	//	return 0;
	//}
	//if (!query.evaluateTo(&strOutput)) {
	//	qDebug() << "Can't evaluate the query!";
	//	return 0;
	//}
//}

void  XmlAddressParse::parseXml()
{
	//QtConcurrent::run(this, &QtGuiApplication1::foo);
	ui.resultsBrowser->append("Wait up to 10 minutes\n");
	qApp->processEvents();
	QStringList results = foo();
    int i = 0;
	if (!results.empty()) {
		results.sort();
		results.removeDuplicates();
		for (QString str : results)
		{
            QString ss = QString::number(++i);
            ss = ss + " " + str;
            ui.resultsBrowser->append(ss);
			//emit appendIt(str);
		}
	}
	else
	{
		ui.resultsBrowser->append("No such houses!");
	}
	ui.resultsBrowser->append("\n");
}

QStringList XmlAddressParse::foo()
{
	//ui.resultsBrowser->append("Wait up to 10 minutes");
	//QString inp = "77";
	QString re;

	//re = QString("^%1[а-яА-ЯёЁ_]*").arg(inp);
	QString inp;
	if (!ui.findHousesEdit->text().isEmpty())
	{
		inp = ui.findHousesEdit->text();
	}
	else
	{
		return QStringList();
	}
	re = QString("^%1[а-яА-ЯёЁ_]*").arg(inp);

	QRegExp rx(re);

	QString str1 = "7";
	QString str2 = "77";
	QString str3 = "7А";
	QString str4 = "77А";
	QString str5 = "77_А";
	QString str6 = "77/33";
	QStringList s6 = str3.split("/");

	bool res1 = rx.exactMatch(str1);
	bool res2 = rx.exactMatch(str2);
	bool res3 = rx.exactMatch(str3);
	bool res4 = rx.exactMatch(str4);
	bool res5 = rx.exactMatch(str5);
	bool res6 = rx.exactMatch(str6);

	bool res7 = rx.exactMatch(s6.first());


	QFile* xmlFile = new QFile("Moscow.xml");
	if (!xmlFile->open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
		return QStringList();
	}
	QXmlStreamReader xml(xmlFile);
	QMap<QString, QString> streets;
	while (!xml.atEnd() && !xml.hasError())
	{
		if (xml.readNext() == QXmlStreamReader::StartElement)
		{
			if (xml.attributes().hasAttribute("AOGUID"))
			{
				if (!streets.contains(xml.attributes().value("AOGUID").toString()))
				{
					streets.insert(xml.attributes().value("AOGUID").toString(), xml.attributes().value("OFFNAME").toString() + " " + xml.attributes().value("SHORTNAME").toString() + " ");
				}
			}
		}
	}
	xmlFile->close();

	//xmlFile = new QFile("houses_small.xml");
	xmlFile = new QFile("houses.xml");
	if (!xmlFile->open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
		return QStringList();
	}
	xml.setDevice(xmlFile);
	QStringList houses;
	while (!xml.atEnd() && !xml.hasError())
	{
		if (xml.readNext() == QXmlStreamReader::StartElement)
		{
			if (xml.attributes().hasAttribute("HOUSENUM"))
			{
				QStringList str = xml.attributes().value("HOUSENUM").toString().split("/");
				if (rx.exactMatch(str.first()))
				{
					houses.append(streets[xml.attributes().value("AOGUID").toString()] + xml.attributes().value("HOUSENUM").toString());
				}
			}
		}
	}
	xmlFile->close();
	return houses;
}
