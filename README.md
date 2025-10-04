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

The principle of the solving algorithm is explained in <a href="https://www.mad-weasel.de/download/The perfect playing Nine Men Morris computer.pdf" target="_blank">this</a> pdf document.

The current executable and database can be downloaded [here](https://www.mad-weasel.de/morris.html). Download the corresponding zip files and extract both zip files into the same directory.

# Software dependencies
## Libraries
This application uses the following EXTERNAL libraries: 
- [DirectX 11 Toolkit](https://github.com/Microsoft/DirectXTK)
- [Google Test](https://github.com/google/googletest)
- shlwapi.lib, comctl32.lib, DXGI.lib, D3D11.lib, XmlLite.lib
- Win32 API
- C++ STL
## Tools
The following EXTERNAL tools were used for the source code and executable generation: 
- Visual Studio Community 2022
- Visual Studio Code
- [ResEdit](http://www.resedit.net/)
## Environment
System requirements:
- Windows 11
- DirectX 11 (since the DirectXTK 11 is used)

# Latest releases
January 1, 2019 - First release.

January 1, 2026 - Second release.

# Build
- Install MS Visual Studio Community 2022
  - [VCPKG Package Manager](https://github.com/microsoft/vcpkg) (a C++ library manager used here for DirectXTK 11)
  - Visual Studio (Community) C++ 2022
    - Visual Studio C++ Core Features
    - Visual Studio C++ Desktop Features
    - Windows 11 SDK (>= 10.0.22000.0)
- Install Visual Studio Code
- (Optional) Install GitHub Extension for Visual Studio (for Visual Studio, not Visual Studio Code; skip if you already use another GitHub tool)
- Clone the [DirectXTK 11 git repository](https://github.com/Microsoft/DirectXTK)
- Clone this repository
- Clone the library repository [weaselLibrary](https://github.com/madweasel/weaselLibrary)
- Open the "x64 Native Tools Command Prompt for VS 2022" and type in `code .` to open Visual Studio Code in the current directory
- Open the C++ project in Visual Studio Code
- Wait for the VCPKG tool to finish the installation of the DirectXTK 11 library (this is typically handled automatically by CMake integration; manual intervention is only needed if errors occur)
- Click on Build

# Contribute
If you want to contribute to this project, please contact me via [karaizy@mad-weasel.de](mailto:karaizy@mad-weasel.de).

# License
Copyright (c) Thomas Weber. All rights reserved.

Licensed under the [MIT](LICENSE) License.
