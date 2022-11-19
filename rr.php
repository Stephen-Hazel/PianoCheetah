#!/usr/bin/php
<?php 
## rk.php (release kubuntu - cp to /opt/app/$app n create a .deb)

   $app = "pcheetah";   $now = date ('Ymd', time ());
   $arc = "armhf";   
   $siz = "2" . "000000";    // just gimme decimal megabytes
   $exe = [
      $app, 'midicfg',                                     // gui
      'delsame', 'll', 'mid2song', 'midimp', 'txt2song'    // background
   ];
// hopefully you send your exe build dirs to src/_build/exe 
   $src = "/home/sh/_/src";
   $dst = "/home/sh/_/web/PianoCheetah/download";

// CASE SENSITIVE filesystem dir to put our dang .deb
   $top = "/opt/app";        

   $deb = $app . "_$now" . "_$arc";    // our main .deb prefix / dir / etc
   $ctl = 
"Package: $app\n" .
"Version: $now\n" .
"Architecture: $arc\n" .
"Maintainer: " . 
   "Stephen Hazel<stephen.hazel@gmail.com> https://pianocheetah.app\n" .
"Description: " . 
   "PianoCheetah - Steve's weird midi sequencer for piano practice\n" .
"Priority: optional\n" .
"Essential: no\n" .
"Installed-Size: $siz\n" .
"Source:\n";

// cp to my installed dir
   foreach ($exe as $e)  system ("sudo cp $src/_build/$e/$e  $top/$app/$e");

// mkdir n cp exes in prep for makin a .deb
   system ("rm -r                    $top/$deb");
   system ("cp -r $src/$app/zRel/kub $top/$deb");
   $lst = "";
   foreach ($exe as $e) {
      $ex = "opt/app/$app/$e";
      system ("cp $src/_build/$e/$e  $top/$deb/$ex");
      $lst .= " $ex";
   }

// build debian/control to get our dang dependencies
   chdir  ("$top/$deb");
   system ("mkdir DEBIAN");
   system ("mkdir debian");
   file_put_contents ("debian/control", $ctl);
   $out = `dpkg-shlibdeps -v -O $lst`;
   $d = "";   $f = "shlibs:Depends=";
   foreach (explode ("\n", $out) as $o) 
      if (substr ($o, 0, strlen ($f)) == $f)  $d = substr ($o, strlen ($f));
echo "Depends: $d\n";
   system ("rm -fr debian");
   file_put_contents ("DEBIAN/control", $ctl . "Depends: $d\n");

   chdir ($top);
   system ("dpkg-deb --build --root-owner-group $deb");
   system ("mv $deb" . ".deb  $dst");
   system ("rm -fr $deb");
