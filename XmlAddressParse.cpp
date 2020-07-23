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

/*
Файл all_houses_numbers_<city>.xml содержит номер дома, регион, почтовый индекс и идентификатор дома.
Файл all_houses_names_<city>.xml содерржит имя дома и идентификатор.
Идентификаторы дома в файлах совпадают (AOGUID).
В makeShortHousesAtPostal() задаются индексы районов поиска
    и формируется postal_houses_numbers_<city>.xml на основе all_houses_numbers_<city>.xml.
    В formAddressList() формируется список найденных адресов:
    в postal_houses_numbers_<city>.xml или all_houses_numbers_<city>.xml
    ищутся идентификаторы всех домов с искомым номером,
    по этому идентификатору формируется адрес дома из all_houses_names_<city>.xml.
    Далее к списку адресов добавляется город и сохраняется в файл addresses.txt.
    Список адресов из addresses.txt подхватывает js скрипт при обновлении окна браузера приложения (QWebEngineView).
*/

XmlAddressParse::XmlAddressParse(QWidget *parent)
    : QMainWindow(parent),
      m_settings("HomeCorp", "HouseFinder")
{
    ui.setupUi(this);

    //Загрузим информацию по запросам: сколько и когда последний раз делались.
    //(Нам интересно количество запросов в день: у Яндекса ограничение - 10000)
    m_requestsToday = m_settings.value("requests").toInt();
    m_requestsDate  = m_settings.value("date").toDate();
    //Если запрос был не сегодня, отобразим ноль
    //Счетчик при этом не обнуляется - если запросов не будет, то сохранится старое значение счетчика.
    if (m_requestsDate != QDate::currentDate()) {
        ui.requestsNumber->setText(QString::number(0));
    } else {
        ui.requestsNumber->setText(QString::number(m_requestsToday));
    }

    //m_rootPath = "/Users/dmitriy/Projects/QtProjects/XmlAddressParse/XmlAddressParse/";
    m_rootPath = QCoreApplication::applicationDirPath() + "/";

    if (ui.mskButton->isChecked())
        m_city = "msk";
    else
        m_city = "spb";

    setFileNames();

    //Удалим файл со списком адресов, чтобы они не отображались на карте при запуске
    QString filename = m_rootPath + "addresses.txt";
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite)) {
        if (!file.remove()) {
            file.close();
        }
    }

    m_webView = new QWebEngineView(this);
    ui.mapScroll->setWidget(m_webView);
    m_webView->load(QUrl::fromLocalFile(m_rootPath + "ya_page.html"));

    m_inputFile = ui.openEdit->text();

    connect(ui.openButton, SIGNAL(clicked()), this, SLOT(openXml()));
	//connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(findAdd()));
    //connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(findXQuery()));
    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(makeShortHousesAtPostal()));

	connect(ui.findHousesButton, SIGNAL(clicked()), this, SLOT(parseXml()));

    connect(ui.mskButton, &QRadioButton::toggled, this, [=](bool state){
        if (state) m_city = "msk";
        setFileNames();
    });
    connect(ui.spbButton, &QRadioButton::toggled, this, [=](bool state){
        if (state) m_city = "spb";
        setFileNames();
    });
}

XmlAddressParse::~XmlAddressParse()
{
    m_settings.setValue("requests", m_requestsToday);
    m_settings.setValue("date",     m_requestsDate);
}

void XmlAddressParse::setFileNames()
{
    m_housesNamesFile   = m_rootPath + "all_houses_names_"   + m_city + ".xml";
    m_housesNumbersFile = m_rootPath + "all_houses_numbers_" + m_city + ".xml";
    m_postalHousesNumbersFile = m_rootPath + "postal_houses_numbers_" + m_city + ".xml";
}

void XmlAddressParse::openXml()
{
    m_inputFile = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("data (*.xml)"));
    ui.openEdit->setText(m_inputFile);
}

int XmlAddressParse::findAdd()
{
    QFile* xmlFile = new QFile(m_inputFile);
	if (!xmlFile->open(QIODevice::ReadOnly))
	{
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + m_inputFile, QMessageBox::Ok);
		return -1;
	}

	QXmlStreamReader xml(xmlFile);

    QString outFileName = m_objectName + "s_" + m_regionCode;
    QFile* xmlOutFile = new QFile(outFileName + ".xml");
	if (!xmlOutFile->open(QIODevice::WriteOnly))
	{
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + outFileName + ".xml", QMessageBox::Ok);
		return -1;
	}

	QXmlStreamWriter xmlOut(xmlOutFile);
	xmlOut.setAutoFormatting(true);

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

/*========================= Эти изменения стоит проверить внимательнее ======================================*/

            if (xml.name() == m_objectName)
			{
                if (xml.attributes().value("REGIONCODE") == m_regionCode)
				{
					xmlOut.writeStartElement(xml.name().toString());
					xmlOut.writeAttributes(xml.attributes());
					has = true;
                    if (m_objectName == "House")
                        ++counter;
				}
				else
				{
					has = false;
				}
                if (m_objectName == "House") {
                    if (counter >= 300000) {
                        static int i = 1;
                        xmlOutFile->close();
                        xmlOutFile = new QFile(outFileName + "_" + QString::number(++i) + ".xml");
                    }
                }

			}
