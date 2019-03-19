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

        addressCollection = new ymaps.GeoObjectCollection(null, {});

        //objectManager = new ymaps.ObjectManager({
        //    // Чтобы метки начали кластеризоваться, выставляем опцию.
        //    clusterize: true,
            // ObjectManager принимает те же опции, что и кластеризатор.
        //    gridSize: 32,
        //    clusterDisableClickZoom: true
        //});

    var addresses = [];
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

                    // Добавление метки (Placemark) в коллекцию
                    addressCollection.add(new ymaps.Placemark(coordinates, {
                            hintContent: addresses[addressIndex()],
                            balloonContent: addresses[addressIndex()]
                        }, {
                            'preset': 'islands#blueDotIcon'
                        }
                    ));
                }
            )

        })(j);


        myMap.geoObjects.add(addressCollection);
    });

}