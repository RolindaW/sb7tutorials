
# OpenGL SuperBible 7th Edition Tutorials

## Atention

This document explains how to use OpenGL SuperBible 7th Edition "sb7" framework on custom projects (applications).

Before following the steps described below, it is necessary to have completed all the instructions indicated in the "HOWTOBUILD.tx" file.
In other words, it is necessary to have generated with the "CMake" tool the corresponding projects for "GLFW3" and "SuperBible", compiled them and deposited the resulting files in the specified directories, among others.

In this brief guide, we will first create a project template (wizard), which will then be used to create each of the corresponding projects.
This is not really necessary (it can be done directly on a common project), but it is highly recommended in terms of efficiency, since otherwise you would have to configure all the settings for each new project.  

## 1 Locate required files

Note: My working directory for superbible repository is "C:\workspace\sb7code"

Include

- "C:\workspace\sb7code\include"
- "C:\workspace\sb7code\extern\glfw-3.0.4\include"

Lib

- "C:\workspace\sb7code\lib"

Media (only for the example projects that require it)

- "C:\workspace\sb7code\bin\media"

## 2 Copy required files into new working directory

Note: My working directory for superbible tutorials is "C:\workspace\sb7tutorials"
Note: Place all required files in the same "resources" directory simply for convenience.

- "C:\workspace\sb7tutorials\resources\include"
- "C:\workspace\sb7tutorials\resources\extern\glfw-3.0.4\include"
- "C:\workspace\sb7tutorials\resources\lib"
- "C:\workspace\sb7tutorials\resources\media"

## 3 Create and setup a project template (wizard)

### 3.1 Create new wizard project/solution

Note: In VS corresponds to "Windows Desktop Wizard"

- Project name: sb7wizard ("C:\workspace\sb7tutorials\sb7wizard\sb7wizard")
- Solution name: sb7practice ("C:\workspace\sb7tutorials\sb7wizard\sb7wizard.sln")

Then, click "Create"

- Application type: "Desktop Application (.exe)"
- Additional options: Check "Empty project" option

### 3.2 Remove 32 bit (x86) platform configuration from solution settings.

Note: This step is optional.

### 3.3 Add default files to be included within the project (e.g. base "main.cpp" file with framework application initialization)

Note: This step is optional.

### 3.4 Setup the project

Note: In VS, right-click on the project and select "Properties".

#### 3.4.1 Includes

Configuration: All Configurations
Configuration Properties > VC++ Directories > General > Include Directories
Add two lines:
- "C:\workspace\sb7tutorials\resources\include"
- "C:\workspace\sb7tutorials\resources\extern\glfw-3.0.4\include"

Description: Add header files of sb7 and glfw3 (and others) libraries.

#### 3.4.2 Libs

Configuration: All Configurations
Configuration Properties > VC++ Directories > General > Library Directories
Add single line:
- "C:\workspace\sb7tutorials\resources\lib"

Description: Add sb7 and glfw3 libraries.

#### 3.4.3 Pre-processor

Configuration: All Configurations
Configuration Properties > C/C++ Directories > Preprocessor > Preprocessor Definitions
Add single line:
- WIN32

Description: Required by sb7 header file, in order to use windows OS. If not, unix is expected.

#### 3.4.4 Linker

Configuration: Release
Configuration Properties > Linker > Command Line
Add single line:
- sb7.lib glfw3.lib opengl32.lib glu32.lib

Configuration: Debug
Configuration Properties > Linker > Command Line
Add single line:
- sb7_d.lib glfw3_d.lib opengl32.lib glu32.lib

Description: Required for the linker. Note sb7 and glfw3 libraries are different.

#### 3.4.5 Subsystem

Note: Probably not required due to project template type (expected subsystem by default).

Configuration: All Configurations
Configuration Properties > Linker > System > SubSystem
Select:
- Windows (/SUBSYSTEM:WINDOWS)

### 3.5 Export as a project template (select the project and go into Project > Export Template)

## 4 Create new project/solution using the project template

Note: Create all new (practice) projects into same solution simply for convenience.
Note: Use an appropriate naming convention for projects to be able to identify them easily.

### 4.1 Create new practice project/solution

- Project name: ch02app01simpleapp ("C:\workspace\sb7tutorials\sb7practice\ch02app01simpleapp")
- Solution name: sb7practice ("C:\workspace\sb7tutorials\sb7practice\sb7practice.sln")

Note: Base new project on previously created project template.

### 4.2 Add required stuff

### 4.3 Compile, run and enjoy!
