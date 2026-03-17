#include <Arduino.h>

// NeoPixel
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN 14
#define LED_COUNT 239
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t brightness = 100;

//

// Compiler
String programNames[4] = {"-", "-", "-", "-"};
String programs[4];
String tempString;

int16_t xv[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int8_t currentProgram = -1;
int32_t currentProgramLineG = 0;
int32_t sleepValue = 0;
int32_t sleepIndex = 0;
int8_t cursorIndex = 0;
//

// WIFI
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "router001";
const char *password = "001wifiaccess";
unsigned long previousCheck = 0;
const unsigned long checkInterval = 5000;
ESP8266WebServer server(80);
//

// HTML&CSS&JS

String helpText()
{
    String helpContent = R"rawliteral(
-'/set?r= &g= &b= &n='
  -'/fill?r= &g= &b= &s= &f='
  -'/clear'
  -'/cursor?n='
  
  -'/func?cmd=x&val=y'
    (x,y):
      add, 'ScriptName - Script'   'add the script which named 'ScriptName'
      del, Index                   'delete programs[n]'
      list,                        'show script names list'
      run, Index                   'run programs[n]'

  -SET(r.g.b.n):            'pixels[n] = (r,g,b)'
  -FILL(r.g.b.s.f):         'pixels[s->f] = (r,g,b)'
  -CLEAR():                 'pixels.clear();'
  -CURSOR(n):               'cursor->startIndexOffset'
  -SLEEP(t):                'delay(t);'
  -JMP(n):                  'Jump for n foward or backward (-+n), -1 to return current command'
  -IFJMP(x.v.n):            'if xv[x] == v: jump for n times (-+n)'
  -DEFINE(x.v):             'xv[x] = v'
  -ADD(x.v):                'xv[x] += v'
  -END():                   'current program to -1 (not running), currentline to 0'

)rawliteral";
    return helpContent;
}
String mainHTML()
{
    String htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>HSV to RGB LED Controller</title>
        <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/15.7.0/nouislider.min.css">
        <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined" />
        <link rel="stylesheet" href="server.css">
    </head>

    <body class="darkmode">

        <div class="container">
            
            <div class="top">
                
                <div style="margin-top: 40px;"></div>
                
                <!--colorBox-->
                <div class="color-box" id="colorBox" style="justify-self: center;"></div>
                <p id="label-a">RGB: <span id="rgbText">255, 0, 0</span></p>
                
                <div style="margin-top: 15px;"></div>

                <!--Sliders-->
                <button id="darkmodebutton">Theme</button>
                <div style="margin-top: 30px;"></div>

                <div>
                    
                    <label id="label-a" >Hue (0-360): <span id="hVal">0</span></label><br>
                    <input type="range" id="hue" min="0" max="360" value="0" class="slider"><br>
                </div>

                <label id="label-a" >Saturation (0-100%): <span id="sVal">100</span></label><br>
                <input type="range" id="sat" min="0" max="100" value="100" class="slider"><br>

                <label id="label-a" >Value (0-100%): <span id="vVal">100</span></label><br>
                <input type="range" id="val" min="0" max="100" value="100" class="slider"><br>

                <div style="margin-top: 15px;"></div>
            </div>


            <div class="middle">
                <!---->
                <label id="label-a" >Fill Range: <span id="fillRangeVal">0 - 10</span></label>
                <div id="fillRangeSlider" class="slider"></div><br>

                <label id="label-a" >Set Index: <span id="setIndexVal">0</span></label><br>
                <input type="range" id="index" min="0" max="238" value="0" class="slider"><br><br>
            </div>

            <div class="bottom">
                <button class="general" id="fillBtn">FILL</button>
                <button class="general" id="setBtn">SET</button>
                <button class="general" id="clearBtn">CLEAR</button>
            </div>

        </div>

        <script src="https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/15.7.0/nouislider.min.js"></script>
        <script src="server.js"></script>
    </body>
</html>

)rawliteral";
    return htmlContent;
}

