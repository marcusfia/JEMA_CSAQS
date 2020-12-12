


var mapLayers = [];
var button_id;
var requestOptions = { method: 'GET' };
var seconds;
var minutes;
var hours;
var old_data = false;
var mapBounds = []

// time interval device is reading at (400,000 is a bit more than 5 minutes)
// this needs to be adjusted to show data is old, if the sensor read time interval is changed.
var timeInterval = 400000


//used for removing/adding data color scales when filter buttons are selected
var click_ops = { 
        'temps':'tempScaleVisibility', 
        'pressure':'pressureScaleVisibility',
        'humidity':'humidityScaleVisibility',
        'particulate':'particulatesScaleVisibility',
        'nox':'noxScaleVisibility',
        'voltage':'voltageScaleVisibility',
        'voc':'vocScaleVisibility'
    }


//truncate numbers above 999 to xx.xk format
function kFormat(val) {
    return Math.abs(val) > 9999 ? Math.sign(val) * ((Math.abs(val) / 1000).toFixed(1)) + 'k' : Math.sign(val) * Math.abs(val)
}



//create svg circle markers custom to each data point
function createIcon(bg_color, opacity, textAlign, text_margin_top, font_color, font_size, data, data_suffix, old_data) {

    //if a sensor is determined to have old data, we color it gray and make it semitransparent
    if(old_data) {
        font_color = '#2a2a2a';
        bg_color = '#808080';
        opacity = '0.85';
    }

    icon = L.divIcon({ 
        className: "my-custom-pin",
        iconAnchor: [15, 15],
        labelAnchor: [0, 0],
        popupAnchor: [0, -15],
        html: `<svg width="30px" viewBox="0 0 60 60" xmlns="http://www.w3.org/2000/svg" version="1.1">
                    <circle cx="30" cy="30" r="28" opacity="${opacity}" stroke-width="1" stroke="black" fill="${bg_color}" shape-rendering="geometricPrecision"/>
                        <text x="50%" y="50%" text-anchor="middle" fill="${font_color}" font-size="1.9em" dy=".4em">${data}${data_suffix}</text>
               </svg>`
    });

  return icon;
}