/*========================= ************************ ====================================================*/

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
    QFile* moscowFile = new QFile(m_housesNamesFile);
	if (!moscowFile->open(QIODevice::ReadOnly))
	{
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + m_housesNamesFile, QMessageBox::Ok);
		return -1;
	}

	QXmlStreamReader xml(moscowFile);
//==============================================================================================
    QFile* mySocrFile = new QFile(m_rootPath + "shortNames.txt");
    if (!mySocrFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + m_rootPath + "shortNames.txt", QMessageBox::Ok);
        return -1;
    }
    QSet<QString> mySosr;
    QTextStream in(mySocrFile);
    while (!in.atEnd()) {
        mySosr.insert(in.readLine());
    }
    mySocrFile->close();
    //создать новый москоу.иксмл
    QString fileName1 = m_rootPath + "all_houses_names_" + m_city + "New.xml";
    QFile* moscowNew = new QFile(fileName1);
    if (!moscowNew->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + fileName1, QMessageBox::Ok);
        return -1;
    }
    //xml.setDevice(xmlFile);
    QXmlStreamWriter moscowNewXml(moscowNew);
    moscowNewXml.setAutoFormatting(true);
    QSet<QString> objects_id_new;
    //заполнение QSet. парсинг all_houses_names_.xml.
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
    //заполнение QSet. парсинг all_houses_names_.xml.
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
    QFile* dataOutFile = new QFile(m_rootPath + "shortNames.txt");
    if (!dataOutFile->open(QIODevice::WriteOnly))
    {
        //QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + m_rootPath + "shortNames.txt", QMessageBox::Ok);
        ui.resultsBrowser->append("Can not open file!" + m_rootPath + "shortNames.txt\n");
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
    //заполнение QSet. парсинг all_houses_names_.xml.
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
    ui.resultsBrowser->append("Parse all_houses_names_.xml done");

	//Создание xml с домами для Москвы
    QFile* xmlOutFile = new QFile(m_housesNumbersFile);
	if (!xmlOutFile->open(QIODevice::WriteOnly))
	{
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + m_housesNumbersFile, QMessageBox::Ok);
		return -1;
	}

	QXmlStreamWriter xmlOut(xmlOutFile);
	xmlOut.setAutoFormatting(true);

	QFile* xmlFile = new QFile(m_inputFile);
	if (!xmlFile->open(QIODevice::ReadOnly))
	{
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + m_inputFile, QMessageBox::Ok);
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

	xmlFile->close();
	xmlOutFile->close();

    ui.resultsBrowser->append("Done");

    return 0;
}
void  XmlAddressParse::parseXml()
{
	//QtConcurrent::run(this, &QtGuiApplication1::foo);
	ui.resultsBrowser->append("Wait up to 10 minutes\n");
	qApp->processEvents();
    QStringList results = formAddressList();
    int i = 0;

    QFile* dataOutFile = new QFile(m_rootPath + "addresses.txt");
    if (!dataOutFile->open(QIODevice::WriteOnly))
    {
        ui.resultsBrowser->append("Can not open file!\n" + m_rootPath + "addresses.txt");
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

            if (ui.mskButton->isChecked())
                stream << "'Москва, " + str + "'";
            else
                stream << "'Санкт-Петербург, " + str + "'";
            if (i != results.count())
                stream << "\n";
		}
	}
	else
	{
		ui.resultsBrowser->append("No such houses!");
	}
	ui.resultsBrowser->append("\n");
    dataOutFile->close();

    if (!results.empty()) {
        //Если найденных адресов больше 700, выдать вопрос: отображать ли на карте?
        if(results.count() > 700) {
            auto result = QMessageBox::question(this, "Overlong list", "There are too many points!\nShow it anyway?",
                                                QMessageBox::Yes|QMessageBox::No);
            if (result == QMessageBox::No) {
                return;
            }
        }
        //Если дата не совпадает, значит, прошлый запрос был в другой день,
        // и подсчет нужно начинать заново.
        if (m_requestsDate != QDate::currentDate()) {
            m_requestsToday = 0;
            m_requestsDate = QDate::currentDate();
        }
        m_requestsToday += results.count();
        ui.requestsNumber->setText(QString::number(m_requestsToday));
        //Перезагружаем страницу, чтобы отобразить новые адреса из addresses.txt.
        m_webView->reload();
    }
}

