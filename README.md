# ESP8266-LED-Strip-with-Self-Interpreter-IR-Control-WEB-Server-



GET Commands:
~~~
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
~~~


---

Built-In Interpreter Commands:
~~~
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
~~~
