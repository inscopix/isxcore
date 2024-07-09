# isx-core
Core functionality of the isx C++ API

## Supported Platforms

This library has been built and tested on the following operating systems:

|  OS | Version |
|  --------- | ------- |
| macOS   | 13 |
| Ubuntu (Linux) | 20.04 |
| Windows | 11 |

There is no guarantee this library will build on other systems.

## Setup

In order to build `isxcore`, certain third-party dependencies are required. Please contact support@inscopix.bruker.com for a link to download these dependencies for a specific OS.

### macOS 

#### Step 1: Configure terminal for Rosetta (Apple Silicon)

> **Note** If you are on an Intel machine, this step can be skipped.

For macOS machines, `isxcore` is currently only compiled for x86_64 architecture, which is problematic on Apple silicon since it has arm64 architecture. Fortunately, it’s still possible to compile and execute code for x86_64 architecture with Apple Silicon thanks to [Rosetta](https://en.wikipedia.org/wiki/Rosetta_(software)) - a software developed by Apple to automatically translate binaries between different architectures.

To compile `isxcore` on macOS with Apple Silicon, the Terminal app needs to be configured to always open with Rosetta. This ensures that we install the x86 version of dependencies used to compile `isxcore`. To do this:

* Open finder -> click on Applications.
* Double click on Utilities.
* Right click on Terminal -> and select Get info from the options.
* Select the Open using Rosetta check box.

> **Alternatively:** Run the rest of the commands in this guide in an x86 shell with the following command:

```
arch -x86_64 zsh
```

This command will launch an x86 shell, so any dependencies are installed in the x86 version.

#### Step 2: Install brew, cmake

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install cmake
```

#### Step 3: Install Xcode

Currently Xcode 12.4 is used to compile this library. Download this version of Xcode from the following website: [Apple Developer More Downloads](https://inscopix.atlassian.net/wiki/spaces/MOS/pages/2975268872/Initial+IDPS+Development+Environment+Setup+on+MacOS#:~:text=Apple%20Developer%20More%20Downloads). 

Unzip the application and drag it to the Applications folder. After that, run the following commands.

```
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
sudo xcodebuild -license accept
```

### Ubuntu 20.04

#### Step 1: Install development tools

```
sudo apt-get update
sudo apt-get install -y build-essential autoconf automake gdb git cmake
```

### Step 2: Install gcc-7

Currently gcc-7 is required to build `isxcore`, which can be installed from apt:

```
sudo apt-get install gcc-7
```

Then configure gcc-7 as the default on your system:

```
sudo update-alternatives --remove-all gcc 
sudo update-alternatives --remove-all g++
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 30
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 30
```

Verify the default gcc is configured correctly by ensuring the version is 7:

```
gcc --version
g++ --version
```

### Windows 11

#### Step 1: Install Microsoft Visual Studio 2015

Currently `isxcore` is compiled using Visual Studio Install 2015 on Windows. Install this version of Visual Studio using one of the links below:

* [Web Installer](https://go.microsoft.com/fwlink/?LinkId=532606&clcid=0x409)
* [ISO Image](https://go.microsoft.com/fwlink/?LinkId=615448&clcid=0x409)

During installation, select custom installation and check the box next to C++ to install C++-related packages.

#### Step 2: Install Git Bash

Download and install Git Bash from here: https://git-scm.com/. The remaining commands should be run in a git bash terminal.

## Build

To build the library run the following command in the root of this repo:

```
make build
```

Ensure third party dependencies are downloaded prior to running this command.
By default, the build system assumes the third party dependencies are in a folder named `third_party` under the root of this repo.

If the folder is located elsewhere on the machine, the path to this folder can be pass as a variable when running the build command:

```
make build THIRD_PARTY_DIR=/path/to/third/party/dir
```

## Test

To test the library run the following command in the root of this repo:

```
make test
```

Ensure test data is downloaded prior to running this command.
By default, the build system assumes the test data is in a folder named `test_data` under the root of this repo.

If the folder is located elsewhere on the machine, the path to this folder can be pass as a variable when running the build command:

```
make test THIRD_PARTY_DIR=/path/to/third/party/dir TEST_DATA_DIR=/path/to/test/data/dir
```
