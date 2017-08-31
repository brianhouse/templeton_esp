var gestures = {};
var collection = "test";

// prevent scrolling
document.ontouchmove = function(event) {
    event.preventDefault();
}

function setup() {
    console.log("Hello world");
    var canvas = createCanvas(windowWidth, windowHeight);
    canvas.parent('p5');
    background(255, 204, 0);

    var prev_x = 0;
    var prev_y = 0.5;
    for (var i in data) {
        entry = data[i];
        x = entry['t'] * width;
        y = ((entry['mag'] / 7.0) + 0.5) * height;
        line(prev_x, prev_y, x, y);
        // point(x, y);
        prev_x = x;
        prev_y = y;
    }
}

function draw() {

}

function windowResized() {
  resizeCanvas(windowWidth, windowHeight);
}