String mainCSS()
{
    String cssContent = R"rawliteral(
:root{
  --base-color: white;
  --base-variant: #e8e9ed;
  --text-color: #111528;
  --secondary-text: #232738;
  --primary-color: #3a435d;
  --accent-color: #0071ff;
}
.darkmode{
  --base-color: #080a1b;
  --base-variant: #101425;
  --text-color: #ffffff;
  --secondary-text: #a4a5b8;
  --primary-color: #3a435d;
  --accent-color: #0071ff;
}

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body { 
    font-family: 'Nunito Sans', sans-serif;
    display: flex;
    justify-content: center;
    align-items: center;
    background:#000000;
}

div.container {
    border-style: solid;
    border-color: var(--secondary-text);
    border-width: 0px;
    width: 370px;
    height: 100%;
    background: var(--base-color);
    border-radius: 30px;
}

div.top {
    text-align: center;
    justify-content: center;
    align-items: center;
}

div.bottom {
    text-align: center;
    justify-content: center;
    align-items: center;
}

div.middle {
    text-align: center;
    justify-content: center;
    align-items: center;
}

.slider { 
    width: 300px; margin: 10px 0;
    margin-left: 30px;
}
.color-box { 
    width: 100px; height: 100px; margin-top: 20px; border: 1px solid #000; 
}
button.general { 
    font-size: 1.5em;
    font-weight: 500;
    margin: 5px; 
    width: 100px;
    height: 50px;
    border-radius: 3px;
    border-style: hidden;
    background-color: var(--accent-color);
    color: #ffffff;
}

#label-a {
    text-decoration: solid;
    font-size: 1.0em;
    font-weight: 700;
    color: var(--text-color);
}

#darkmodebutton {
    font-size: 1em;
    font-weight: 500;
    margin: 5px; 
    width: 100px;
    height: 50px;
    border-radius: 3px;
    border-style: hidden;
    background-color: var(--accent-color);
    color: #ffffff;
}
)rawliteral";
    return cssContent;
}

