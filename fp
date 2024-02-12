#!/usr/bin/env php
<?php # fp - flatpak publish

system ("rm -fr .flatpak-builder");
system ("rm -fr _build");   system ("rm -fr _repo");
system ("mkdir  _build");
system ("flatpak-builder gpg-sign=--repo=../sh_repo _build _mani");
