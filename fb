#!/usr/bin/env php
<?php # fb - flatpak builder stuph

system ("rm -fr _build .flatpak-builder");
system ("mkdir  _build");
system ("flatpak-builder _build _mani");
system ("rm -fr .flatpak-builder _build/files/lib/debug");