String mainJS()
{
    String cssContent = R"rawliteral(
// HSV → RGB conversion
function hsvToRgb(h, s, v) {
    s /= 100; v /= 100;
    let c = v*s, x = c*(1 - Math.abs((h/60)%2 -1)), m = v-c;
    let r=0,g=0,b=0;
    if(h>=0&&h<60){ r=c; g=x; b=0; }
    else if(h>=60&&h<120){ r=x; g=c; b=0; }
    else if(h>=120&&h<180){ r=0; g=c; b=x; }
    else if(h>=180&&h<240){ r=0; g=x; b=c; }
    else if(h>=240&&h<300){ r=x; g=0; b=c; }
    else if(h>=300&&h<360){ r=c; g=0; b=x; }
    r = Math.round((r+m)*255);
    g = Math.round((g+m)*255);
    b = Math.round((b+m)*255);
    return [r,g,b];
}

// Update preview
function updateColor() {
    let h=parseInt(document.getElementById('hue').value);
    let s=parseInt(document.getElementById('sat').value);
    let v=parseInt(document.getElementById('val').value);
    document.getElementById('hVal').textContent=h;
    document.getElementById('sVal').textContent=s;
    document.getElementById('vVal').textContent=v;
    let [r,g,b]=hsvToRgb(h,s,v);
    document.getElementById('colorBox').style.backgroundColor=`rgb(${r},${g},${b})`;
    document.getElementById('rgbText').textContent=`${r}, ${g}, ${b}`;
    return [r,g,b];
}

// Initialize Fill Range Slider (two handles)
var fillSlider = document.getElementById('fillRangeSlider');
noUiSlider.create(fillSlider, {
    start: [50, 200],
    connect: true,
    range: { min: 0, max: 238 },
    step: 1,
});
fillSlider.noUiSlider.on('update', function(values){
    document.getElementById('fillRangeVal').textContent = `${Math.round(values[0])} - ${Math.round(values[1])}`;
});

// Send XHR to ESP8266
function sendColor(action) {
    let xhr = new XMLHttpRequest();

    if(action === 'set'){
        let [r,g,b]=updateColor();
        let n=parseInt(document.getElementById('index').value)||0;
        let url=`/set?r=${r}&g=${g}&b=${b}&n=${n}`;
        xhr.open('GET', url, true);
    } 
    else if(action === 'fill'){
        let [r,g,b]=updateColor();
        let range = fillSlider.noUiSlider.get();
        let s=Math.round(range[0]), f=Math.round(range[1]);
        let url=`/fill?r=${r}&g=${g}&b=${b}&s=${s}&f=${f}`;
        xhr.open('GET', url, true);
    } 
    else if(action === 'clear'){
        xhr.open('GET','/clear', true);
    }

    xhr.onreadystatechange=function(){
        if(xhr.readyState===4 && xhr.status===200){
            console.log('ESP Response:', xhr.responseText);
        }
    };
    xhr.send();
}

const btn = document.getElementById('darkmodebutton');

  btn.addEventListener('click', () => {
    document.body.classList.toggle('darkmode');
  });

// Event listeners
document.getElementById('hue').addEventListener('input', updateColor);
document.getElementById('sat').addEventListener('input', updateColor);
document.getElementById('val').addEventListener('input', updateColor);
document.getElementById('index').addEventListener('input', ()=>document.getElementById('setIndexVal').textContent=document.getElementById('index').value);

document.getElementById('fillBtn').addEventListener('click', ()=>sendColor('fill'));
document.getElementById('setBtn').addEventListener('click', ()=>sendColor('set'));
document.getElementById('clearBtn').addEventListener('click', ()=>sendColor('clear'));

// Initialize
updateColor();

document.getElementById('setIndexVal').textContent=document.getElementById('index').value;
)rawliteral";
    return cssContent;
}

String editorHTML()
{
    String htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>ESP8266 Editor</title>
<style>
  body { font-family: Arial, sans-serif; padding: 20px; }
  textarea { width: 400px; height: 80px; margin-right: 10px; }
  input[type="number"] { width: 80px; margin-right: 10px; }
  button { margin: 5px; }
  #output { margin-top: 20px; white-space: pre-wrap; border: 1px solid #ccc; padding: 10px; max-height: 300px; overflow-y: auto; }
</style>
</head>
<body>

<h2>ESP8266 Editor</h2>

<!-- Add Item -->
<label>Add Item:</label><br>
<textarea id="nameInput" placeholder="Enter text here"></textarea><br>
<button id="addBtn">Add</button><br><br>

<!-- List Items -->
<button id="listBtn">List</button><br><br>

<!-- Delete, Read, Run -->
<label>Index:</label>
<input type="number" id="indexInput" min="0" value="0">
<button id="deleteBtn">Delete</button>
<button id="readBtn">Read</button>
<button id="runBtn">Run</button>

<div id="output"></div>

<script>
// Function to send GET requests
function sendRequest(url, callback){
    let xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.onreadystatechange=function(){
        if(xhr.readyState===4 && xhr.status===200){
            if(callback) callback(xhr.responseText);
        }
    };
    xhr.send();
}

// Add item
document.getElementById('addBtn').addEventListener('click', ()=>{
    let name = encodeURIComponent(document.getElementById('nameInput').value);
    if(name){
        sendRequest(`/func?cmd=add&val="${name}"`, (res)=>{
            document.getElementById('output').textContent = "Added:\n" + name + "\n" + res;
        });
    }
});

// List items
document.getElementById('listBtn').addEventListener('click', ()=>{
    sendRequest(`/func?cmd=list&val=0`, (res)=>{
        document.getElementById('output').textContent = "List:\n" + res;
    });
});

// Delete item
document.getElementById('deleteBtn').addEventListener('click', ()=>{
    let idx = parseInt(document.getElementById('indexInput').value)||0;
    sendRequest(`/func?cmd=del&val=${idx}`, (res)=>{
        document.getElementById('output').textContent = "Deleted index " + idx + "\n" + res;
    });
});

// Read item
document.getElementById('readBtn').addEventListener('click', ()=>{
    let idx = parseInt(document.getElementById('indexInput').value)||0;
    sendRequest(`/func?cmd=read&val=${idx}`, (res)=>{
        document.getElementById('output').textContent = "Read index " + idx + ":\n" + res;
    });
});

// Run item
document.getElementById('runBtn').addEventListener('click', ()=>{
    let idx = parseInt(document.getElementById('indexInput').value)||0;
    sendRequest(`/func?cmd=run&val=${idx}`, (res)=>{
        document.getElementById('output').textContent = "Run index " + idx + ":\n" + res;
    });
});
</script>

</body>
</html>

)rawliteral";
    return htmlContent;
}
//

