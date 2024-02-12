#!/usr/bin/env php
<?php
## s - make and kill stv dir for qtcreator / flatpak-build
## flatpak-builder --repo=_repo --subject="pcheetah `date`" --ccache --force-clean _app _mani

   $x = isset ($argv [1]) ? 'y' : 'n';           // any arg=y
   
   $exe = [
      'pianocheetah', 'midicfg', 'initme',           // gui
      'll', 'mid2song', 'midimp', 'txt2song',    // background
      'synsnd', 'sfz2syn', 'mod2song'
   ];
   foreach ($exe as $e) {
      if ($x == 'y')  system ("ln -s ../../stv $e/stv");
      else            system ("rm              $e/stv");
   }
