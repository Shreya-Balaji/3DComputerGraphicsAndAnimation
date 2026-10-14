The VS Code C/C+ and CMake Tools extensions are recommended
#Git, CMake, Visual Studio needs to be downloaded.

#clone repository
gh repo clone Shreya-Balaji/3DComputerGraphicsAndAnimation
cd YOUR-REPOSITORY

## vcpkg download
```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg 
.\bootstrap-vcpkg.bat
.\vcpkg.exe version
```
## Configure the build
Run these commands from the repository root. The following assumes vcpkg is installed at `C:\vcpkg`:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```
CMake should configure the project, find the dependencies, and compile the executable. The build output is normally:

```text
build\Release\CableCarAlps.exe
```
## Run

From the repository root:

```powershell
.\build\Release\CableCarAlps.exe
```