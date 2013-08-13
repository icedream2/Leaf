
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
var colorPurple = "#cb3594";
var backgroundImage = new Image();
var clickX = new Array();
var clickY = new Array();
var new_clickX = new Array();
var new_clickY = new Array();
var clickDrag = new Array();
var paint = false;
var drawingAreaX = 0;
var drawingAreaY = 0;
var drawingAreaWidth = 200;
var drawingAreaHeight = 200;
var imageHeight;
var rate;


/**
* Calls the redraw function after all neccessary resources are loaded.
*/
function resourceLoaded()
{
		context.translate(Math.min(canvasHeight,canvasWidth)/2, Math.min(canvasHeight,canvasWidth)/2);
		context.rotate(90*Math.PI/180);
		context.translate(-Math.min(canvasHeight,canvasWidth)/2, -Math.min(canvasHeight,canvasWidth)/2);
		context.drawImage(backgroundImage,0,0,canvasWidth,canvasHeight);				
		imageHeight=backgroundImage.height;
		rate=imageHeight/canvasHeight;
		alert("imageHeight="+imageHeight);
		alert("first rate="+rate);
		redraw();
}

/**
* Creates a canvas element, loads images, adds events, and draws the canvas for the first time.
*/
function prepareCanvas(imgs)
{

	//initialize again
	clickX.length=0;
	clickY.length=0;
	new_clickX.length=0;
	new_clickY.length=0;
	clickDrag.length=0;
	paint = false;

	if(context)
		context.clearRect(0, 0, canvasWidth, canvasHeight);	
	
	var canvasDiv = document.getElementById('canvasDiv');
	if(!canvas){
		//alert("you canvas");
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
		imageHeight=backgroundImage.height;
		$("#canvas").on("vmousedown",function(e)
		{
			paint = true;
			addClick( e.pageX - this.offsetLeft, e.pageY - this.offsetTop, false);
			redraw();
		});
		
		$("#canvas").on("vmousemove",function(e){
			if(paint==true){
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
		
	}
	else{
		buf=1;
		backgroundImage.src=imgs;
	}
}

/**
* Adds a point to the drawing array.
* @param x
* @param y
* @param dragging
*/
function addClick(x, y, dragging)
{
	if(x>0&&y>0 && x<canvasHeight && y<canvasWidth){
		clickX.push(x);
		clickY.push(y);
		new_clickX.push(parseInt(y*rate));
		new_clickY.push(parseInt((canvasHeight-x)*rate));
		clickDrag.push(dragging);
	}
}

/**
* Redraws the canvas.
*/
function redraw()
{
	// Make sure required resources are loaded before redrawin
	context.clearRect(0, 0, canvasWidth, canvasHeight);	
	
	//context.drawImage(backgroundImage, 0, -canvasHeight);
	context.drawImage(backgroundImage, 0,0,canvasWidth,canvasHeight);
	// Keep the drawing in the drawing area
	context.save();
	context.beginPath();
	context.rect(drawingAreaX, drawingAreaY, drawingAreaWidth, drawingAreaHeight);
	context.clip();
		
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
	
		context.strokeStyle = colorPurple;
		context.lineJoin = "round";
		context.lineWidth = 5;
		context.stroke();
		
	}
	//context.globalCompositeOperation = "source-over";// To erase instead of draw over with white
	context.restore();
	
	context.globalAlpha = 1; // No IE support
}