// GET request to retreive latest data and populate markers
fetch("https://hogcaszzdb.execute-api.us-west-2.amazonaws.com/prod/mapdata", requestOptions)
    .then(response => response.json())
    .then(function(response) { 

        for (var key in mapLayers) {
            Object.keys(mapLayers).forEach(function(key) {
                mapLayers[key].removeFrom(map);
            }, mapLayers);
        }

        mapLayers["nox"] = L.geoJSON(response, {

            pointToLayer: function( feature, latlng ) {

                latlng.lat = latlng.lat/10000000
                latlng.lng = latlng.lng/10000000 

                feature.properties.ttl = "temp";
                feature.properties.nox = Number.parseFloat(feature.properties.nox).toPrecision(2)   

                if (feature.properties.nox < 51) {
                    nox_color = "#339a00"; font_color = "#FFF";
                } else if (feature.properties.nox >= 51 && feature.properties.nox < 101) {
                    nox_color = "#ffee00"; font_color = "#000";
                } else if (feature.properties.nox >= 101 && feature.properties.nox < 151){
                    nox_color = "#edaf34"; font_color = "#000";
                } else if (feature.properties.nox >= 151 && feature.properties.nox < 201){
                    nox_color = "#ff3a3a"; font_color = "#000";
                } else if (feature.properties.nox >= 201 && feature.properties.nox < 301){
                    nox_color = "#b700c1"; font_color = "#000";
                } else {
                    nox_color = "#883232"; font_color = "#000";
                }

                var date = new Date(0); // The 0 there is the key, which sets the date to the epoch
                var d = new Date();
                date.setUTCSeconds(feature.properties.time);
                var opacity;

                if(d - date > timeInterval){
                    old_data = true;
                }

                var nox_icon = createIcon(nox_color, 1, 'center', 0.3, font_color, 12, kFormat(feature.properties.nox), '', old_data);

                old_data = false;

                return L.marker(latlng, {
                    icon: nox_icon,
                    riseOnHover: true
                });

            }, onEachFeature: onEachFeature

        })


        // create and store voc markers
        mapLayers["voc"] = L.geoJSON(response, {

            pointToLayer: function( feature, latlng ) {

                latlng.lat = latlng.lat/10000000
                latlng.lng = latlng.lng/10000000
                feature.properties.voc = Math.round(feature.properties.voc/1000)

                if (feature.properties.voc < 51) {
                    voc_color = "#339a00"; font_color = "#000";
                } else if (feature.properties.voc >= 51 && feature.properties.voc < 101) {
                    voc_color = "#ffee00"; font_color = "#000";
                } else if (feature.properties.voc >= 101 && feature.properties.voc < 151){
                    voc_color = "#edaf34"; font_color = "#000";
                } else if (feature.properties.voc >= 151 && feature.properties.voc < 201){
                    voc_color = "#ff3a3a"; font_color = "#000";
                } else if (feature.properties.voc >= 201 && feature.properties.voc < 301){
                    voc_color = "#b700c1"; font_color = "#000";
                } else {
                    voc_color = "#883232"; font_color = "#000";
                }

                var date = new Date(0); // The 0 there is the key, which sets the date to the epoch
                var d = new Date();
                date.setUTCSeconds(feature.properties.time);
                var opacity;

                function kFormat(val) {
                    return Math.abs(val) > 999 ? Math.sign(val) * ((Math.abs(val) / 1000).toFixed(1)) + 'k' : Math.sign(val) * Math.abs(val)
                }


                if(d - date > timeInterval){
                    old_data = true;
                }

                var voc_icon = createIcon(voc_color, 1, 'center', 0.3, font_color, 11, kFormat(feature.properties.voc), '', old_data);

                old_data = false;

                return L.marker(latlng, {
                    icon: voc_icon,
                    riseOnHover: true
                });

            }, onEachFeature: onEachFeature

        })


        // create and store humidity markers
        mapLayers["humidity"] = L.geoJSON(response, {

            pointToLayer: function( feature, latlng ) {

                latlng.lat = latlng.lat/10000000
                latlng.lng = latlng.lng/10000000    
                feature.properties.humidity = Math.round(feature.properties.humidity);

                if (feature.properties.humidity < 10) {
                    humidity_color = "#4783cb"; font_color = "#FFF";
                } else if (feature.properties.humidity >= 10 && feature.properties.humidity < 20){
                    humidity_color = "#3fb1dc"; font_color = "#000";
                } else if (feature.properties.humidity >= 20 && feature.properties.humidity < 30){
                    humidity_color = "#37deec"; font_color = "#000";
                } else if (feature.properties.humidity >= 30 && feature.properties.humidity < 40){
                    humidity_color = "#3bcc88"; font_color = "#000";
                } else if (feature.properties.humidity >= 40 && feature.properties.humidity < 50){
                    humidity_color = "#3fb923"; font_color = "#000";
                } else if (feature.properties.humidity >= 50 && feature.properties.humidity < 60){
                    humidity_color = "#8ec71d"; font_color = "#000";
                } else if (feature.properties.humidity >= 60 && feature.properties.humidity < 70){
                    humidity_color = "#e2e42f"; font_color = "#000";
                } else if (feature.properties.humidity >= 70 && feature.properties.humidity < 80){
                    humidity_color = "#e6a011"; font_color = "#000";
                } else if (feature.properties.humidity >= 80 && feature.properties.humidity < 90){
                    humidity_color = "#f73606"; font_color = "#000";
                } else {
                    humidity_color = "#ff0000"; font_color = "#000";
                }

                var date = new Date(0); // The 0 there is the key, which sets the date to the epoch
                var d = new Date();
                date.setUTCSeconds(feature.properties.time);
                var opacity;

                if(d - date > timeInterval){
                    old_data = true;
                }

                var humidity_icon = createIcon(humidity_color, 1, 'center', 0.3, font_color, 12, kFormat(feature.properties.humidity), '%', old_data);
                
                old_data = false;

                return L.marker(latlng, {
                    icon: humidity_icon,
                    riseOnHover: true
                });

            }, onEachFeature: onEachFeature

        })


        // create and store pressure markers
        mapLayers["pressure"] = L.geoJSON(response, {

            pointToLayer: function( feature, latlng ) {

                latlng.lat = latlng.lat/10000000
                latlng.lng = latlng.lng/10000000

                feature.properties.pressure = Math.round(feature.properties.pressure); 

                if (feature.properties.pressure < 956) {
                    pressure_color = "#cb0c35"; font_color = "#FFF";
                } else if (feature.properties.pressure >= 956 && feature.properties.pressure < 979){
                    pressure_color = "#96186a"; font_color = "#FFF";
                } else if (feature.properties.pressure >= 979 && feature.properties.pressure < 1007){
                    pressure_color = "#682397"; font_color = "#FFF";
                } else if (feature.properties.pressure >= 1007 && feature.properties.pressure < 1035){
                    pressure_color = "#352fca"; font_color = "#FFF";
                } else {
                    pressure_color = "#003bff"; font_color = "#FFF";
                }

                var date = new Date(0); // The 0 there is the key, which sets the date to the epoch
                var d = new Date();
                date.setUTCSeconds(feature.properties.time);
                var opacity;

                if(d - date > timeInterval){
                    old_data = true;
                }

                var pressure_icon = createIcon(pressure_color, 1, 'center', 0.3, font_color, 11, kFormat(feature.properties.pressure), '', old_data);
                
                old_data = false;

                return L.marker(latlng, {
                    icon: pressure_icon,
                    riseOnHover: true
                });

            }, onEachFeature: onEachFeature

        })


        // create and store particulate markers
        mapLayers["particulate"] = L.geoJSON(response, {

            pointToLayer: function( feature, latlng ) {

                latlng.lat = latlng.lat/10000000
                latlng.lng = latlng.lng/10000000    
            
                if (feature.properties.IR < 50) {
                    IR_color = "#5eff00"; font_color = "#000";
                } else if (feature.properties.IR >= 50 && feature.properties.IR < 100){
                    IR_color = "#30cc00"; font_color = "#000";
                } else if (feature.properties.IR >= 100 && feature.properties.IR < 150){
                    IR_color = "#90cc2f"; font_color = "#000";
                } else if (feature.properties.IR >= 150 && feature.properties.IR < 200){
                    IR_color = "#cfb500"; font_color = "#000";
                } else if (feature.properties.IR >= 200 && feature.properties.IR < 300){
                    IR_color = "#f8ff00"; font_color = "#000";
                } else {
                    IR_color = "#ff0000"; font_color = "#000";
                }

                var date = new Date(0); // The 0 there is the key, which sets the date to the epoch
                var d = new Date();
                date.setUTCSeconds(feature.properties.time);
                var opacity;

                if(d - date > timeInterval){
                    old_data = true;
                }

                var IR_icon = createIcon(IR_color, 1, 'center', 0.3, font_color, 12, kFormat(feature.properties.vcc), '', old_data);
                
                old_data = false;

                return L.marker(latlng, {
                    icon: IR_icon,
                    riseOnHover: true
                });

            }, onEachFeature: onEachFeature

        })


        // create and store voltage markers
        mapLayers["voltage"] = L.geoJSON(response, {

            pointToLayer: function( feature, latlng ) {
                
                latlng.lat = latlng.lat/10000000
                latlng.lng = latlng.lng/10000000

                feature.properties.vcc = Number.parseFloat(feature.properties.vcc).toPrecision(2)

                if (feature.properties.vcc < 4) {
                    voltage_color = "#ff4848"; font_color = "#000";
                } else if (feature.properties.vcc >= 4 && feature.properties.vcc < 4.4){
                    voltage_color = "#d1ab50"; font_color = "#000";
                } else if (feature.properties.vcc >= 4.4 && feature.properties.vcc < 4.8){
                    voltage_color = "#46a35c"; font_color = "#000";
                } else {
                    voltage_color = "#30DC6E"; font_color = "#000";
                }

                var date = new Date(0); // The 0 there is the key, which sets the date to the epoch
                var d = new Date();
                date.setUTCSeconds(feature.properties.time);
                var opacity;

                if(d - date > timeInterval){
                    old_data = true;
                }

                var voltage_icon = createIcon(voltage_color, 1, 'center', 0.3, `${font_color}`, 12, kFormat(feature.properties.vcc), '', old_data)
                
                old_data = false;

                return L.marker(latlng, {
                    icon: voltage_icon,
                    riseOnHover: true
                });

            }, onEachFeature: onEachFeature

        })

        // create and store temps markers
        mapLayers["temps"] = L.geoJSON(response, {

            pointToLayer: function( feature, latlng ) {

                //convert temp (C to F)
                feature.properties.temp = Math.round((feature.properties.temp * 1.8) + 32)

                latlng.lat = latlng.lat/10000000
                latlng.lng = latlng.lng/10000000    
                
                if(feature.properties.temp < 40) {
                    if (feature.properties.temp <= -20) {
                        temp_color = "#9740ff"; font_color = "#000";
                    } else if (feature.properties.temp > -20 && feature.properties.temp < -15){
                        temp_color = "#ff40ed"; font_color = "#000";
                    } else if (feature.properties.temp >= -15 && feature.properties.temp < -10){
                        temp_color = "#da40ff"; font_color = "#000";
                    } else if (feature.properties.temp >= -10 && feature.properties.temp < -5){
                        temp_color = "#9740ff"; font_color = "#FFF";
                    } else if (feature.properties.temp >= -5 && feature.properties.temp < 0){
                        temp_color = "#6b0ce5"; font_color = "#FFF";
                    } else if (feature.properties.temp >= 0 && feature.properties.temp < 5){
                        temp_color = "#2131e7"; font_color = "#FFF";
                    } else if (feature.properties.temp >= 5 && feature.properties.temp < 10){
                        temp_color = "#2b5fd9"; font_color = "#FFF";
                    } else if (feature.properties.temp >= 10 && feature.properties.temp < 15){
                        temp_color = "#0183ec"; font_color = "#FFF";
                    } else if (feature.properties.temp >= 15 && feature.properties.temp < 20){
                        temp_color = "#45b5f5"; font_color = "#000";
                    } else if (feature.properties.temp >= 20 && feature.properties.temp < 25){
                        temp_color = "#00dbdd"; font_color = "#000";
                    } else if (feature.properties.temp >= 25 && feature.properties.temp < 30){
                        temp_color = "#00ffd2"; font_color = "#000";
                    } else if (feature.properties.temp >= 30 && feature.properties.temp < 35){
                        temp_color = "#27f4a7"; font_color = "#000";
                    } else {
                        temp_color = "#00ec93"; font_color = "#000";
                    }
                } else {
                    if (feature.properties.temp < 45){
                        temp_color = "#14ec65"; font_color = "#000";
                    } else if (feature.properties.temp >= 45 && feature.properties.temp < 50){
                        temp_color = "#29ff31"; font_color = "#000";
                    } else if (feature.properties.temp >= 50 && feature.properties.temp < 55){
                        temp_color = "#6ffb38"; font_color = "#000";
                    } else if (feature.properties.temp >= 55 && feature.properties.temp < 60){
                        temp_color = "#acff45"; font_color = "#000";
                    } else if (feature.properties.temp >= 60 && feature.properties.temp < 65){
                        temp_color = "#cbff59"; font_color = "#000";
                    } else if (feature.properties.temp >= 65 && feature.properties.temp < 70){
                        temp_color = "#e2ff5b"; font_color = "#000";
                    } else if (feature.properties.temp >= 70 && feature.properties.temp < 75){
                        temp_color = "#f7fe62"; font_color = "#000";
                    } else if (feature.properties.temp >= 75 && feature.properties.temp < 80){
                        temp_color = "#fad847"; font_color = "#000";
                    } else if (feature.properties.temp >= 80 && feature.properties.temp < 85){
                        temp_color = "#ffc600"; font_color = "#000";
                    } else if (feature.properties.temp >= 85 && feature.properties.temp < 90){
                        temp_color = "#f9a536"; font_color = "#000";
                    } else if (feature.properties.temp >= 90 && feature.properties.temp < 95){
                        temp_color = "#fa7922"; font_color = "#000";
                    } else {
                        temp_color = "#ff0000"; font_color = "#000";
                    }
                }

                var date = new Date(0); // The 0 there is the key, which sets the date to the epoch
                var d = new Date();
                date.setUTCSeconds(feature.properties.time);
                var opacity;

                if(d - date > timeInterval){
                    old_data = true;
                }

                var temp_icon = createIcon(temp_color, 1, 'center', 0.2, `${font_color}`, 14, kFormat(feature.properties.temp), '&deg', old_data)
        
                old_data = false;

                mapBounds.push([latlng])

                return L.marker(latlng, {
                    icon: temp_icon,
                    riseOnHover: true
                })

            }, onEachFeature: onEachFeature

        })
    map.fitBounds(mapLayers["temps"].getBounds().pad(0.5));
    return addFirstLayer(mapLayers);

 })


