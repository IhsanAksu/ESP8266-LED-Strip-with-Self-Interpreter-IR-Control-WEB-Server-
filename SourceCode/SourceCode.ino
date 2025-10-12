#include <Arduino.h>

//NeoPixel
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN   14 
#define LED_COUNT 239
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t brightness = 100;

//

//Compiler
String programNames[4] = {"-","-","-","-"};
String programs[4];
String tempString;

int16_t xv[] = {0,0,0,0,0,0,0,0,0,0};

int8_t currentProgram = -1;
int32_t currentProgramLineG = 0;
int32_t sleepValue = 0;
int32_t sleepIndex = 0;
int8_t cursorIndex = 0;
//

//WIFI
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "ssid";
const char* password = "password";
ESP8266WebServer server(80);
//

//HTML&CSS&JS
String htmlContentF(){
String htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>HSV to RGB LED Controller</title>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/15.7.0/nouislider.min.css">
<style>
  body { font-family: Arial, sans-serif; padding: 20px; }
  .slider { width: 300px; margin: 10px 0; }
  .color-box { width: 100px; height: 100px; margin-top: 20px; border: 1px solid #000; }
  button { margin: 5px; }
</style>
</head>
<body>

<h2>HSV to RGB LED Controller</h2>

<label>Hue (0-360): <span id="hVal">0</span></label><br>
<input type="range" id="hue" min="0" max="360" value="0" class="slider"><br>

<label>Saturation (0-100%): <span id="sVal">100</span></label><br>
<input type="range" id="sat" min="0" max="100" value="100" class="slider"><br>

<label>Value (0-100%): <span id="vVal">100</span></label><br>
<input type="range" id="val" min="0" max="100" value="100" class="slider"><br>

<hr>

<label>Fill Range: <span id="fillRangeVal">0 - 10</span></label>
<div id="fillRangeSlider" class="slider"></div><br>

<label>Set Index: <span id="setIndexVal">0</span></label><br>
<input type="range" id="index" min="0" max="238" value="0" class="slider"><br><br>

<button id="fillBtn">Fill</button>
<button id="setBtn">Set</button>
<button id="clearBtn">Clear</button>

<div class="color-box" id="colorBox"></div>
<p>RGB: <span id="rgbText">255, 0, 0</span></p>

<script src="https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/15.7.0/nouislider.min.js"></script>
<script>
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
    start: [0, 10],
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
</script>
</body>
</html>

)rawliteral";
return htmlContent;
}

String editorContentF(){
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


//IR
#include "PinDefinitionsAndMore.h"
#include "TinyIRReceiver.hpp"
#define USE_EXTENDED_NEC_PROTOCOL

#if !defined(STR_HELPER)
    #define STR_HELPER(x) #x
    #define STR(x) STR_HELPER(x)
#endif
bool pixelPower = true;
uint8_t NPR = 0;
uint8_t NPG = 0;
uint8_t NPB = 0;

//

//NeoPixelSetup

void NeoPixelSetup(){
    #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
        clock_prescale_set(clock_div_1);
    #endif
    pixels.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
        pixels.show();            // Turn OFF all pixels ASAP
    pixels.setBrightness(brightness);
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
    pixels.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    pixels.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}
//

//WIFI

String removeWhitespace(String input) {
  String result = "";
  for (int i = 0; i < input.length(); i++) {
    char c = input[i];
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
      result += c;
    }
  }
  return result;
}

void handleRoot() {  
  server.send(200,"text/html", htmlContentF());
  Serial.println("requested: " + server.uri());
}
void handleEditor() {  
  server.send(200,"text/html", editorContentF());
  Serial.println("requested: " + server.uri());
}

void setHandle(){
    if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b") && server.hasArg("n")){
    pixels.setPixelColor(cursorIndex + server.arg("n").toInt(), pixels.Color(server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt()));
    pixels.show();
    server.send(200,"text/plain","OK.");
    }
    else{
    server.send(200,"text/plain","ERROR.");
    }
    
}

