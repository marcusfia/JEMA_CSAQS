 $('#modChart').on('shown.bs.modal', function(event) {

    // get time 24 hours ago
    var date = new Date()
    past24 = Math.floor(date.getTime()/1000) - 86400








    var d3_container_ids = [
        '#d3_container_temps',
        '#d3_container_humidity',
        '#d3_container_pressure',
        '#d3_container_vcc',
        '#d3_container_voc',
        '#d3_container_nox'
    ]

    const query_terms = [
        'temps',
        'humidity',
        'pressure',
        'vcc',
        'voc',
        'nox'
    ]

    const chart_titles = [
        'Temperature (' + String.fromCharCode(176) +'F) - (BME680)',
        'Relative Humidity (%) -(BME680)',
        'Barometric Pressure (mb) -(BME680)',
        'Device Voltage (V)',
        'VOC sensor - BME680-(Resistence - Ohms)',
        'Nitrogen Oxides - (MICS-2714) - Voltage'
    ]











    // All sensors plots except MAX30105 (see below for MAX30105)

    var chart_title_selection;
    var clientID = $('.flex-clientID').text();
    for (var i = 0; i < query_terms.length; i++) {
        var url = "https://bnujqsc1g1.execute-api.us-west-2.amazonaws.com/prod/clientID/" + clientID + "/" + query_terms[i] + "?t=" + past24

        // Set dimensions of the canvas
        var margin = {top: 17, right: 10, bottom: 20, left: 35},
            width = $('.modal-body').width() - margin.left - margin.right,
            height = 210 - margin.top - margin.bottom;

        // Parse date & time
        var parse  = d3.timeParse("%s");

        // Add svg canvas
        const svg = d3.select(d3_container_ids[i])
            .append("svg")
                .attr("width", width + margin.left + margin.right)
                .attr("height", height + margin.top + margin.bottom)
                .attr("font-size", '10px')
            .append("g")
                .attr("transform", 
                      "translate(" + margin.left + "," + margin.top + ")");



              d3.csv(url).then(function(data) {

                    // used for determining if chart y axis/data needs to be scaled
                    temps = false;
                    voc = false;
                    baro = false;
                    vcc = false;

                    data.forEach(function(d) {
                        //select time column data
                        d.time = parse(d.time);
                        //select second column data
                        d.columnVal = +d[Object.keys(d)[1]];

                        if(Object.keys(d)[1] == 'temps') {
                            temps = true;
                        } else if(Object.keys(d)[1] == 'voc') {
                            voc = true;
                        } else if(Object.keys(d)[1] == 'pressure') {
                            baro = true;
                        } else if(Object.keys(d)[1] == 'vcc') {
                            vcc = true;
                        } else {
                            voc = false;
                            temps = false;
                            baro = false;
                        }

                    });

                    // Add X axis --> it is a date format
                    var x = d3.scaleTime()
                      .domain(d3.extent(data, function(d) { return d.time; }))
                      .range([ 0, width ]);
                    svg.append("g")
                      .attr("transform", "translate(0," + height + ")")
                      .call(d3.axisBottom(x).ticks(5));

                    // Add Y axis params for transforming data
                    var scalar
                    var sum
                    var lower_bound = 0

                    // conditional scalar values for y axis/data
                    if(temps){
                        scalar = 1.8
                        sum = 32
                        lower_bound = -25
                    } else if(voc) {
                        scalar = 0.0001
                        sum = 0
                    } else if(baro) {
                        scalar = 1
                        sum = 0
                        lower_bound = 950
                    } else if(vcc) {
                        scalar = 1
                        sum = 0
                        lower_bound = 3    
                    } else {
                        scalar = 1
                        sum = 0
                    }

                    var y = d3.scaleLinear()
                      .domain([lower_bound, d3.max(data, function(d) { 
                        return ( d[Object.keys(d)[1]] * scalar + sum ) + ( (d[Object.keys(d)[1]] * scalar + sum ) * .1) })])
                      .range([ height, 0 ]);
                    svg.append("g")
                      .call(d3.axisLeft(y).ticks(12));

                    var numberOfTicks = 14;

                    var yAxisGrid = d3.axisRight(y)
                      .ticks(numberOfTicks) 
                      .tickSize(width, 0)
                      .tickFormat("")
                    
                    numberOfTicks = 7; 

                    var xAxisGrid = d3.axisTop(x)
                      .ticks(numberOfTicks)
                      .tickSize(-height, 0)
                      .tickFormat("")


                    svg.append("g")
                      .classed('y', true)
                      .classed('axis', true)
                      .call(yAxisGrid);

                    svg.append("g")
                      .classed('x', true)
                      .classed('axis', true)
                      .call(xAxisGrid);
                      

                    svg.append("path")
                      .datum(data)
                      .attr("fill", "none")
                      .attr("stroke", "steelblue")
                      .attr("stroke-width", 1.5)
                      .attr("d", d3.line()
                        .x(function(d) { return x(d.time) })
                        .y(function(d) { return y(d[Object.keys(d)[1]] * scalar + sum )})
                    )

            })
            chart_title_selection = d3_container_ids[i] + ' svg'
            d3.select(chart_title_selection)
              .append("text")
              .attr("x", (width/1.75))             
              .attr("y", 12)
              .attr("text-anchor", "middle")  
              .text(chart_titles[i])
              .attr("style", "font-size:15px")
        }


















        // MAX30105 plot (R G B data, multi-line plot)

        var url = " *** URL FOR QUERY REQUESTS HERE *** " + clientID + "/particulate?t=" + past24

        // Set dimensions of the canvas
        var margin = {top: 17, right: 10, bottom: 20, left: 35},
            width = $('.modal-body').width() - margin.left - margin.right,
            height = 210 - margin.top - margin.bottom;

        // Parse date & time
        var parse  = d3.timeParse("%s");

        // Add svg canvas
        const svg = d3.select('#d3_container_PM')
            .append("svg")
                .attr("width", width + margin.left + margin.right)
                .attr("height", height + margin.top + margin.bottom)
                .attr("font-size", '10px')
            .append("g")
                .attr("transform", 
                      "translate(" + margin.left + "," + margin.top + ")");

              d3.csv(url).then(function(data) {

                    data.forEach(function(d) {
                        //select time column data
                        d.time = parse(d.time);
                        //select other column data
                        d.red = +d[Object.keys(d)[1]];
                        d.green = +d[Object.keys(d)[2]];
                        d.IR = +d[Object.keys(d)[3]];

                    });

                    // Add X axis --> it is a date format
                    var x = d3.scaleTime()
                      .domain(d3.extent(data, function(d) { return d.time; }))
                      .range([ 0, width ]);
                    svg.append("g")
                      .attr("transform", "translate(0," + height + ")")
                      .call(d3.axisBottom(x).ticks(5));

                    var y = d3.scaleLinear()
                      .domain([-50, d3.max(data, function(d) { 
                        return Math.max( (d[Object.keys(d)[1]] + (d[Object.keys(d)[1]] * .1)), (d[Object.keys(d)[2]] + (d[Object.keys(d)[2]] * .1)), (d[Object.keys(d)[3]] + (d[Object.keys(d)[3]] * .1)) )})])
                      .range([ height, 0 ]);
                    svg.append("g")
                      .call(d3.axisLeft(y).ticks(12));

                    var numberOfTicks = 14;

                    var yAxisGrid = d3.axisRight(y)
                      .ticks(numberOfTicks) 
                      .tickSize(width, 0)
                      .tickFormat("")
                    
                    numberOfTicks = 7; 

                    var xAxisGrid = d3.axisTop(x)
                      .ticks(numberOfTicks) 
                      .tickSize(-height, 0)
                      .tickFormat("")


                    svg.append("g")
                      .classed('y', true)
                      .classed('axis', true)
                      .call(yAxisGrid);

                    svg.append("g")
                      .classed('x', true)
                      .classed('axis', true)
                      .call(xAxisGrid);
                      

                    svg.append("path")
                      .datum(data)
                      .attr("fill", "none")
                      .attr("stroke", "red")
                      .attr("stroke-width", 1.5)
                      .attr("d", d3.line()
                        .x(function(d) { return x(d.time) })
                        .y(function(d) { return y(d[Object.keys(d)[1]])})
                    )

                    svg.append("path")
                      .datum(data)
                      .attr("fill", "none")
                      .attr("stroke", "green")
                      .attr("stroke-width", 1.5)
                      .attr("d", d3.line()
                        .x(function(d) { return x(d.time) })
                        .y(function(d) { return y(d[Object.keys(d)[2]])})
                    )

                    svg.append("path")
                      .datum(data)
                      .attr("fill", "none")
                      .attr("stroke", "purple")
                      .attr("stroke-width", 1.5)
                      .attr("d", d3.line()
                        .x(function(d) { return x(d.time) })
                        .y(function(d) { return y(d[Object.keys(d)[3]])})
                    ) 

            })
            chart_title_selection = '#d3_container_PM' + ' svg'
            d3.select(chart_title_selection)
              .append("text")
              .attr("x", (width/1.75))             
              .attr("y", 12)
              .attr("text-anchor", "middle")  
              .text("MAX30105 Raw Optical Data")
              .attr("style", "font-size:15px")
        


    }).on('hidden.bs.modal', function(event){
    // reset canvas size
    d3.selectAll(".carousel-inner svg > *").remove();
    $('.carousel-inner svg').remove(); 
    // destroy modal
    $(this).data('bs.modal', null);
});