QStringList XmlAddressParse::formAddressList()
{
	QString re;

	QString inp;
	if (!ui.findHousesEdit->text().isEmpty())
	{
		inp = ui.findHousesEdit->text();
	}
	else
	{
		return QStringList();
	}
    re = QString("^%1[а-яА-ЯёЁ_]*").arg(inp); //Ищутся адреса: 23, 45А, 31_а, 74к2, 1с1. Не найдутся: 34к7с2

	QRegExp rx(re);

    QFile* xmlFile = new QFile(m_housesNamesFile);
	if (!xmlFile->open(QIODevice::ReadOnly))
	{
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + m_housesNamesFile, QMessageBox::Ok);
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

    //Выбираем, где искать дома во всем городе или в заданном (по индексам) районе
    QString areaFileName;
    if (ui.districtButton->isChecked()) {
        areaFileName = m_postalHousesNumbersFile;
    } else {
        areaFileName = m_housesNumbersFile;
    }

    //Открываем файл с номерами домов
    xmlFile = new QFile(areaFileName);
	if (!xmlFile->open(QIODevice::ReadOnly))
	{
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + areaFileName, QMessageBox::Ok);
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
    QSet<int> postalCodes_msk;
    postalCodes_msk << /* СВАО */
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


    QSet<int> postalCodes_msk_2020;
        postalCodes_msk_2020 << /* ЮВАО */
                           109145 << 109153 << 109156 << 109507 << 109431 << /* Выхино-Жулебино */
                           109542 << 109444 << 109472 <<
                           111020 << 111024 << 111033 << 111116 << 111250 << /* Лефортово */
                           111674 <<                                         /* Некрасовка */
                           109377 << 109428 << 109456 << 109457 <<           /* Рязанский */
                           109429 << 109649 <<                               /* Капотня */
                           109380 << 109382 << 109386 << 109387 << 109559 << /* Люблино */
                           109052 << 109202 << 109391 <<                     /* Нижегородский */
                           109125 << 109129 << 109263 << 109390 << 109518 << /* Текстильщики */
                           109387 << 109548 <<
                           109117 << 109378 << 109439 << 109443 << 109457 << /* Кузьминки */
                           109462 <<
                           109144 << 109341 << 109369 << 109451 << 109469 << /* Марьино */
                           109651 << 109652 <<
                           109235 << 109383 << 109388 << 109548 <<           /* Печаники */
                           115088 << 115432 << 109193;                       /* Южнопортовый */
                           /* ЮАО */


    QSet<int> postalCodes_spb;

    postalCodes_spb << /* Адмиралтейский */
                   190000 << 190005 << 190013 << 190020 << 190031 <<
                   190068 << 190098 << 190103 << 190121 << 191002 <<
                   196084 <<
                   /* Центральный */
                   191014 << 191015 << 191023 << 191024 << 191025 <<
                   191028 << 191036 << 191038 << 191040 << 191060 <<
                   191119 << 191123 << 191124 << 191144 << 191167 <<
                   191180 << 191186 << 191187 << 191317 << 192007 <<
                   /* Кировский */
                   198035 << 198095 << 198096 << 198097 << 198099 <<
                   198152 << 198184 << 198188 << 198207 << 198216 <<
                   198217 << 198255 << 198260 << 198261 << 198262 <<
                   198302 << 198303 <<
                   /* Московский */
                   196006 << 196066 << 196070 << 196105 << 196128 <<
                   196135 << 196140 << 196143 << 196158 << 196191 <<
                   196210 << 196211 << 196233 << 196240 << 196244 <<
                   196247 <<
                   /* Фрунзенский */
                   192071 << 192102 << 192212 << 192236 << 192238 <<
                   192239 << 192241 << 192242 << 192281 << 192283 <<
                   192284 << 192286 << 192288 << 192289 <<
                   /* Невский */
                   192012 << 192019 << 192029 << 192076 << 192131 <<
                   192148 << 192171 << 192174 << 192177 << 193079 <<
                   193091 << 193149 << 193168 << 193230 << 193231 <<
                   193232 << 193312 << 193313 << 193315 << 193318 <<
                   /* Пушкинский */
                   196601 << 196602 << 196603 << 196605 << 196606 <<
                   196607 << 196608 << 196620 << 196621 << 196625 <<
                   196626 << 196627 << 196631 << 196632 << 196634 <<
                   196641 << 196642 << 199053 <<
                   /* Колпинский */
                   196650 << 196651 << 196652 << 196653 << 196654 <<
                   196655 << 196657 << 196658 << 196690 <<
                   /* Прочее */
                   197046 << 194300 << 195009 << 192159 << 194044 <<
                   195197 << 195160 << 195176 << 195248 << 195271 <<
                   195067;
    QSet<int> pc;
    pc << 194100 << 194153 << 194044 << 194223 << 194021 << 195251 << 195220 << 195427 << 194156 << 197343 << 197342;

    QSet<int> postalCodes;
    postalCodes = postalCodes_msk_2020;

    QFile* xmlFile = new QFile(m_housesNumbersFile);
    if (!xmlFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + m_housesNumbersFile, QMessageBox::Ok);
    }

    QFile* xmlOutFile = new QFile(m_postalHousesNumbersFile);
    if (!xmlOutFile->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(nullptr, "Warning", "Can not open file!\n" + m_postalHousesNumbersFile, QMessageBox::Ok);
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
            xmlOut.writeStartElement("Houses");  //<Houses>
            break;
        case QXmlStreamReader::EndDocument:
            xmlOut.writeEndElement();  // </Houses>
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
                has = false;
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
    ui.openEdit->setText("done");
}

void XmlAddressParse::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return) {
        parseXml();
    }
    QMainWindow::keyPressEvent(event);
}