void fillHandle(){
    if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b") && server.hasArg("s") && server.hasArg("f")){
    
    pixels.fill(pixels.Color(server.arg("r").toInt(), server.arg("g").toInt(), server.arg("b").toInt()),cursorIndex + server.arg("s").toInt(), server.arg("f").toInt()-(server.arg("s").toInt() + cursorIndex)+1);
    pixels.show();
    server.send(200,"text/plain","OK.");
    }
    else{
    server.send(200,"text/plain","ERROR.");
    }
    
}
void clearHandle(){
    pixels.clear();
    pixels.show();  
    server.send(200,"text/plain","OK.");
}

void cursorHandle(){
    if(server.hasArg("n")){
    cursorIndex = (int8_t)server.arg("n").toInt();
    server.send(200,"text/plain","OK.");
    }
    else{
        server.send(200,"text/plain","OK.");
    }
}

void funcHandle(){
    if(server.hasArg("cmd") && server.hasArg("val")){
        String ret;
        String cmd = server.arg("cmd");
        String val = server.arg("val");
    if(cmd == "list"){
        ret = "{"+String(programNames[0]) + ',' + String(programNames[1]) + ',' + String(programNames[2]) + ',' + String(programNames[3])+"}";
        server.send(200,"text/plain",ret);
    }
    else if(cmd == "add"){
        int8_t index = -1;
        for (int8_t i = 0; i < 3; i++) {
            if (programNames[i] == "-") {
                index = i;
                break;
            }
        }
        if(index == -1){
            server.send(200,"text/plain","ERROR: NO SPACE TO ADD.");
        }
        else{
            programs[index] = server.arg("val").substring(server.arg("val").indexOf('-')+1);
            programNames[index] = server.arg("val").substring(0,server.arg("val").indexOf('-'));
            programs[index] = removeWhitespace(programs[index]);
            server.send(200,"text/plain","OK.");
        }
    }
    else if(cmd == "del"){
        programs[server.arg("val").toInt()] = "";
        programNames[server.arg("val").toInt()] = "-";
        server.send(200,"text/plain","OK.");
    }
    else if(cmd == "read"){
        ret = String(programNames[server.arg("val").toInt()]) + '-' + String(programs[server.arg("val").toInt()]);
        server.send(200,"text/plain",ret);
    }
    else if(cmd == "run"){
        currentProgram = server.arg("val").toInt();
        currentProgramLineG = 0;
        server.send(200,"text/plain","OK.");
    }
    }
    else{
        server.send(200,"text/plain","OK.");
    }
}

void endHandle() {
    currentProgram = -1;
    currentProgramLineG = 0;
    server.send(200, "text/plain", "OK.");
}

void resetHandle() {
    pixels.clear();
    pixels.show();
    server.send(200, "text/plain", "OK.");
    ESP.restart();
}

void brightHandle() {
    if(server.hasArg("n")){
        if(server.arg("n").toInt()<256 && server.arg("n").toInt()>0){
            brightness = server.arg("n").toInt();
            pixels.setBrightness(brightness);
            pixels.show();
            server.send(200, "text/plain", "OK.");
        }
        else{
            server.send(200, "text/plain", "ERROR: 'n' must between 0,255.");
        }
    }
    else{
        server.send(200, "text/plain", "ERROR: 'n' not found.");
    }
}

void handleNotFound() {
  Serial.print("Unknown request: ");  
  Serial.println(server.uri());  // Print unknown request path
  server.send(404, "text/plain", "Not found");
}
void wifiSetup(){
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
}

//

