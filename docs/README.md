# pianocheetah 

Steve's pianohero++ app

![logo](https://pianocheetah.app/img/logo.png "logo")

https://pianocheetah.app for user docs

environment:  Qt c++ in a flatpak

build this flatpak with
 - sudo apt install flatpak flatpak-builder qtcreator
 - flatpak install org.kde.Sdk//6.10 org.kde.Platform//6.10
 - b.sh   (or b.sh d - for debugging)

Uses my stv library of c++ assumed to be in ../stv
CMakeList.txt s should have 
 -  ../../stv/os.cpp
 -  ../../stv/ui.cpp
 -  ../../stv/syn.cpp
 -  etc
