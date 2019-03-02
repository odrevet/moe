# moe [ moɛ ] 萌

A GTK3 rewrite of KanjiPad, a Japanese Handwriting recognition software

# Why this rewrite of kanjipad 

Kanjipad is a great software, but it's also quit old and cannot be compiled
with the later version of GTK2 and of course with GTK3 .

So I rewrite it using GTK3, Glade and Cairo, keeping most of the engine as-is. 

# Similarites with Kanjipad

* Almost the same user interface layout
* Uses the same engine (JStroke)

# Differances with KanjiPad

* Use GTK3: Cairo, Glade and non deprecated widgets
* Code simplified and commented
* JStroke is now integrated at compile time instead using a separate software
  (KanjiPad <-stdin-> KPengine <-linked-> Jstroke). At the cost UI / Engine full
  separation, but at the benefits of a much more clear code.  
* Prettier, thanks to Cairo and GTK3 theme.
  
<i>Credits for the Kanjipad and JStroke softwares goes to the original author 
and contributors.</i>
