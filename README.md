# Introduction
## Nine Men's Morris Game - The perfect playing computer
This application uses a precalculated database containing all possible moves to show the user the perfect move in the standard Nine Men's Morris game.

<div align="center">
  <a href="https://www.mad-weasel.de/morris.html"
     target="_blank">
    <img src="https://www.mad-weasel.de/images/muehle_spielfeld_gewonnen.png"
         alt="Ingame screenshot"
         width="240" height="180" border="10" />
  </a>
  <a href="https://www.mad-weasel.de/morris.html"
     target="_blank">
    <img src="https://www.mad-weasel.de/images/muehle_spielfeld_verloren.png"
         alt="Ingame screenshot"
         width="240" height="180" border="10" />
  </a>  
</div>

The principle of the solving algorithmn is explained in <a href="https://www.mad-weasel.de/download/The perfect playing Nine Men Morris computer.pdf" target="_blank">this</a> pdf document.

The current executable and database can be downloaded [here](https://www.mad-weasel.de/morris.html). Download the corresponding zip files and extract both zip files into the same directory.

# Software dependencies
## Libraries
This application uses the following EXTERNAL libraries: 
- [DirectX 11 Toolkit](https://github.com/Microsoft/DirectXTK)
- shlwapi.lib, comctl32.lib, DXGI.lib, D3D11.lib, XmlLite.lib
- Win32 API
- C++ STL
## Tools
The following EXTERNAL tools were used for the source code and executable generation: 
- MS Visual Studio Community 2017
- [ResEdit](http://www.resedit.net/)
- GitHub Extension for Visual Studio
## Environment
System requirements:
- Windows 10
- DirectX 11 (since the DirectXTK 11 is used)

# Latest releases
January 1, 2019 - First release.

# Build
- Install MS Visual Studio Community 2017
  - NuGet-Paket-Manager (for DirectXTK 11)
  - VC++ 2017
  - Visual Studio C++-Corefeatures
  - Windows 10 SDK (> 10.0.17763.0)
- Install GitHub Extension for Visual Studio (if no other github tool is already installed)
- Clone the [DirectXTK 11 git repository](https://github.com/Microsoft/DirectXTK)
- Clone this repositority
- Clone the library repository [weaselLibrary](https://github.com/madweasel/weaselLibrary)
- Open the project via the .sln file (MuehleWin.sln)
- Compile and run

# Contribute
TODO: Explain how other users and developers can contribute to make your code better. 

Contact: [karaizy@mad-weasel.de](mailto:karaizy@mad-weasel.de).

# License
Copyright (c) Thomas Weber. All rights reserved.

Licensed under the [MIT](LICENSE) License.