function addFirstLayer(mapLayers){
    mapLayers["temps"].addTo(map)
}



// used in below function for storing the old object to be able to remove fiter button highlighting from it when a new filter button is clicked 
var oldClass;

//jquery function for handling all behavior related to clicking filter buttons. (changes color scale, changes button highlighted, changes markers)
$(function() {
    $(".navbtn_jema").click(function() {
        //remove layers
        Object.keys(mapLayers).forEach(function(key) {
            this[key].removeFrom(map);
        }, mapLayers);
        //remove current color scale
        Object.keys(click_ops).forEach(function(key) {
            document.getElementsByClassName(this[key])[0].style.display = 'none';
        }, click_ops);
        //add new layer and color scale
        button = $(this).attr('name');
        var colorScale = document.getElementsByClassName(click_ops[button]);
        colorScale[0].style.display = 'block';
        mapLayers[button].addTo(map);
        $(oldClass).removeClass("btn-light")
        oldClass = this;
        $(this).addClass("btn-light");
        //change scale text
        scaleText = $(this).attr('label');
        $('.scale-text').text(scaleText);
    });
});





//time function used in popups
function get_seconds(date) {
    if(date.getSeconds() < 10){
        seconds = "0" + date.getSeconds();
        return seconds;
    } else { 
        seconds = date.getSeconds();
        return seconds; 
    }
}

