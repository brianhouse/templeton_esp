var select_start = null;
var select_stop = null;
var min_t = null;
var max_t = null;

// prevent scrolling
document.ontouchmove = function(event) {
    event.preventDefault();
}

function setup() {
    console.log("Hello world");
    var canvas = createCanvas(windowWidth, windowHeight);
    canvas.parent('p5');
    background(255, 204, 0);

    if (data.length == 0) {
        return;
    }

    min_t = data[0]['t'];
    max_t = data[data.length - 1]['t'];

    var prev_x = 0;
    var prev_y = 0.5;
    stroke(0, 0, 0, 128);
    for (var i in data) {
        entry = data[i];
        x = ((entry['t'] - min_t) / (max_t - min_t)) * width;
        y = ((entry['mag'] / 14.0) + 0.5) * height;
        line(prev_x, prev_y, x, y);
        prev_x = x;
        prev_y = y;
    }

    var prev_x = 0;
    var prev_y = height/2;
    stroke(255, 0, 0, 255);
    for (var i in data) {
        entry = data[i];
        x = ((entry['t'] - min_t) / (max_t - min_t)) * width;
        y = (1.0 - (entry['bat'] / 105)) * height;
        line(prev_x, prev_y, x, y);
        prev_x = x;
        prev_y = y;
    }

    var prev_x = 0;
    var prev_y = height/2;
    stroke(0, 0, 255, 255);
    for (var i in data) {
        entry = data[i];
        x = ((entry['t'] - min_t) / (max_t - min_t)) * width;
        y = ((abs(entry['rssi']) - 25) / 90) * height;
        line(prev_x, prev_y, x, y);
        prev_x = x;
        prev_y = y;
    }
    stroke(0, 0, 0, 64);
    textFont("monospace", 12);
    text(start_segment, 30, 130);
    text(stop_segment, 30, 140);
}

function draw() {
    stroke(0, 0, 0, 0);
    fill(255, 204, 0);
    rect(0, 0, width, 20); 
    fill(0, 0, 0);
    if (select_start != null) {
        rect(select_start, 0, select_stop - select_start, 20); 
    }
}

function mousePressed() {
    select_start = mouseX;
    return false;
}

function mouseDragged() {
    select_stop = mouseX;
    return false;
}

function mouseReleased() {
    var start = 0.0;
    var stop = 1.0;
    if (select_start < select_stop) {
        start = select_start / width;
        stop = select_stop / width;
    } else {
        start = select_stop / width;
        stop = select_start / width;
    }

    console.log(start);
    console.log(stop);

    var duration = max_t - min_t;
    var start_t = int(min_t + (start * duration));
    var stop_t = int(min_t + (stop * duration));
    console.log(start_t);
    console.log(stop_t);

    select_start = null;
    select_stop = null;    

    window.location = "/" + collar_id + "/" + start_t + "/" + stop_t;

    return false;
}


// function windowResized() {
//   resizeCanvas(windowWidth, windowHeight);
// }