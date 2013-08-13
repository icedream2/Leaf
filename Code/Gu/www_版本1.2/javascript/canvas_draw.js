
// Copyright 2010 William Malone (www.williammalone.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


var buf=0;
var canvas;
var context;
var canvasWidth = 200;
var canvasHeight = 200;
var padding = 25;
var lineWidth = 8;
var colorPurple = "#cb3594";
var colorGreen = "#659b41";
var colorYellow = "#ffcf33";
var colorBrown = "#986928";
var backgroundImage = new Image();
var clickX = new Array();
var clickY = new Array();
var new_clickX = new Array();
var new_clickY = new Array();
var clickColor = new Array();
var clickTool = new Array();
var clickSize = new Array();
var clickDrag = new Array();
var paint = false;
var curColor = colorPurple;
var curTool = "marker";
var curSize = "normal";
var drawingAreaX = 0;
var drawingAreaY = 0;
var drawingAreaWidth = 200;
var drawingAreaHeight = 200;
var sizeHotspotWidthObject = new Object();
sizeHotspotWidthObject.huge = 39;
sizeHotspotWidthObject.large = 25;
sizeHotspotWidthObject.normal = 18;
sizeHotspotWidthObject.small = 16;
var totalLoadResources = 1;
var curLoadResNum = 0;

var a=0,b=0,c=0,d=0;



/**
* Calls the redraw function after all neccessary resources are loaded.
*/
function resourceLoaded()
{
	if(++curLoadResNum >= totalLoadResources){	//document.getElementById('canvas').getContext("2d").drawImage(backgroundImage, 0,0,canvasWidth, canvasHeight);
		context.translate(Math.min(canvasHeight,canvasWidth)/2, Math.min(canvasHeight,canvasWidth)/2);
		//alert(backgroundImage.height+" : "+backgroundImage.width);
		//context.translate(160,160);
		context.rotate(90*Math.PI/180);
		context.translate(-Math.min(canvasHeight,canvasWidth)/2, -Math.min(canvasHeight,canvasWidth)/2);
		//context.translate(-160,-160);
		context.drawImage(backgroundImage,0,0,canvasWidth,canvasHeight);				
		
		/*
		context.translate(Math.min(canvasHeight,canvasWidth)/2, Math.min(canvasHeight,canvasWidth)/2);
		//alert(backgroundImage.height+" : "+backgroundImage.width);
		context.rotate(-90*Math.PI/180);
		context.translate(-Math.min(canvasHeight,canvasWidth)/2, -Math.min(canvasHeight,canvasWidth)/2);
		*/

		redraw();
	}
}

