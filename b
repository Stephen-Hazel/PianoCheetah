#!/bin/env php
<?php
// b - build with flatpak-builder n stuff
// args:   o   build only - for release to flathub
//         c   wipe config dir - for full reinstall
   $arg = '';   if ($argc > 1)  $arg = $argv [1];

// repo name n app triplet
   $repo = "pc_repo";   $app = "app.pianocheetah.pianocheetah";
   $f = "flatpak";      $fb = "flatpak-builder";

// source => _build
   system ("rm -fr _build .$fb");
   system ("mkdir  _build");
   system ("$fb    _build _manif", $rc);         // _manif is build manifest
   system ("rm -fr .$fb");
   if (($rc != 0) || ($arg == 'o'))  exit;
                                       // build error :( or build only
// uninstall old app
   system ("$f uninstall -y $app");

// _build => _repo  (flathub pr does this, too)
// add _build n release manifest to flathub pr's git repo
   system ("rm -fr _xb _repo");        // unused build dir _xb for fb
   system ("mkdir  _xb _repo");
   system ("$fb --repo=_repo _xb $app.json");    // flathub release manifest
   system ("rm -fr    _build _xb .$fb");

// add repo, install, remove n rm repo  (just tryna be neat)
   system ("$f remote-add --no-gpg-verify $repo _repo");
   system ("$f install --noninteractive   $repo $app");
   system ("$f --force remote-delete      $repo");
   system ("rm -fr _repo");

// cleanup app's .var/app/ dir for full reset
   if ($arg != 'c')  exit;
   system ("rm -fr ~/.var/app/$app");
   system ("rm -fr ~/pianocheetah");   // specific to mee