// IR
#include "PinDefinitionsAndMore.h"
#include "TinyIRReceiver.hpp"
#define USE_EXTENDED_NEC_PROTOCOL

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif
bool pixelPower = true;
bool manualRGB = false;
uint8_t NPR = 0;
uint8_t NPG = 0;
uint8_t NPB = 0;

//

// NeoPixelSetup

void NeoPixelSetup()
{
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
#endif
    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
    pixels.show();  // Turn OFF all pixels ASAP
    pixels.setBrightness(brightness);
}

void colorWipe(uint32_t color, int wait)
{
    for (int i = 0; i < pixels.numPixels(); i++)
    {                                   // For each pixel in strip...
        pixels.setPixelColor(i, color); //  Set pixel's color (in RAM)
        pixels.show();                  //  Update strip to match
        delay(wait);                    //  Pause for a moment
    }
}
//

// WIFI

String removeWhitespace(String input)
{
    String result = "";
    for (int i = 0; i < input.length(); i++)
    {
        char c = input[i];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
        {
            result += c;
        }
    }
    return result;
}

void handleRoot()
{
    server.send(200, "text/html", mainHTML());
    Serial.println("requested: " + server.uri());
}

void handleCSS()
{
    server.send(200, "text/css", mainCSS());
    Serial.println("requested: " + server.uri());
}
void handleJS()
{
    server.send(200, "text/javascript", mainJS());
    Serial.println("requested: " + server.uri());
}
void handleEditor()
{
    server.send(200, "text/html", editorHTML());
    Serial.println("requested: " + server.uri());
}

void handleHelp(){
    server.send(200, "text/plain", helpText());
    Serial.println("requested: " + server.uri());
}

void setHandle()
{
    if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b") && server.hasArg("n"))
    {
        pixels.setPixelColor(cursorIndex + server.arg("n").toInt(), pixels.Color(server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt()));
        pixels.show();
        server.send(200, "text/plain", "OK.");
    }
    else
    {
        server.send(200, "text/plain", "ERROR.");
    }
}

void fillHandle()
{
    if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b") && server.hasArg("s") && server.hasArg("f"))
    {

        pixels.fill(pixels.Color(server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt()), cursorIndex + server.arg("s").toInt(), server.arg("f").toInt() - (server.arg("s").toInt() + cursorIndex) + 1);
        pixels.show();
        server.send(200, "text/plain", "OK.");
    }
    else
    {
        server.send(200, "text/plain", "ERROR.");
    }
}
void clearHandle()
{
    pixels.clear();
    pixels.show();
    server.send(200, "text/plain", "OK.");
}

void cursorHandle()
{
    if (server.hasArg("n"))
    {
        cursorIndex = (int8_t)server.arg("n").toInt();
        server.send(200, "text/plain", "OK.");
    }
    else
    {
        server.send(200, "text/plain", "OK.");
    }
}