/**
* Creates a canvas element, loads images, adds events, and draws the canvas for the first time.
*/
function prepareCanvas(imgs)
{
/*
	var canvasDiv = document.getElementById('canvasDiv');
	canvas = document.createElement('canvas');
	canvas.setAttribute('width', 600);
	canvas.setAttribute('height', 400);
	canvas.setAttribute('id', 'canvas');
	canvas.setAttribute('style','border: 5px solid black');
	canvasDiv.appendChild(canvas);
	context = canvas.getContext("2d");
	//context.rotate(90*Math.PI/180);
	var img=document.getElementById("aaa");
	context.drawImage(img, 0,0);
	
	alert("a");
	//alert(Math.min(img.height,img.width)/2 + ":" + Math.min(img.height,img.width)/2);
	canvas.setAttribute('width', 400);
	canvas.setAttribute('height', 600);
	context.translate(Math.min(img.height,img.width)/2, Math.min(img.height,img.width)/2);
	context.rotate(90*Math.PI/180);
	//alert("a");
	context.translate(-Math.min(img.height,img.width)/2, -Math.min(img.height,img.width)/2);
	//context.drawImage(img, -Math.min(400,600)/2,-Math.min(400,600)/2);
	context.drawImage(img, 0,0);
	//alert("b");
	return true;
*/

	
	//initialize again
	clickX = new Array();
	clickY = new Array();
	new_clickX = new Array();
	new_clickY = new Array();
	clickColor = new Array();
	clickTool = new Array();
	clickSize = new Array();
	clickDrag = new Array();
	paint = false;
	sizeHotspotWidthObject = new Object();
	totalLoadResources = 1;
	curLoadResNum = 0;
	a=0;b=0;c=0;d=0;
	if(context)
		clearCanvas();
	
	
	var canvasDiv = document.getElementById('canvasDiv');
	if(!canvas){
		alert("you canvas");
		canvas = document.createElement('canvas');
		var w=window.screen.width;
		var h=window.screen.height;
		//var w=544;
		//var h=320;		
		canvas.setAttribute('width', canvasWidth);
		canvas.setAttribute('height', canvasHeight);
		canvas.setAttribute('id', 'canvas');
		canvas.setAttribute('style','border: 5px solid black');
		canvasDiv.appendChild(canvas);
		context = canvas.getContext("2d");
		
		//update height & width
		
		canvasHeight= w-2*20;
		canvasWidth = canvasHeight*4/3;
		drawingAreaHeight = canvasHeight;
		drawingAreaWidth = canvasWidth;
		canvas.setAttribute('width', canvasHeight);
		canvas.setAttribute('height', canvasWidth);
		
		// Load images
		backgroundImage.onload = function(){ 
			if(buf==0)
				resourceLoaded();
			else{
				context.drawImage(backgroundImage,0,0,canvasWidth,canvasHeight);
				redraw();
			}
		}
		backgroundImage.src=imgs;
	}
	else{
		buf=1;
		backgroundImage.src=imgs;
		++curLoadResNum;
	}
	
	//rotate picture
	//alert("a");
	// Add mouse events
	// ----------------
	$("#canvas").on("vmousedown",function(e)
	{
	//a++;
		//$("#mousedown").text(a);
		// Mouse down location
		// Mouse down location
		var mouseX = e.pageX - this.offsetLeft;
		var mouseY = e.pageY - this.offsetTop;
		$("#mousedown").text(mouseX + " : " +mouseY);
		//alert(mouseX + "  :  " + mouseY);
		paint = true;
		addClick(mouseX, mouseY, false);
		redraw();
	});
	
	$("#canvas").on("vmousemove",function(e){
	
		var mouseX = e.pageX - this.offsetLeft;
		var mouseY = e.pageY - this.offsetTop;
		$("#mousemove").text(mouseX + " : " +mouseY);	
		if(paint==true){
		//alert("a");
			addClick(e.pageX - this.offsetLeft, e.pageY - this.offsetTop, true);
			redraw();
		}
	});
	$("#canvas").on("vmouseup",function(e){

		paint = false;
	  	redraw();
	});
	
	$("#canvas").on("vmouseout",function(e){

		paint = false;
	});
	
	//alert("prepare done");
}

/**
* Adds a point to the drawing array.
* @param x
* @param y
* @param dragging
*/
function addClick(x, y, dragging)
{
	if(x>0&&y>0&&x<canvasHeight&&y<canvasWidth){
		clickX.push(x);
		clickY.push(y);
		new_clickX.push(y);
		new_clickY.push(canvasHeight-x);
		clickTool.push(curTool);
		clickColor.push(curColor);
		clickSize.push(curSize);
		clickDrag.push(dragging);
	}
}

/**
* Clears the canvas.
*/
function clearCanvas()
{
	context.clearRect(0, 0, canvasWidth, canvasHeight);
}

/**
* Redraws the canvas.
*/
function redraw()
{
	// Make sure required resources are loaded before redrawing
	if(curLoadResNum < totalLoadResources){ return; }
	
	clearCanvas();
	
	//context.drawImage(backgroundImage, 0, -canvasHeight);
	context.drawImage(backgroundImage, 0,0,canvasWidth,canvasHeight);
	// Keep the drawing in the drawing area
	context.save();
	context.beginPath();
	context.rect(drawingAreaX, drawingAreaY, drawingAreaWidth, drawingAreaHeight);
	context.clip();
		
	var radius;
	var i = 0;
	for(; i < clickX.length; i++)
	{		
		context.beginPath();
		if(clickDrag[i] && i){
			//context.moveTo(clickX[i-1], clickY[i-1]);
			context.moveTo(clickY[i-1],canvasHeight-clickX[i-1]);
		}else{
			//context.moveTo(clickX[i], clickY[i]);
			context.moveTo(clickY[i],canvasHeight-clickX[i]);
		}
		//context.lineTo(clickX[i], clickY[i]);
		context.lineTo(clickY[i],canvasHeight-clickX[i]);
		context.closePath();
		
		/*
		if(clickTool[i] == "eraser"){
			//context.globalCompositeOperation = "destination-out"; // To erase instead of draw over with white
			context.strokeStyle = 'white';
		}else{
			//context.globalCompositeOperation = "source-over";	// To erase instead of draw over with white
			context.strokeStyle = clickColor[i];
		}*/
		
		context.strokeStyle = clickColor[i];
		
		context.lineJoin = "round";
		context.lineWidth = radius;
		context.stroke();
		
	}
	//context.globalCompositeOperation = "source-over";// To erase instead of draw over with white
	context.restore();
	
	context.globalAlpha = 1; // No IE support
}
