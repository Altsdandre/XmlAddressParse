ymaps.ready(init);

function init() {
    // Создание карты.
    // https://tech.yandex.ru/maps/doc/jsapi/2.1/dg/concepts/map-docpage/
    var myMap = new ymaps.Map("map", {
            // Координаты центра карты.
            // Порядок по умолчнию: «широта, долгота».
            center: [55.76, 37.64],
            // Уровень масштабирования. Допустимые значения:
            // от 0 (весь мир) до 19.
            zoom: 10,
            // Элементы управления
            // https://tech.yandex.ru/maps/doc/jsapi/2.1/dg/concepts/controls/standard-docpage/
            controls: [

                'zoomControl', // Ползунок масштаба
                'rulerControl', // Линейка
                'routeButtonControl', // Панель маршрутизации
                'trafficControl', // Пробки
                'typeSelector', // Переключатель слоев карты
                'fullscreenControl', // Полноэкранный режим

                // Поисковая строка
                new ymaps.control.SearchControl({
                    options: {
                        // вид - поисковая строка
                        size: 'large',
                        // Включим возможность искать не только топонимы, но и организации.
                        provider: 'yandex#search'
                    }
                })

            ]
        }),

        addressCollection = new ymaps.GeoObjectCollection(null, {}),

        clusterer = new ymaps.Clusterer({
            /**
             * Через кластеризатор можно указать только стили кластеров,
             * стили для меток нужно назначать каждой метке отдельно.
             * @see https://api.yandex.ru/maps/doc/jsapi/2.1/ref/reference/option.presetStorage.xml
             */
            //preset: 'islands#invertedVioletClusterIcons',
            /**
             * Ставим true, если хотим кластеризовать только точки с одинаковыми координатами.
             */
            groupByCoordinates: false
            /**
             * Опции кластеров указываем в кластеризаторе с префиксом "cluster".
             * @see https://api.yandex.ru/maps/doc/jsapi/2.1/ref/reference/ClusterPlacemark.xml
             */
            //clusterDisableClickZoom: true,
            //clusterHideIconOnBalloonOpen: false,
            //geoObjectHideIconOnBalloonOpen: false
        }),
        ind = 0;
        addresses = [],
        marks = [];
    
    $.ajax({
        url: "addresses.txt"
    }).done(function(data) {
        addresses = data.split('\n');
        // Ищем координаты указанного адреса
        for (var j = 0, k = addresses.length; j < k; j++) (function(j) {

            var addressIndex = function() {
                return j;
            };

            var geocoder = ymaps.geocode(addresses[j]);

            // После того, как поиск вернул результат, вызывается callback-функция
                geocoder.then(
                    function (res) {
                        // координаты объекта
                        var coordinates = res.geoObjects.get(0).geometry.getCoordinates();

                        marks[addressIndex()] = new ymaps.Placemark(coordinates, {
                                hintContent: addresses[addressIndex()],
                                balloonContent: addresses[addressIndex()]
                            }, {
                                'preset': 'islands#blueDotIcon'
                            }
                        );
                        // Добавление метки (Placemark) в коллекцию
                        addressCollection.add(marks[addressIndex()]);
                        clusterer.add(marks[addressIndex()]);
                        ind = ind + 1;
                        // if (ind == addresses.length - 1) alert(marks.length);
                         if (ind == 500) alert("500");
                        // if (ind == 600) alert("600");
                         if (ind == 700) alert("700");
                        // if (ind == 800) alert("800");
                        // if (ind == 1000) alert("1000");
                    },
                    function (err) {
                        // Обработка ошибки.
                        alert(err);
                    }
                );

        })(j);

        myMap.geoObjects.add(clusterer);

        myMap.setBounds(clusterer.getBounds(), {
            checkZoomRange: true
        });

        //myMap.geoObjects.add(addressCollection);
    });

}