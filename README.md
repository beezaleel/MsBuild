# MsBuild

### How to build

1. Open a command prompt terminal
1. CD to `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin` (I'm using visual studio 2022)
2. Run `MSBuild.exe \path\to\project\MSBuildProject.vcxproj -t:project_03 -property:Platform=x64 -property:Configuration=Release

This will not execute the deployment as instructed. The project only ships in `Debug` mode.

3. Run `MSBuild.exe \path\to\project\MSBuildProject.vcxproj -t:project_03 -property:Platform=x64 -property:Configuration=Debug

This will ship the project to `c:\cnd\Ademola_Adedeji`

4. A zip file will also be created in `c:\cnd` with name `Ademola_Adedeji.zip`