//NeoPixel
void IRToNeoPixel(uint8_t cmd){
    if(cmd == 2){pixelPower = false; pixels.clear(); pixels.show();}
    if(cmd == 3){pixelPower = true;}
    if(pixelPower){
    switch(cmd){
        case 0:
            if(brightness+50<255){
                brightness +=50;
            }
            else{
                brightness =255;
            }
            pixels.setBrightness(brightness);
            pixels.show();
            break;
        case 1:
            if(brightness+50>0){
                brightness -= 50;
            }
            else{
                brightness = 0;
            }
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
            NPR = 240;
            NPG = 50;
            NPB = 0;
            break;
        case 9:
            NPR = 50;
            NPG = 240;
            NPB = 0;
            break;
        case 10:
            NPR = 0;
            NPG = 50;
            NPB = 240;
            break;
        case 11:
            NPR = 255;
            NPG = 0;
            NPB = 0;
            break;
        case 12:
            NPR = 255;
            NPG = 0;
            NPB = 0;
            break;
        case 13:
            NPR = 255;
            NPG = 0;
            NPB = 0;
            break;
    }
    pixels.clear();
    pixels.fill(pixels.Color(NPR, NPG, NPB),0,LED_COUNT);
    Serial.println(NPR);
    Serial.println(NPG);
    Serial.println(NPB);
    pixels.show();
    }
    else{
        if(cmd == 11){
            String ip = WiFi.localIP().toString();
            Serial.println(ip);
            ip = ip.substring(ip.lastIndexOf('.')+1);
            Serial.println(ip);
            pixels.clear();
            int lastIndex=0;
            for(int i = 0; i<ip.length(); i++){
                pixels.setPixelColor(lastIndex, pixels.Color(0, 0, 255));
                pixels.show();
                lastIndex += 1;
                for (int i2 = 0; i2 < (ip[i] - '0'); i2++) {
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
}
//

//IR
void IRSetup(){
    pinMode(2,OUTPUT);
    digitalWrite(2, 0);
    Serial.println();
    if (!initPCIInterruptForTinyReceiver()) {
        Serial.println(F("No interrupt available for pin " STR(IR_RECEIVE_PIN))); // optimized out by the compiler, if not required :-)
    }
    Serial.println(F("Ready to receive NEC IR signals at pin " STR(IR_RECEIVE_PIN)));
    String ip = WiFi.localIP().toString();
            Serial.println(ip);
            ip = ip.substring(ip.lastIndexOf('.')+1);
            Serial.println(ip);
            pixels.clear();
            int lastIndex=0;
            for(int i = 0; i<ip.length(); i++){
                pixels.setPixelColor(lastIndex, pixels.Color(0, 0, 255));
                pixels.show();
                lastIndex += 1;
                for (int i2 = 0; i2 < (ip[i] - '0'); i2++) {
                    Serial.println(lastIndex);
                    pixels.setPixelColor(lastIndex, pixels.Color(0, 255, 0));
                    pixels.show();
                    lastIndex++;
                }
                pixels.setPixelColor(lastIndex, pixels.Color(0, 0, 255));
                pixels.show();
            }
}
void IRLoop(){
    if (TinyReceiverDecode()) {
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

//Compiler
void getArgs(char *value, int32_t *args){
  tempString = value;
  int8_t pointIndex = -1;
  int8_t cArg = 0;
  while(true){
    String tmpArg = tempString.substring(pointIndex+1, tempString.indexOf('.', pointIndex+1));
    if(tmpArg.indexOf('x') != -1){
        int xIndex = (int)tmpArg.substring(1).toInt();
        args[cArg] = xv[xIndex];
    }
    else{
        args[cArg] = (int32_t)tmpArg.toInt();
    }
    pointIndex = tempString.indexOf('.',pointIndex+1);
    cArg += 1;
    if(pointIndex == -1){break;}
  }
}

void compiler(char command[8], char value[20], int32_t S3){
    int32_t args[5] = {0,0,0,0,0}; 
    String cmd = String(command);
    getArgs(value,args);

    if(cmd == "SET"){
        Serial.println("SET " + String(cursorIndex + args[3]) + ":" + String(args[0]) + ',' + String(args[1]) + ',' + String(args[2]));
        pixels.setPixelColor(cursorIndex + args[3], pixels.Color(args[0], args[1], args[2]));
        pixels.show();
        currentProgramLineG = S3;
    }
    else if(cmd == "FILL"){
        Serial.println( "Fill from  " + String(cursorIndex + args[3]) + " to " + String(cursorIndex + args[4]) + ':' + String(args[0]) + ',' + String(args[1]) + ',' + String(args[2]));
        pixels.fill(pixels.Color(args[0], args[1], args[2]),cursorIndex + args[3],args[4]-(args[3]+cursorIndex)+1);
        pixels.show();
        currentProgramLineG = S3;
    }
    else if(cmd == "CLEAR"){
        pixels.clear();
        Serial.println("Cleared.");
        pixels.show();
        currentProgramLineG = S3;
    }
    else if(cmd == "CURSOR"){
        cursorIndex = args[0];
        Serial.println("New Cursor: " + String(cursorIndex));
        currentProgramLineG = S3;
    }
    else if(cmd == "SLEEP"){
        sleepValue = args[0];
        sleepIndex = millis();
        Serial.println("Sleeping for" + String(sleepValue));
        currentProgramLineG = S3;
    }
    else if(cmd == "JMP"){
        Serial.print("JumpedFrom " + String(currentProgramLineG) + " to ");
        int32_t lastIndex = S3;
        for(int16_t i = 0; i < abs(args[0]); i++){
        if(args[0] < 0){
            lastIndex = programs[currentProgram].lastIndexOf(':',lastIndex-1);
        }
        else if (args[0] > 0){
            lastIndex = programs[currentProgram].indexOf(':',lastIndex+1);
        }
        if(lastIndex == -1){lastIndex = 0;}
        }
        currentProgramLineG = lastIndex + 1;
        Serial.println(currentProgramLineG);
    }
    else if(cmd == "IFJMP"){
        args[0] = xv[args[0]];
        if(args[0] == args[1]){
        Serial.println("Jumped.");
        int32_t lastIndex = S3;
        for(int16_t i = 0; i < abs(args[2]); i++){
            if(args[0] < 0){
            lastIndex = programs[currentProgram].lastIndexOf(':',lastIndex);
            }
            else if (args[0] > 0){
            lastIndex = programs[currentProgram].indexOf(':',lastIndex+1);
            }
            if(lastIndex == -1){lastIndex = 0;}
        }
        currentProgramLineG = lastIndex + 1;
        }
        else{currentProgramLineG = S3;Serial.println("Continue.");}
    }
    else if(cmd == "DEFINE"){
        Serial.println("xv[" + String(args[0]) + ']' + " to " + String(args[1]) );
        xv[args[0]] = args[1];
        currentProgramLineG = S3;
    }
    else if(cmd == "ADD"){
        Serial.println("xv[" + String(args[0]) + ']' + " to " + String(xv[args[0]]+args[1]) );
        xv[args[0]] += args[1];
        currentProgramLineG = S3;
    }
    else if(cmd == "END"){
        Serial.println("Ended.");
        currentProgram = -1;
        currentProgramLineG = 0;
    }
    
}

void ScriptReader(String Script, int32_t currentProgramLine){
  int32_t S1 = Script.indexOf('(',currentProgramLine);
  int32_t S2 = Script.indexOf("):",S1);
  int32_t S3 = S2 + 2;
  char command[8];
  char value[20];
  Script.substring(currentProgramLine, S1).toCharArray(command, sizeof(command));;
  Script.substring(S1 + 1, S2).toCharArray(value, sizeof(value));
  compiler(command, value, S3);
}

bool SleepHandler(){
  return millis()-sleepIndex > sleepValue;
}

//

void setup() {
    Serial.begin(115200);
    NeoPixelSetup();
    wifiSetup();
    IRSetup();
}

void loop() {
    
    IRLoop();
    server.handleClient();
    if(currentProgram != -1){
        if(SleepHandler()){
            ScriptReader(programs[currentProgram], currentProgramLineG);
        }
    }
}





