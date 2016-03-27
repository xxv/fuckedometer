var background = new Image();
var canvas;
var ctx;
var x = 0;

var low = 51.5;
var high = 128.5;

var target_value = 0;
var current_value = 0;
var noise_max = 0.2;
var speed_mult = 2.5;
var max_v = 50;

var prev_frame_t = new Date().getTime();

function load_bg() {
  canvas = document.getElementById('meter');
  if (canvas.getContext) {
    ctx = canvas.getContext('2d');
  }

  background.onload = function() {
    window.requestAnimationFrame(draw);
    load();
  }

  background.src = "images/meter.png";
}

function draw() {
  if (ctx) {
    var now = new Date().getTime();
    var dt = (now - prev_frame_t)/1000;

    var v = Math.min(max_v, dt * speed_mult * (target_value - current_value));

    current_value += v;

    var noise = Math.random() * noise_max - noise_max/2;

    draw_meter(current_value + noise);
    prev_frame_t = now;
  }

  window.requestAnimationFrame(draw);
}

function load() {
  load_value(animate_to);
}

function load_value(onLoaded) {
  $.ajax("/fuckedometer", {"success": function (data) {

    var value = parseInt(data, 10);
   onLoaded(value);
  }});
  window.setTimeout(load, 60 * 1000);
}

function animate_to(value) {
  target_value = value;
}

function draw_meter(value) {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.drawImage(background, 0, 0);
  ctx.save();
  ctx.translate(canvas.width/2, canvas.height + 150);
  var angle = (value * (high - low))/100 + low;
  ctx.rotate(angle * Math.PI / 180);
  ctx.beginPath();
  ctx.moveTo(0, 0);
  ctx.lineTo(-300, 0);
  ctx.lineWidth = 4;
  ctx.strokeStyle = '#af0000';
  ctx.stroke();
  ctx.restore();
}

function update_target(){
  animate_to(Math.random() * 100);
  window.setTimeout(update_target, 5000);
}

$(document).ready(function() {
  load_bg();
});

