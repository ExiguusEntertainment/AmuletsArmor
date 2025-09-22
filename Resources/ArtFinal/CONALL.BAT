copy ..\*.pic
rem copy ..\*.cpc
rem copy ..\*.mpr
conpal *.pic
cd mpress
copy ..\sword.pic
copy ..\xbow.pic
copy ..\punch.pic
mpress
del *.pic
cd ..
copy mpress\*.mpr
cpc *.pic
copy \prg\dos32\game\intergr\*.pic
copy \prg\dos32\game\res\*.pic
..\..\resmake allpic.fl pics.res
copy palette.new game.pal
genpal
concat mdat.fl mdat.res
..\..\resmake sample.fl sample.res
game
