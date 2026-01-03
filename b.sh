#!/bin/php
<?php
// b - build with flatpak-builder n stuff
// args:  c  wipe config dir - for full reinstall
//        d  build with debugging so ya can gdb inside it
   $arg = '';   if ($argc > 1)  $arg = $argv [1];

// app triplet n fpak cmds
   $app = "app.pianocheetah.pianocheetah";
   $f = "flatpak";   $fb = "$f-builder";

   system ("rm -fr _build .$fb");      // cleanup

echo "...uninstall old one\n";
   system ("$f uninstall -y $app");

   if ($arg == 'd') {
/*    DEBUGGIN !  then go...
      flatpak run --command=sh --devel --filesystem=$(pwd) app_triplet
      gdb /app/bin/pianocheetah
      set logging enabled on
      thread apply all backtrace
      run
      where
      bt full
*/    system ("mkdir _build");
      system ("$fb --user --force-clean --install _build fpak.dbg", $rc);
      system ("$f install --reinstall --user --assumeyes ".
              "/home/sh/src/pianocheetah/.$fb/cache $app");
      system ("$f install --reinstall --user --assumeyes ".
              "/home/sh/src/pianocheetah/.$fb/cache $app".".Debug");
      exit;
   }

// source => _build => install
   system ("mkdir _build");

echo "...compilin n installin\n";
   system ("$fb --user --install _build fpak", $rc);
   if ($rc != 0)  exit;                // build error :(

echo "...cleanup\n";
   system ("rm -fr _build .$fb");
   if ($arg != 'c')  exit;

// cleanup .var/app/ dir for full reset (wipe config files etc)
echo "...reset config files\n";
   system ("rm -fr ~/.var/app/$app");