void funcHandle()
{
    if (server.hasArg("cmd") && server.hasArg("val"))
    {
        String ret;
        String cmd = server.arg("cmd");
        String val = server.arg("val");
        if (cmd == "list")
        {
            ret = "{" + String(programNames[0]) + ',' + String(programNames[1]) + ',' + String(programNames[2]) + ',' + String(programNames[3]) + "}";
            server.send(200, "text/plain", ret);
        }
        else if (cmd == "add")
        {
            int8_t index = -1;
            for (int8_t i = 0; i < 3; i++)
            {
                if (programNames[i] == "-")
                {
                    index = i;
                    break;
                }
            }
            if (index == -1)
            {
                server.send(200, "text/plain", "ERROR: NO SPACE TO ADD.");
            }
            else
            {
                programs[index] = server.arg("val").substring(server.arg("val").indexOf('-') + 1);
                programNames[index] = server.arg("val").substring(0, server.arg("val").indexOf('-'));
                programs[index] = removeWhitespace(programs[index]);
                server.send(200, "text/plain", "OK.");
            }
        }
        else if (cmd == "del")
        {
            programs[server.arg("val").toInt()] = "";
            programNames[server.arg("val").toInt()] = "-";
            server.send(200, "text/plain", "OK.");
        }
        else if (cmd == "read")
        {
            ret = String(programNames[server.arg("val").toInt()]) + '-' + String(programs[server.arg("val").toInt()]);
            server.send(200, "text/plain", ret);
        }
        else if (cmd == "run")
        {
            currentProgram = server.arg("val").toInt();
            currentProgramLineG = 0;
            server.send(200, "text/plain", "OK.");
        }
    }
    else
    {
        server.send(200, "text/plain", "OK.");
    }
}

void endHandle()
{
    currentProgram = -1;
    currentProgramLineG = 0;
    server.send(200, "text/plain", "OK.");
}

void resetHandle()
{
    pixels.clear();
    pixels.show();
    server.send(200, "text/plain", "OK.");
    ESP.restart();
}

void brightHandle()
{
    if (server.hasArg("n"))
    {
        if (server.arg("n").toInt() < 256 && server.arg("n").toInt() > 0)
        {
            brightness = server.arg("n").toInt();
            pixels.setBrightness(brightness);
            pixels.show();
            server.send(200, "text/plain", "OK.");
        }
        else
        {
            server.send(200, "text/plain", "ERROR: 'n' must between 0,220.");
        }
    }
    else
    {
        server.send(200, "text/plain", "ERROR: 'n' not found.");
    }
}

void handleNotFound()
{
    Serial.print("Unknown request: ");
    Serial.println(server.uri()); // Print unknown request path
    server.send(404, "text/plain", "Not found");
}
void wifiSetup()
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    server.on("/", handleRoot);
    server.on("/server.css", handleCSS);
    server.on("/server.js", handleJS);
    server.on("/help", handleHelp);
    server.on("/editor", handleEditor);
    server.on("/set", setHandle);
    server.on("/fill", fillHandle);
    server.on("/clear", clearHandle);
    server.on("/cursor", cursorHandle);
    server.on("/func", funcHandle);
    server.on("/end", endHandle);
    server.on("/reset", resetHandle);

    server.on("/brightness", brightHandle);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    String ip = WiFi.localIP().toString();
    Serial.println(ip);
    ip = ip.substring(ip.lastIndexOf('.') + 1);
    Serial.println(ip);
    pixels.clear();
    int lastIndex = 0;
    for (int i = 0; i < ip.length(); i++)
    {
        pixels.setPixelColor(lastIndex, pixels.Color(0, 0, 255));
        pixels.show();
        lastIndex += 1;
        for (int i2 = 0; i2 < (ip[i] - '0'); i2++)
        {
            Serial.println(lastIndex);
            pixels.setPixelColor(lastIndex, pixels.Color(0, 255, 0));
            pixels.show();
            lastIndex++;
        }
        pixels.setPixelColor(lastIndex, pixels.Color(0, 0, 255));
        pixels.show();
    }
}

