# ed_highway
Elite Dangerous route plotter GUI for www.spansh.co.uk/plotter (neutron stars).

For installation instructions see arch_linux/PKGBUILD file which is basically collection of bash scripts.
On arch you can just download that single file and do "makepkg -is"

Project uses submodules, so once clonned do
`git submodule init

What's new:

    added OCR. Should be "reading" star system name from current mouseover tooltip on galaxy map
    
To use OCR you need download tessdata from https://tesseract-ocr.github.io/tessdoc/Data-Files

and put in 1 of folders subfolder "tessdata":

QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0) + "/" + qAppName(),

QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0) + "/tesseract",
        
QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0) + "/.local/share/",
        
QCoreApplication::applicationDirPath(),

