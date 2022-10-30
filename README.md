# Simple menu program or game launcher for DOS
Inspired from Rloader (that's why you will find a rloader generator as well, not used on this readme).

Example usage:
You should have some games or programs in directories with long (human readable) name.

* execute LIST.TXT generator.
This will use your long directory names to generate a menu with them
and copy to output directory with DOS limitations.

`python3 menugen.py -i /Users/jsmolina/DIST/GAMES1  -i /Users/jsmolina/DIST/GAMES2 -o /Users/jsmolina/OUT -d c:\\menu`

It might ask you if more than one executable exists:
```
Creating /Users/jsmolina/OUT...
Generating LIST.txt from ['/Users/jsmolina/DIST/GAMES1', '/Users/jsmolina/DIST/GAMES2', '/Users/jsmolina/DIST/GAMES3']...
Generating PCXT list in /Users/jsmolina/OUT/LIST.TXT...
Creating games directory in /Users/jsmolina/OUT/GAMES...
Generating PCXT LIST.TXT /Users/jsmolina/OUT/LIST.TXT...
*** Multiple execs in /Users/jsmolina/DIST/GAMES1/Cool Croc ***
0 - COOL.EXE
1 - GO.BAT
2 - COOL.COM
3 - START.COM
Which one? 1
Executable will be exec.bat
Copy Cool Croc to /Users/jsmolina/OUT/GAMES/COOLCRO...
Executable will be exec.bat
Copy demotest to /Users/jsmolina/OUT/GAMES/DEMOTEST...
*** Multiple execs in /Users/jsmolina/DIST/GAMES1/Agent Usa ***
0 - AGENTPC.EXE
1 - AGENTUSA.COM
Which one? 1
Executable will be exec.bat
```

Then 
* copy the OUT directory into the directory you specified in -d (e.g. c:\menu) in your DOS machine. 
* COPY menu.exe where your LIST.TXT is
* execute menu.exe


Simple but works!

<img width="633" alt="image" src="https://user-images.githubusercontent.com/447481/198902575-1d5ac892-c79a-4447-81f7-1aa81dfe865f.png">



https://user-images.githubusercontent.com/447481/198902539-7361644f-7a5c-4429-8ee7-9aab299d1359.mov

Collaborators: @jsmolina @Mills

