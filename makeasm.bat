rem tasm pcxtmenu.asm /dNO_SNOW: to avoid CGA snow on real CGA cards
rem tasm pcxtmenu.asm /dMDA: to compile for MDA-HERCULES
tasm pcxtmenu.asm 
tlink /t pcxtmenu.obj
del pcxtmenu.obj
del pcxtmenu.map