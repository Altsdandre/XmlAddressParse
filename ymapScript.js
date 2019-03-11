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
        yellowCollection = new ymaps.GeoObjectCollection(null, {
            preset: 'islands#yellowIcon'
        }),

        addressCollection = new ymaps.GeoObjectCollection(null, {
            preset: 'islands#yellowIcon'
        }),

        yellowCoords = [[55.73, 37.75], [55.81, 37.75]],

        addresses = [
            'Москва, 1905 года ул., д.19',
            'Москва, ул. Льва Толстого, 16'
        ];

    // Строка с адресом, который необходимо геокодировать
    //var address = 'Москва, ул. Льва Толстого, 16';
    //var addresses = [
    //    'Москва, 1905 года ул., д.19',
    //    'Москва, ул. Льва Толстого, 16'
    //];
    //var address = 'Москва';

    // Ищем координаты указанного адреса
    // https://tech.yandex.ru/maps/doc/jsapi/2.1/ref/reference/geocode-docpage/
    for (var j = 0, k = addresses.length; j < k; j++) {
        //var j = 0;
        var geocoder = ymaps.geocode(addresses[j]);
        //var geocoder = ymaps.geocode('Москва, 1905 года ул., д.19');

        // После того, как поиск вернул результат, вызывается callback-функция
        geocoder.then(
            function (res) {

                // координаты объекта
                var coordinates = res.geoObjects.get(0).geometry.getCoordinates();

                // Добавление метки (Placemark) на карту
                //var placemark = new ymaps.Placemark(
                //    coordinates, {
                //        'hintContent': addresses[j],
                //        //'balloonContent': address[j]
                //    }, {
                //        'preset': 'islands#redDotIcon'
                //    }
                //);
                addressCollection.add(new ymaps.Placemark(
                    coordinates, {
                        'hintContent': addresses[j],
                        //        'balloonContent': address[j]
                    }, {
                        'preset': 'islands#redDotIcon'
                    }
                ));
                //myMap.geoObjects.add(placemark);
            }
        );
    }
    myMap.geoObjects.add(addressCollection);
    //for (var i = 0, l = yellowCoords.length; i < l; i++) {
    //    yellowCollection.add(new ymaps.Placemark(yellowCoords[i]));

    //myMap.geoObjects.add(yellowCollection);

}