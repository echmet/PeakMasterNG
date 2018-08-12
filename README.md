PeakMaster NG
===

Introduction
---
PeakMaster NG is a graphical user interface for [LEMNG](https://github.com/echmet/LEMNG) model for simulations of capillary zone electrophoresis experiments. PeakMaster NG provides convenient means to enter input data for the LEMNG model and display the results of LEMNG's calculations. PeakMaster NG binaries and a user guide can be downloaded from the [ECHMET research group website](https://echmet.natur.cuni.cz/peakmaster).

Building
---
The following tools and libraries are required to build PeakMaster NG

- C++14-aware compiler
- [Qt 5 toolkit](https://www.qt.io/) (Qt 5.6 or higher is required)
- [Qwt toolkit](http://qwt.sourceforge.net/)
- [SQLite 3](https://www.sqlite.org/index.html)
- [ECHMETCoreLibs](https://github.com/echmet/ECHMETCoreLibs)
- [LEMNG](https://github.com/echmet/LEMNG)
- [ECHMETUpdateCheck](https://github.com/echmet/ECHMETUpdateCheck)

Refer to official documentation of the respective projects for details how to set them up.

### Linux/UNIX
The README assumes that a system-wide installation of the Qt toolkit is available. If you intend to use a custom Qt installation, adjust the building steps accordingly.

1. Set up paths to dependencies accordingly to your setup in `PeakMasterNG.pri` file. See the content of the file for details.
2. `cd` to the directory with PeakMaster NG sources.
3. Create a new directory named `build` and `cd` into the directory
4. Run `qmake ../PeakMasterNG.pro CONFIG+=release`
5. Run `make`

Prior to launching the binary, make sure that all required binary libraries are available to the OS'es dynamic linker.

Compatibility with \*NIX systems other than Linux has not been tested.

### Windows
Windows building procedure relies on the [Qt Creator](https://www.qt.io/qt-features-libraries-apis-tools-and-ide/#ide) IDE to set up and build the project.

1. Launch Qt Creator and load the `PeakMasterNG.pro` project file
2. Set up paths to dependencies accordingly to your setup in `PeakMasterNG.pri` file. See the content of the file for details.
3. Configure build path and select `Release` build in the Qt Creator interface. Consult the Qt Creator documentation for details.
4. Click `Build`.

Prior to launching the binary, make sure that all required binary libraries are available to the OS'es dynamic linker.

Licensing
---
The PeakMaster NG project is distributed under the terms of **The GNU General Public License v3** (GNU GPLv3). See the enclosed `LICENSE` file for details.

As permitted by section 7. *Additional Terms* of The GNU GPLv3 license, the authors require that any derivative work based on PeakMaster NG clearly refers to the origin of the software and its authors. Such reference must include the address of this source code repository (https://github.com/echmet/PeakMasterNG) and names of all authors and their affiliation stated in section [Authors](#Authors) of this README file.

<a name="Authors"></a>
Authors
---
Milan Štědrý (1)  
Vlastimil Hruška (1)  
Michal Jaroš (1)  
Iva Zusková (1)  
Bob Gaš (1)  
Pavel Dubský (1)  
Gabriel Gerlero (2,3)  

(1)  
Group of Electromigration and Chromatographic Methods (http://echmet.natur.cuni.cz)

Department of Physical and Macromolecular Chemistry  
Faculty of Science, Charles University, Czech Republic

(2)  
Departamento de Ingeniería en Sistemas de Información, FRSF-UTN.  
Santa Fe, Argentina

(3)  
Centro de Investigación de Métodos Computacionales (CIMEC, UNL-CONICET).  
Santa Fe, Argentina.
