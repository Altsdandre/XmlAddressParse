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
#include <QTextStream>

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

const QString rootPath = "/Users/dmitriy/Projects/QtProjects/XmlAddressParse/XmlAddressParse/";

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
    m_webView->load(QUrl::fromLocalFile(rootPath + "ya_page.html"));

    QRegExpValidator *validator = new QRegExpValidator(rx);
    ui.copyEdit->setValidator(validator);

    m_inputFile = ui.openEdit->text();

    connect(ui.openButton, SIGNAL(clicked()), this, SLOT(openXml()));
    connect(ui.copyButton, SIGNAL(clicked()), this, SLOT(copyXml()));
    connect(ui.findButton, SIGNAL(clicked()), this, SLOT(findXml()));
    connect(ui.saveStructButton, SIGNAL(clicked()), this, SLOT(saveXmlStruct()));
	//connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(findAdd()));
    //connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(findXQuery()));
    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(makeShortHousesAtPostal()));

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
    QFile* moscowFile = new QFile(rootPath + "Moscow.xml");
	if (!moscowFile->open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
		return -1;
	}

	QXmlStreamReader xml(moscowFile);
//==============================================================================================
    QFile* mySocrFile = new QFile(rootPath + "shortNames.txt");
    if (!mySocrFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
        return -1;
    }
    QSet<QString> mySosr;
    QTextStream in(mySocrFile);
    while (!in.atEnd()) {
        mySosr.insert(in.readLine());
    }
    mySocrFile->close();
    //создать новый москоу.иксмл
    QFile* moscowNew = new QFile(rootPath + "MoscowNew.xml");
    if (!moscowNew->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
        return -1;
    }
    //xml.setDevice(xmlFile);
    QXmlStreamWriter moscowNewXml(moscowNew);
    moscowNewXml.setAutoFormatting(true);
    QSet<QString> objects_id_new;
    //заполнение QSet. парсинг Moscow.xml.
    bool has2 = false;

    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (xml.error() != QXmlStreamReader::NoError) {
            qDebug() << xml.error() << "   " << xml.errorString();
        }
        switch (token)
        {
        case QXmlStreamReader::StartDocument:
            moscowNewXml.writeStartDocument();
            break;
        case QXmlStreamReader::EndDocument:
            moscowNewXml.writeEndDocument();
            break;
        case QXmlStreamReader::StartElement:
            if (xml.name() == "Object")
            {
                if (xml.attributes().hasAttribute("AOGUID"))
                {
                    if (mySosr.contains(xml.attributes().value("SHORTNAME").toString())) {
                        objects_id_new.insert(xml.attributes().value("AOGUID").toString());
                    }
                }
            }
            if (xml.name() == "Object")
            {
                if (xml.attributes().hasAttribute("AOGUID")) {
                    if (mySosr.contains(xml.attributes().value("SHORTNAME").toString()))
                    {
                        moscowNewXml.writeStartElement(xml.name().toString());
                        moscowNewXml.writeAttributes(xml.attributes());
                        has2 = true;
                    }
                    else
                    {
                        has2 = false;
                    }
                }
            }
            break;
        case QXmlStreamReader::EndElement:
            if (has2)
            {
                moscowNewXml.writeEndElement();
            }
            break;
        case QXmlStreamReader::Characters:
            moscowNewXml.writeCharacters(xml.text().toString());
            break;
        default:
            break;
        }
    }



    moscowFile->close();
    moscowNew->close();



    //создать новый хоусез.иксмл

//==============================================================================================
    QSet<QString> shortNames;
    //заполнение QSet. парсинг Moscow.xml.
    while (!xml.atEnd() && !xml.hasError())
    {
        if (xml.readNext() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "Object")
            {
                if (xml.attributes().hasAttribute("SHORTNAME"))
                {
                    shortNames.insert(xml.attributes().value("SHORTNAME").toString());
                }
            }
        }
        if (xml.error() != QXmlStreamReader::NoError)
        {
            qDebug() << xml.error() << "   " << xml.errorString();
        }

    }
    QFile* dataOutFile = new QFile(rootPath + "shortNames.txt");
    if (!dataOutFile->open(QIODevice::WriteOnly))
    {
        //QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
        ui.resultsBrowser->append("Can not open file!\n");
        //return -1;
    }
    QList<QString> nameslist = shortNames.toList();
    qSort(nameslist.begin(), nameslist.end());
    QTextStream stream(dataOutFile);

    for (QString str : nameslist) {
        stream << str + "\n";
    }
    dataOutFile->close();
