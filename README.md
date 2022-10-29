# rloader-generator
Generates rLoader

Example usage:


* execute LIST.TXT generator.
This will use your long directory names to generate a menu with them
and copy to output directory with DOS limitations.

`python3 menugen.py -i /Users/jsmolina/DIST/GAMES1  -i /Users/jsmolina/DIST/GAMES2 -o /Users/jsmolina/OUT -d c:\\menu`

Then 
* copy the OUT directory into the directory you specified in -d (e.g. c:\menu) in your DOS machine. 
* COPY menu.exe where your LIST.TXT is

