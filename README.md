## Summary

This repository is a PoC to reproduce a JVM crash I found in the Windows crash reporting on Java 18+. A PR was made in Java 22 to fix it, but I'm still able to reproduce it.

If the program runs successfully, it will show `Code successfully ran with Java xx.xx.xx` near the end.
If not, will crash and not generate any JVM crash log because of a stack overflow in the reporting process.

WinDbg info about the crash:
```
ModLoad: 00007ff8`5e630000 00007ff8`5edce000   C:\Windows\SYSTEM32\windows.storage.dll
ModLoad: 00007ff8`5ff10000 00007ff8`5ff3e000   C:\Windows\SYSTEM32\Wldp.dll
ModLoad: 00007ff8`610c0000 00007ff8`6118d000   C:\Windows\System32\OLEAUT32.dll
ModLoad: 00007ff8`61510000 00007ff8`615bd000   C:\Windows\System32\SHCORE.dll
ModLoad: 00007ff8`625e0000 00007ff8`62635000   C:\Windows\System32\shlwapi.dll
ModLoad: 00007ff8`604f0000 00007ff8`60515000   C:\Windows\SYSTEM32\profapi.dll
ModLoad: 00007fff`f4350000 00007fff`f4427000   C:\Program Files\Java\jdk-23\bin\jsvml.dll
(f61c.10030): Stack overflow - code c00000fd (first chance)
First chance exceptions are reported before any exception handling.
This exception may be expected and handled.
jvm!verify+0x89e57:
00007fff`845598b7 e8b4b30d00      call    jvm!AsyncGetCallTrace+0xcf5e0 (00007fff`84634c70)
```

This program has a 20 seconds sleep at the beginning, so you have time to attach a debugger.

Interesting fact: It seems that the last class being loaded before the crash is `ClassCircularityError`. From what I saw online it could be somewhat triggered incorrectly when an error happens when loading up a class ? I never worked on the JDK directly, so I'm no expert here.

## Build

To build the natives, use cmake and w/e compiler you want (I personally use Visual Studio 2019).
The project is set up to use Java 8 headers **ONLY**, it's with that setup that I reproduced the issue. I didn't test building with newer JDKs.

For the Java side, a simple `mvn clean package` will do the trick. If you don't want to recompile when switching between JDKs, I recommend you build with the oldest one you have.

## Run

Example commands (cygwin):
```bash
# If you want to use JDK 17
PATH=/c/Program\ Files/Java/jdk-17.0.1/bin:${PATH}

# If you want to use JDK 22
PATH=/c/Program\ Files/Java/jdk-22.0.1/bin:${PATH}

# If you want to use JDK 23
PATH=/c/Program\ Files/Java/jdk-23/bin:${PATH}

java -agentpath:native/cmake-build-debug/jdk-21-crash.dll -jar java/target/jdk-21-crash-1.0-SNAPSHOT.jar
```

Example output (Java 17):
```
[...]
[ClassFileLoadHook] Called.
[ClassFileLoadHook] Loaded class: XXX/XXX/XXX with loader 1
[ClassFileLoadHook] Phase: 4
[...]
Code successfully ran with Java 17.0.1
[...]
```

Example output (Java 18-23):
```
[...]
[ClassFileLoadHook] Called.
[ClassFileLoadHook] Loaded class: XXX/XXX/XXX with loader 1
[ClassFileLoadHook] Phase: 4
[ClassFileLoadHook] Called.
[ClassFileLoadHook] Loaded class: java/lang/ClassCircularityError with loader 1
[ClassFileLoadHook] Phase: 4
[BREAKS]
```