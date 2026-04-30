@echo off
"C:\\Program Files\\Android\\Android Studio\\jbr\\bin\\java" ^
  --class-path ^
  "C:\\Users\\rialpe\\.gradle\\caches\\modules-2\\files-2.1\\com.google.prefab\\cli\\2.0.0\\f2702b5ca13df54e3ca92f29d6b403fb6285d8df\\cli-2.0.0-all.jar" ^
  com.google.prefab.cli.AppKt ^
  --build-system ^
  cmake ^
  --platform ^
  android ^
  --abi ^
  x86_64 ^
  --os-version ^
  21 ^
  --stl ^
  c++_shared ^
  --ndk-version ^
  25 ^
  --output ^
  "C:\\Users\\rialpe\\AppData\\Local\\Temp\\agp-prefab-staging11452317801573183365\\staged-cli-output" ^
  "C:\\Users\\rialpe\\.gradle\\caches\\8.12\\transforms\\147640e7cccd059147855c7f80a51283\\transformed\\SDL3-3.4.0\\prefab"
