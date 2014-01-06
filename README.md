moe [ moɛ ] 萌
===

A GTK3 rewrite of KanjiPad, a Japanese Handwriting recognition software

<b>Why rewrite kanjipad</b>

Kanjipad is a great software, but it's also quit old and cannot be compiled
with the later version of GTK2 and of course with GTK3 .

So I rewrite it using GTK3, Glade and Cairo, keeping most of the engine as-is. 

<b>Similarites with Kanjipad</b>

* Almost the same user interface layout
* Uses the same engine (JStroke)

<b>Differances with KanjiPad</b>

* Use GTK3: Cairo, Glade and non deprecated widgets
* Code simplified and commented
* JStroke is now integrated at compile time instead using a separate software
  (KanjiPad <-stdin-> KPengine <-linked-> Jstroke). At the cost UI / Engine full
  separation, but at the benefits of a much more clear code.  
* More prettier, thanks to Cairo and GTK3 theme.


<b>TODO</b>

* Remove some legacy Code from Kanjipad (datatype)
* Undo functionnality
* Annotate
* Finish menu


I need some help for:

* Better drawing callback. Right now I(s a bit laggy bevause all the strokes are redrawn at each invalidate
  It must have a better way (invalidate a rect like in kanjipad)
* Add effects (brush, ...) using Cairo. 

  
<i>Credits for the Kanjipad and JStroke softwares goes to the original author 
and contributors. </i>