//

void IRLoop();

// NeoPixel
void IRToNeoPixel(uint8_t cmd)
{
    if (cmd == 2)
    {
        pixelPower = false;
        pixels.clear();
        pixels.show();
    }
    if (cmd == 3)
    {
        pixelPower = true;
    }
    if (pixelPower)
    {
        if (manualRGB)
        {
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
            switch (cmd)
            {
            case 15:
                manualRGB = false;
                break;
            case 4:
                if (NPR + 5 < 255)
                {
                    NPR += 5;
                }
                else
                {
                    NPR = 255;
                }
                break;
            case 5:
                if (NPG + 5 < 255)
                {
                    NPG += 5;
                }
                else
                {
                    NPG = 255;
                }
                break;
            case 6:
                if (NPB + 5 < 255)
                {
                    NPB += 5;
                }
                else
                {
                    NPB = 255;
                }
                break;
            case 8:
                if (NPR - 5 > 0)
                {
                    NPR -= 5;
                }
                else
                {
                    NPR = 0;
                }
                break;
            case 9:
                if (NPG - 5 > 0)
                {
                    NPG -= 5;
                }
                else
                {
                    NPG = 0;
                }
                break;
            case 10:
                if (NPB - 5 > 0)
                {
                    NPB -= 5;
                }
                else
                {
                    NPB = 0;
                }
                break;
            }
            pixels.clear();
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
            pixels.fill(pixels.Color(NPR, NPG, NPB), 1, LED_COUNT);
            Serial.println(NPR);
            Serial.println(NPG);
            Serial.println(NPB);
            pixels.show();
        }
        else
        {
            switch (cmd)
            {
            case 0:
                if (brightness + 5 < 230)
                {
                    brightness += 5;
                }
                else
                {
                    brightness = 230;
                }
                Serial.print("B:");
                Serial.println(brightness);
                pixels.setBrightness(brightness);
                pixels.show();
                break;
            case 1:
                if (brightness - 5 > 0)
                {
                    brightness -= 5;
                }
                else
                {
                    brightness = 0;
                }
                Serial.print("B:");
                Serial.println(brightness);
                pixels.setBrightness(brightness);
                pixels.show();
                break;

            case 4:
                NPR = 255;
                NPG = 0;
                NPB = 0;
                break;
            case 5:
                NPR = 0;
                NPG = 255;
                NPB = 0;
                break;
            case 6:
                NPR = 0;
                NPG = 0;
                NPB = 255;
                break;
            case 7:
                NPR = 255;
                NPG = 255;
                NPB = 255;
                break;
            case 8:
                NPR = 255;
                NPG = 65;
                NPB = 0;
                break;
            case 9:
                NPR = 157;
                NPG = 255;
                NPB = 0;
                break;
            case 10:
                NPR = 0;
                NPG = 140;
                NPB = 255;
                break;
            case 12:
                NPR = 255;
                NPG = 119;
                NPB = 0;
                break;
            case 13:
                NPR = 48;
                NPG = 242;
                NPB = 233;
                break;
            case 14:
                NPR = 123;
                NPG = 48;
                NPB = 242;
                break;
            case 15:
                manualRGB = true;
                pixels.setPixelColor(0, pixels.Color(255, 0, 0));
                break;
            case 20:
                NPR = 255;
                NPG = 139;
                NPB = 15;
                break;
            case 19:
                pixels.clear();
                pixels.fill(pixels.Color(255, 255, 255), 0, 40);
                pixels.fill(pixels.Color(255, 255, 255), 94, 74);
                pixels.fill(pixels.Color(255, 255, 255), 208, 30);
                pixels.show();
                return;
                break;
            }
            pixels.clear();
            if (!manualRGB)
            {
                pixels.fill(pixels.Color(NPR, NPG, NPB), 0, LED_COUNT);
            }
            Serial.println(NPR);
            Serial.println(NPG);
            Serial.println(NPB);
            pixels.show();
        }
    }
    else
    {
        if (cmd == 11)
        {
            String ip = WiFi.localIP().toString();
            Serial.println(ip);
            ip = ip.substring(ip.lastIndexOf('.') + 1);
            Serial.println(ip);
            pixels.clear();
            int lastIndex = 0;
            for (int i = 0; i < ip.length(); i++)
            {
                pixels.setPixelColor(lastIndex, pixels.Color(0, 0, 255));
                pixels.show();
                lastIndex += 1;
                for (int i2 = 0; i2 < (ip[i] - '0'); i2++)
                {
                    Serial.println(lastIndex);
                    pixels.setPixelColor(lastIndex, pixels.Color(0, 255, 0));
                    pixels.show();
                    lastIndex++;
                }
                pixels.setPixelColor(lastIndex, pixels.Color(0, 0, 255));
                pixels.show();
            }
        }
    }
    Serial.print("manual:");
    Serial.println(manualRGB);
}
//

