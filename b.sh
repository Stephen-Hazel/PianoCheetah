#!/bin/php
<?php
// b - build with flatpak-builder n stuff
// args:   o   build only - for release to flathub
//         c   wipe config dir - for full reinstall
   $arg = '';   if ($argc > 1)  $arg = $argv [1];

// repo name n app triplet
   $app = "app.pianocheetah.pianocheetah";
   $f = "flatpak";   $fb = "$f-builder";

// source => _build
   system ("rm -fr _build _repo .$fb");
   system ("mkdir  _build _repo");
   system ("$fb --repo=_repo _build _manif", $rc);
// build error :( or build only
   if (($rc != 0) || ($arg == 'o'))  exit;

// uninstall old app
   system ("$f uninstall -y $app");

// add repo, install, n remove repo  (just tryna be neat)
   system ("$f remote-add --no-gpg-verify _repo");
   system ("$f install --noninteractive   _repo $app");
   system ("$f --force remote-delete      _repo");

// cleanup app's .var/app/ dir for full reset
   system ("/home/sh/.bin/x");         # wipe dbg.txt

   if ($arg != 'c')  exit;
   system ("rm -fr ~/.var/app/$app");
