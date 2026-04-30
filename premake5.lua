-- Experimental android studio premake5 module
--require "tools/premake5/android_studio"

workspace "SDL3Project"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    startproject "SDL3App"
    location ("build/" .. _ACTION)

    filter "system:windows"
        platforms { "Win64" }
        
    filter "system:macosx"
        platforms { "macOS" }
        
    filter "system:ios"
        platforms { "iOS" }
        architecture "ARM64"
        
    filter "system:android"
        platforms { "Android" }
        architecture "ARM64"
        
    filter {}

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "SDL3App"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir)
    objdir ("build/obj/" .. outputdir)

    files {
        "include/**.h",
        "src/**.cpp"
    }

    includedirs {
        "include"
    }

    externalincludedirs {
        "deps/SDL3/include"
    }

    -- Windows Configuration
    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS" }
        
        externalincludedirs {
          "deps/glew/include"
        }

        libdirs { 
          "deps/SDL3/lib_windows/x64",
          "deps/glew/lib/Release/x64"
        }
        links { "SDL3", "opengl32", "glew32s" }
        
        postbuildcommands {
            --"{COPYFILE} deps/SDL3/lib_windows/x64/SDL3.dll %{cfg.targetdir}"
            "{COPYFILE} ../../deps/SDL3/lib_windows/x64/SDL3.dll %{cfg.targetdir}"
        }
        --copy { "deps/SDL3/lib_windows/x64/SDL3.dll" }

    -- macOS Configuration
    filter "system:macosx"
        systemversion "10.15"
        defines { "PLATFORM_MACOS" }
    
        files {
          "include/**.h",
          "src/**.cpp"
        }
       
        libdirs {
          "deps/SDL3/lib_macos"
        }

        links {
          "SDL3"
        }

        --frameworkdirs { "deps/SDL3/frameworks/SDL3.xcframework/macos-arm64_x86_64" }
        
        links {
            --"SDL3.framework",
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework",
            "CoreAudio.framework",
            "AudioToolbox.framework",
            "AVFoundation.framework",
            "CoreMedia.framework",
            "CoreHaptics.framework",
            "ForceFeedback.framework",
            "GameController.framework",
            "Carbon.framework",
            "CoreFoundation.framework",
            "QuartzCore.framework",
            "UniformTypeIdentifiers.framework",
            "Metal.framework",
            "OpenGL.framework"
        }
        

    -- iOS Configuration
    filter "system:ios"
        kind "WindowedApp"
        systemversion "12.0"
        defines { "PLATFORM_IOS" }
        
        files {
          "include/**.h",
          "src/**.cpp"
        }
      
        libdirs {
          "deps/SDL3/lib_ios_simulator"
        }

        links {
          "SDL3"
        }

        --frameworkdirs { "deps/SDL3/frameworks/SDL3.xcframework/ios-arm64" }
        --frameworkdirs { "deps/SDL3/frameworks/SDL3.xcframework/ios-arm64_x86_64-simulator" }
        
        links {
            "UIKit.framework",
            "Foundation.framework",
            "CoreGraphics.framework",
            "QuartzCore.framework",
            "CoreAudio.framework",
            "AudioToolbox.framework",
            "AVFoundation.framework",
            "GameController.framework",
            "Metal.framework",
            "IOKit.framework",
            "CoreVideo.framework",
            "CoreMedia.framework",
            "CoreHaptics.framework",
            "CoreFoundation.framework",
            "UniformTypeIdentifiers.framework",
            "CoreBluetooth.framework",
            "CoreMotion.framework",
            "Metal.framework",
            "OpenGLES.framework",
        }
        
        
        xcodebuildsettings {
            ["GENERATE_INFOPLIST_FILE"] = "YES",
            ["PRODUCT_BUNDLE_IDENTIFIER"] = "com.mycompany.sdl3app"
            --["INFOPLIST_FILE"] = "ios/Info.plist",
            --["CODE_SIGN_IDENTITY"] = "iPhone Developer",
            --["DEVELOPMENT_TEAM"] = "YOUR_TEAM_ID"
        }

    -- Android Configuration
    --filter "system:android"
        --kind "SharedLib"
        --defines { "PLATFORM_ANDROID" }
        
        --buildoptions {
        --    "-std=c++17",
        --    "-fPIC"
        --}
        
        --includedirs {
        --    "$(ANDROID_NDK_ROOT)/sources/android/native_app_glue"
        --}
        
        --links {
        --    "SDL3",
        --    "GLESv2",
        --    "log",
        --    "android"
        --}
        
        --libdirs { "deps/SDL3/lib/android/$(TARGET_ARCH_ABI)" }

    -- Debug Configuration
    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "on"
        optimize "off"

    -- Release Configuration
    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        symbols "off"
        optimize "on"

    filter {}
