#!/usr/bin/php
## rk.php (release kubuntu - cp to /opt/app n create a .deb)
   $exe = [
      'pcheetah', 'midicfg',                               // gui
      'delsame', 'll', 'mid2song', 'midiimp', 'txt2song'   // background
   ];
// put in my installed dir
   foreach ($exe as $e)  system ("sudo cp ../_build/$e/$e  /opt/app/$e");

// build a dang .deb for kubuntu
   $dt = date ('Ymd', time ());
   $dd = "pcheetah_$dt" . "_amd64";
   system ("cp -r zRelease/kub /opt/app/$dd");
   foreach ($exe as $e)  system ("cp      ../_build/$e/$e  " . 
                                 "/opt/app/$dd/opt/app/pcheetah/$e");

#
cp ../_build/delsame/delsame   zRelease/delsame
cp ../_build/ll/ll             zRelease/ll
cp ../_build/mid2song/mid2song zRelease/mid2song
cp ../_build/midicfg/midicfg   zRelease/midicfg
cp ../_build/midiimp/midiimp   zRelease/midiimp
cp ../_build/pcheetah/pcheetah zRelease/pcheetah
cp ../_build/txt2song/txt2song zRelease/txt2song
