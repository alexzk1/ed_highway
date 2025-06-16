# ed_highway
Elite Dangerous route plotter GUI for www.spansh.co.uk/plotter (neutron stars).

For installation instructions see arch_linux/PKGBUILD file which is basically collection of bash scripts.
On arch you can just download that single file and do "makepkg -is"

Project uses submodules, so once clonned do
`git submodule init

What's new:

    ~~added OCR. Should be "reading" star system name from current mouseover tooltip on galaxy map~~
    removed OCR as game has "copy name" button for long.
    carrier's fuel mining is tested during 2 years trip for 62000ly, it predicts correct refuel plan.
    
