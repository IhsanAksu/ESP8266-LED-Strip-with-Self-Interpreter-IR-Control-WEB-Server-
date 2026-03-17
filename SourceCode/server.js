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