// IR
void IRSetup()
{
    pinMode(2, OUTPUT);
    digitalWrite(2, 0);
    Serial.println();
    if (!initPCIInterruptForTinyReceiver())
    {
        Serial.println(F("No interrupt available for pin " STR(IR_RECEIVE_PIN))); // optimized out by the compiler, if not required :-)
    }
    Serial.println(F("Ready to receive NEC IR signals at pin " STR(IR_RECEIVE_PIN)));
}
void IRLoop()
{
    if (TinyReceiverDecode())
    {
        uint8_t lastIRCommand = TinyIRReceiverData.Command;
        Serial.print("IR Command:");
        Serial.println(lastIRCommand);
        digitalWrite(2, 1);
        delay(100);
        IRToNeoPixel(TinyIRReceiverData.Command);
        digitalWrite(2, 0);
    }
}
//

// Compiler
void getArgs(char *value, int32_t *args)
{
    tempString = value;
    int8_t pointIndex = -1;
    int8_t cArg = 0;
    while (true)
    {
        String tmpArg = tempString.substring(pointIndex + 1, tempString.indexOf('.', pointIndex + 1));
        if (tmpArg.indexOf('x') != -1)
        {
            int xIndex = (int)tmpArg.substring(1).toInt();
            args[cArg] = xv[xIndex];
        }
        else
        {
            args[cArg] = (int32_t)tmpArg.toInt();
        }
        pointIndex = tempString.indexOf('.', pointIndex + 1);
        cArg += 1;
        if (pointIndex == -1)
        {
            break;
        }
    }
}

