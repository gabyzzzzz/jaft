build: build.exe

build.exe: ..\src\*.cpp
	C:\msys64\usr\bin\bash.exe -c "g++ -fdiagnostics-color=always -g -I ../includes ../src/*.cpp -o build.exe"

clean:
	del /Q .\build\build.exe
