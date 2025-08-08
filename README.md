# EasyLib
EasyLib is a collection of libraries used for optimisation (`superthread`, `dynAVX`, `speedIO`), and easy tools (`media`, `AI`) without requiring more than a few headers.

## Compatibility &amp; Usability
A list of projects that need nothing more than headers:
* DynAVX
* Superthread

A list of projects that need extra compilation:
* SpeedIO (`.asm` file needs compilation, `.o` file available)

A list of projects that require other programs:
* AI (llama.cpp)
* Media (ffmpeg)

And, not all projects support Linux either. Some are based off windows' API.
* Media
* AI

## How to use
Each folder has a README.md inside, describing how to use them and whether they need extra compilation (and how to do that compilation) alongside tutorials and examples.