@echo off
"C:\\Users\\mtayk\\.jdks\\jbr-17.0.14\\bin\\java" ^
  --class-path ^
  "C:\\Users\\mtayk\\.gradle\\caches\\modules-2\\files-2.1\\com.google.prefab\\cli\\2.0.0\\f2702b5ca13df54e3ca92f29d6b403fb6285d8df\\cli-2.0.0-all.jar" ^
  com.google.prefab.cli.AppKt ^
  --build-system ^
  cmake ^
  --platform ^
  android ^
  --abi ^
  armeabi-v7a ^
  --os-version ^
  21 ^
  --stl ^
  c++_static ^
  --ndk-version ^
  21 ^
  --output ^
  "C:\\Users\\mtayk\\AppData\\Local\\Temp\\agp-prefab-staging3949031922538660737\\staged-cli-output" ^
  "C:\\Users\\mtayk\\.gradle\\caches\\8.9\\transforms\\11fe44731a1634b54b2fbcac14133211\\transformed\\jetified-shadowhook-1.0.9\\prefab"