//time function used in popups
function get_minutes(date) {
    if(date.getMinutes() < 10){
        minutes = "0" + date.getMinutes();
        return minutes;
    } else { 
        minutes = date.getMinutes();
        return minutes; 
    }
}

//time function used in popups
function get_hours(date) {
        hours = date.getHours();
        return hours; 
}



//function to create popups for each marker and add hover/tap behaviors to them
function onEachFeature(feature, layer) {
    var date = new Date(0);
    date.setUTCSeconds(feature.properties.time);
    var d = new Date();

    // if data is not 'old data', then allow it to be clicked for the data modal.
    if(d - date <= timeInterval) {
        layer.bindPopup('<strong>client id:</strong> '+ feature.properties.clientID);
        // on click - set the popup modal values to the device clicked on
        layer.on('click', function (e) {
            map.panTo(this.getLatLng())
            $('.flex-clientID').text(feature.properties.clientID)
            $('.flex-temperature').text(feature.properties.temp + String.fromCharCode(176))
            $('.flex-pressure').text(feature.properties.pressure + 'mb')
            $('.flex-humidity').text(feature.properties.humidity + '%')
            $('.flex-voc').text(feature.properties.voc)
            $('.flex-particulate').text(feature.properties.particulate)
            $('.flex-nox').text(feature.properties.nox + 'V')
            $('.flex-vcc').text(feature.properties.vcc + 'V')
            $('.flex-time').text(time)
            $("#modChart").modal()
        });
    } else {
        // display 'OFFLINE' in popup if device if old data detected
        layer.bindPopup('<strong>client id:</strong> '+ feature.properties.clientI + '<br><strong>OFFLINE</strong>');
    }

    // format into 12 hour AM/PM time
    var AM_PM = (date.getHours() >= 12) ? "PM" : "AM"
    var hours = (get_hours(date)%12 == 0) ? get_hours(date) : get_hours(date)%12
    var time = hours  + ":" + get_minutes(date) + AM_PM + "  " + (date.getMonth() + 1) + "/" + date.getDate() + "/" + date.getFullYear()


    // mouse over shows the client id - applicable to desktop version only
    layer.on('mouseover', function (e) {
        this.openPopup();
    });
    layer.on('mouseout', function (e) {
        this.closePopup();
    });

}
