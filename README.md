# mgs2mods
ASI plugin with gameplay modifications for Metal Gear Solid 2 Substance on PC.

See https://mgs.w00ty.com/mgs2/asi/ for prebuild downloads and more information.

## Build process
* Use Visual Studio (developed in VS Community 2022) to build.
* Configuration `MGS2` is the public version.
* Configuration `Debug` requires that the contents of the `lib\sqlite` folder be included in the project.

## Installation
* Ensure [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases) is installed in MGS2.
  * If you have [V's Fix](https://github.com/VFansss/mgs2-v-s-fix/releases) installed, UASIL is already installed.
* Copy `MGS2.asi` to MGS2's `bin\scripts` directory.
* Also copy any desired INI config files from `MGS2.ConfigFiles` or `MGS2.ExtraConfigFiles` to the scripts directory.
  
