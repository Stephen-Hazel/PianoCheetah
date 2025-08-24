#!/bin/php
<?php
// bDbg - build with flatpak-builder n stuff DEBUG VERSION
// build then...
/*
flatpak run --command=sh --devel --filesystem=$(pwd) app.pianocheetah.pianocheetah
gdb /app/bin/pianocheetah
set logging enabled on
thread apply all backtrace
run
bt full
*/
// args:   o   build only - for release to flathub
//         c   wipe config dir - for full reinstall
   $arg = '';   if ($argc > 1)  $arg = $argv [1];

// repo name n app triplet
   $repo = "pc_repo";   $app = "app.pianocheetah.pianocheetah";
   $f = "flatpak";      $fb = "flatpak-builder";

// source => _build
   system ("rm -fr _build .$fb");
   system ("mkdir  _build");
   system ("$fb    --user --force-clean --install _build _manif.dbg", $rc);
   system (
"$f install --reinstall --user --assumeyes /home/sh/src/pianocheetah/.flatpak-builder/cache $app")   ;
   system (
"$f install --reinstall --user --assumeyes /home/sh/src/pianocheetah/.flatpak-builder/cache $app".".Debug")   ;
exit;

// _manif is build manifest
#  system ("rm -fr .$fb");
   if (($rc != 0) || ($arg == 'o'))  exit;
                                       // build error :( or build only
// uninstall old app
   system ("$f uninstall -y $app");

// _build => _repo  (flathub pr does this, too)
// add _build n release manifest to flathub pr's git repo
   system ("rm -fr _xb _repo");        // unused build dir _xb for fb
   system ("mkdir  _xb _repo");
// make repo from THIS _build dir w _rel json
   system ("$fb --repo=_repo _xb _rel");    
#  system ("rm -fr    _build _xb .$fb");
   system ("rm -fr    _build _xb");

// add repo, install, n remove repo  (just tryna be neat)
   system ("$f remote-add --no-gpg-verify $repo _repo");
   system ("$f install --include-sdk --include-debug --noninteractive ".
           "$repo $app");
   system ("$f install --include-sdk --include-debug --noninteractive ".
           "$repo $app" . ".Debug");
   system ("$f --force remote-delete      $repo");
   system ("rm -fr _repo");

// cleanup app's .var/app/ dir for full reset
   system ("/home/sh/.bin/x");         # wipe dbg.txt

   if ($arg != 'c')  exit;
   system ("rm -fr ~/.var/app/$app");