//==============================================================================================
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

    QFile* dataOutFile = new QFile(rootPath + "addresses.txt");
    if (!dataOutFile->open(QIODevice::WriteOnly))
    {
        //QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
        ui.resultsBrowser->append("Can not open file!\n");
        //return -1;
    }
    QTextStream stream(dataOutFile);
	if (!results.empty()) {
		results.sort();
		results.removeDuplicates();
		for (QString str : results)
		{
            QString ss = QString::number(++i);
            ss = ss + " " + str;
            ui.resultsBrowser->append(ss);


            stream << "'Москва, " + str + "'\n";
		}
	}
	else
	{
		ui.resultsBrowser->append("No such houses!");
	}
	ui.resultsBrowser->append("\n");
    dataOutFile->close();

    if (!results.empty()) {
        m_webView->reload();
    }

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


    QFile* xmlFile = new QFile(rootPath + "Moscow.xml");
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
    //xmlFile = new QFile("houses.xml");
    xmlFile = new QFile(rootPath + "PostalHouses.xml");
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

void XmlAddressParse::makeShortHousesAtPostal()
{
    QSet<int> postalCodes;
    postalCodes << /* СВАО */
                   129164 << 129301 << 129366 << 129626 <<              /* Алексеевский */
                   127349 << 127543 << 127549 << 127560 <<              /* Бибирево */
                   129281 << 129336 << 129345 << 129346 <<              /* Лосиноостровский */
                   127224 << 127282 <<                                  /* Медведково Северное */
                   127273 << 127490 << 127562 << 127566 <<              /* Отрадное */
                   127204 << 127495 <<                                  /* Северный */
                   127410 <<                                            /* Алтуфьево */
                   127254 << 127322 <<                                  /* Бутырский */
                   127106 << 127276 << 127427 <<                        /* Марьино */
                   127081 << 127221 << 127642 <<                        /* Медведково Южное */
                   129128 << 129226 <<                                  /* Ростокино */
                   129337 << 129338 << 129347 <<                        /* Ярославский */
                   129327 << 129344 <<                                  /* Бабушкинский */
                   127253 << 127572 << 127576 <<                        /* Лианозово */
                   127018 << 127521 << 129594 <<                        /* Марьина Роща */
                   129075 << 129085 << 129223 << 129515 <<              /* Останкинский */
                   129323 << 129329 << 129343 <<                        /* Свиблово */
                   /* ЦАО */
                   123001 << 123022 << 123056 << 123100 <<              /* Пресненский */
                   123104 << 123242 << 123290 << 123317 << 123557 <<    /* Пресненский */
                   109012 << 109240 << 125009 << 125047 <<              /* Тверской */
                   127006 << 127030 << 127051 << 127055 << 127473 <<    /* Тверской */
                   107031 << 129090 << 129110 << 129272 <<              /* Мещанский */
                   107045 << 107078 << 107140 <<                        /* Красносельский */
                   101000 << 105005 << 105062 <<                        /* Басманный */
                   105064 << 105066 << 105082 << 105094 <<              /* Басманный */
                   /* САО */
                   125040 << 125124 << 125284 << 127137 <<              /* Беговой */
                   127015 << 127220 << 127287 <<                        /* Савеловский */
                   127206 << 127238 << 127422 << 127434 << 127550 <<    /* Тимирязевский */
                   125008 << 125183 << 125239 << 125299 <<              /* Коптево */
                   125413 << 125438 << 125493 << 125499 << 125565 <<    /* Головинский */
                   125414 << 125475 << 125502 << 125581 <<              /* Ховрино */
                   125195 << 125445 <<                                  /* Левобережный */
                   125412 << 125599 << 125635 <<                        /* Западное дегунино */
                   125237 << 125540 << 125591 <<                        /* Восточное Дегунино */
                   127411 << 127644 <<                                  /* Дмитровский */
                   127247 << 127474 << 127486 <<                        /* Бескудниковский */
                   /* ВАО */
                   107014 << 107113 <<                                  /* Сокольники */
                   107150 << 107258 << 107370 << 107564 <<              /* Богородское */
                   107143 <<                                            /* Метрогородок */
                   107023 << 107061 << 107076 << 107362 << 107553 <<    /* Преображенское */
                   107065 << 107207 << 107241 << 107497 << 107589;      /* Гольяново */

    QFile* xmlFile = new QFile(rootPath + "houses.xml");
    if (!xmlFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
    }
    QFile* xmlOutFile = new QFile(rootPath + "PostalHouses.xml");
    if (!xmlOutFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!", QMessageBox::Ok);
    }
    QXmlStreamReader xml(xmlFile);

    QXmlStreamWriter xmlOut(xmlOutFile);
    xmlOut.setAutoFormatting(true);

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
                if (xml.attributes().hasAttribute("POSTALCODE")) {
                    if (postalCodes.contains(xml.attributes().value("POSTALCODE").toInt()))
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

    xmlFile->close();
    xmlOutFile->close();
}