void compiler(char command[8], char value[20], int32_t S3)
{
    int32_t args[5] = {0, 0, 0, 0, 0};
    String cmd = String(command);
    getArgs(value, args);

    if (cmd == "SET")
    {
        Serial.println("SET " + String(cursorIndex + args[3]) + ":" + String(args[0]) + ',' + String(args[1]) + ',' + String(args[2]));
        pixels.setPixelColor(cursorIndex + args[3], pixels.Color(args[0], args[1], args[2]));
        pixels.show();
        currentProgramLineG = S3;
    }
    else if (cmd == "FILL")
    {
        Serial.println("Fill from  " + String(cursorIndex + args[3]) + " to " + String(cursorIndex + args[4]) + ':' + String(args[0]) + ',' + String(args[1]) + ',' + String(args[2]));
        pixels.fill(pixels.Color(args[0], args[1], args[2]), cursorIndex + args[3], args[4] - (args[3] + cursorIndex) + 1);
        pixels.show();
        currentProgramLineG = S3;
    }
    else if (cmd == "CLEAR")
    {
        pixels.clear();
        Serial.println("Cleared.");
        pixels.show();
        currentProgramLineG = S3;
    }
    else if (cmd == "CURSOR")
    {
        cursorIndex = args[0];
        Serial.println("New Cursor: " + String(cursorIndex));
        currentProgramLineG = S3;
    }
    else if (cmd == "SLEEP")
    {
        sleepValue = args[0];
        sleepIndex = millis();
        Serial.println("Sleeping for" + String(sleepValue));
        currentProgramLineG = S3;
    }
    else if (cmd == "JMP")
    {
        Serial.print("JumpedFrom " + String(currentProgramLineG) + " to ");
        int32_t lastIndex = S3;
        for (int16_t i = 0; i < abs(args[0]); i++)
        {
            if (args[0] < 0)
            {
                lastIndex = programs[currentProgram].lastIndexOf(':', lastIndex - 1);
            }
            else if (args[0] > 0)
            {
                lastIndex = programs[currentProgram].indexOf(':', lastIndex + 1);
            }
            if (lastIndex == -1)
            {
                lastIndex = 0;
            }
        }
        currentProgramLineG = lastIndex + 1;
        Serial.println(currentProgramLineG);
    }
    else if (cmd == "IFJMP")
    {
        args[0] = xv[args[0]];
        if (args[0] == args[1])
        {
            Serial.println("Jumped.");
            int32_t lastIndex = S3;
            for (int16_t i = 0; i < abs(args[2]); i++)
            {
                if (args[0] < 0)
                {
                    lastIndex = programs[currentProgram].lastIndexOf(':', lastIndex);
                }
                else if (args[0] > 0)
                {
                    lastIndex = programs[currentProgram].indexOf(':', lastIndex + 1);
                }
                if (lastIndex == -1)
                {
                    lastIndex = 0;
                }
            }
            currentProgramLineG = lastIndex + 1;
        }
        else
        {
            currentProgramLineG = S3;
            Serial.println("Continue.");
        }
    }
    else if (cmd == "DEFINE")
    {
        Serial.println("xv[" + String(args[0]) + ']' + " to " + String(args[1]));
        xv[args[0]] = args[1];
        currentProgramLineG = S3;
    }
    else if (cmd == "ADD")
    {
        Serial.println("xv[" + String(args[0]) + ']' + " to " + String(xv[args[0]] + args[1]));
        xv[args[0]] += args[1];
        currentProgramLineG = S3;
    }
    else if (cmd == "END")
    {
        Serial.println("Ended.");
        currentProgram = -1;
        currentProgramLineG = 0;
    }
}

void ScriptReader(String Script, int32_t currentProgramLine)
{
    int32_t S1 = Script.indexOf('(', currentProgramLine);
    int32_t S2 = Script.indexOf("):", S1);
    int32_t S3 = S2 + 2;
    char command[8];
    char value[20];
    Script.substring(currentProgramLine, S1).toCharArray(command, sizeof(command));
    ;
    Script.substring(S1 + 1, S2).toCharArray(value, sizeof(value));
    compiler(command, value, S3);
}

bool SleepHandler()
{
    return millis() - sleepIndex > sleepValue;
}

//

void setup()
{
    Serial.begin(115200);
    NeoPixelSetup();
    wifiSetup();
    IRSetup();
}

void loop()
{

    IRLoop();
    server.handleClient();
    if (currentProgram != -1)
    {
        if (SleepHandler())
        {
            ScriptReader(programs[currentProgram], currentProgramLineG);
        }
    }

    if (millis() - previousCheck >= checkInterval)
    {
        previousCheck = millis();
        if (WiFi.status() != WL_CONNECTED)
        {
            pixels.clear();
            pixels.fill(pixels.Color(0, 255, 0), 0, 20);
            pixels.fill(pixels.Color(0, 0, 255), 20, 20);
            pixels.show();
            Serial.println("WiFi disconnected! Restarting WiFi...");
            WiFi.disconnect();
            wifiSetup();
        }
    }
}
